# HT9045_CONFIG 欄位速查：群組 [A] Function（自動化 / 流程控制）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Axx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 101 個欄位，分屬 89 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| A01 | Competence | `cbA01` | `bA01AutoSwitchToOperatorMode` | bool | `bAutoSwitchToOperatorMode` | -- | -- | Competence | kevin 20150206 超過時間變成op模式 |
| A01 | Competence | `edA01` | `iA01ChangeOpTime` | int | `iChangeOpTime` | -- | -- | Competence | kevin 20150206 超過時間變成op模式 |
| A01-1 | Press start auto switch to operator | `cbA01_1` | `bA01PressStartAutoSwitchToOperatorMode` | bool | `bPressStartAutoSwitchToOperatorMode` | 35002 | ECBool | Press start auto switch to operator | JerryYang 20170705 add for JCET 按start要切為OP權限改為選項 |
| A01-2 | Disable Saving Parameters when switch to operator | `cbA01_2` | `bA02DisableSaveParsWhenSwitchToOp` | bool | `b2DisableSaveParsWhenSwitchToOp` |  |  | Disable Saving Parameters when switch to operator | RogerYang 20260305 張寧說時間到都要登出，且不可被更改任何設定(A01_2) |
| A02 | Can select Normal or Prime bin data | `cbA02` | `bA02BinModelPrime` | bool | `BinModelPrime` | 35014 | ECBool | Can select Normal or Prime bin data | ChungHung 20120912 add Bin Setting like Epson |
| A03 | After home, pick and carry IC put to error bin | `cbA03` | `bA03UseAfterHomeCarryAndSuckIcToRBin` | bool | `bA03UseAfterHomeCarryAndSuckIcToRBin` | 35003 | ECBool | After home, pick and carry IC put to error bin |  |
| A04 | Loader magazine, tray split fail can skip | `cbA04` | `bA04LoaderTraySplitFailCanSkip` | bool | `bA04LoaderTraySplitFailCanSkip` | 35004 | ECBool | Loader magazine, tray split fail can skip |  |
| A05 | Use one touch docking (OTD) | `cbA05` | `bA05UseAutoDocking` | bool | `bUseAutoDocking` | 35005 | ECBool | Use one touch docking (OTD) | ChungHung 20120718 add UseAutoDocking Check Sensor |
| A08 | Loader no tray clean out and clean out finish check again | `cbA08` | `bA08LastLoaderAutoCleanOutAndCheckAgain` | bool | `bLastLoaderAutoCleanOutAndCheckAgain` | 35009 | ECBool | Loader no tray clean out and clean out finish check again | ChungHung 20130528 SCK要求AutoClean後要自動檢測是否Loader有補Tray |
| A09 | Use by arm close site function | `cbA09` | `bA09_ByArmCloseSite` | bool | `bCloseSiteByIndexArm` | 35010 | ECBool | Use by arm close site function | ChungHung 20130910 alter for SCK can close site by Index |
| A09-1 | If all site closed, disable arm automatically. | `chkA09_1` | `bA09_1_AutoCloseArm` | bool | `bA09_1_AutoCloseArm` |  | ECBool | If all site closed, disable arm automatically. | Steven 20220819 : 單Arm Site全關時, 就把Arm關了 |
| A10-1 | Enable ART | `cbA10` | `bA10_AutoReTest` | bool | `bAutoReTest` | 35011 | ECBool | Enable ART | ChungHung 20140317 add Auto Retest |
| A10-2 | Auto retest limit | `edA10_2` | `iAutoRetestLimit` | int | `iAutoRetestLimit` |  | ECInteger | Auto retest limit |  |
| A10-3 | Fail yield rate >= | `cbA10_3` | `bA10TestModeForART` | bool | `EnableA10TestModeForART` |  | ECInteger | Fail yield rate >= | Steven 20161208 : ART need to set to 32binGS for ATK |
| A10-3 | Fail yield rate >= | `cbA10_3_ARTTestMode` | `iA10TestModeForART` | int | `A10TestModeForART` |  | ECInteger | Fail yield rate >= | Steven 20161208 : ART need to set to 32binGS for ATK |
| A10-3 | Fail yield rate >= | `edA10_3` | `iFailYieldRate_ART` | int | `iFailYieldRate_ART` |  | ECInteger | Fail yield rate >= |  |
| A10-4 | Tray arm speed when ART | `edA10_4` | `iARTTrayArmSpeed` | int | `A10_4_ARTTrayArmSpeed` |  | ECInteger | Tray arm speed when ART |  |
| A10-5 | Enable auto correction function | `cbA10_5` | `bA10_5SCKART_AutoCorrection` | bool | `bA10_5SCKART_AutoCorrection` |  | ECBool | Enable auto correction function | Steven 20171211 (Wei) : Auto correction for SCK ART |
| A10-6 | Enable HANA test mode | `cbA10_6` | `bA10_6_HANA_ART_TestMode_Enable` | bool | `bA10_6_HANA_ART_TestMode_Enable` |  |  | Enable HANA test mode | JimmyChiu 20241023 HANA ART Function |
| A10-6 | Enable HANA test mode | `cbA10_6_HANA_ARTMode` | `iA10_6_HANA_ART_TestMode` | int | `iA10_6_HANA_ART_TestMode` |  |  | Enable HANA test mode | JimmyChiu 20241023 HANA ART Function |
| A10-7 | Enable FTCT | `cbA10_7` | `bA10_7_Renesas_FTCT` | bool | `bA10_7_Renesas_FTCT` |  |  | Enable FTCT | RogerYang 20251108 : 瑞薩FTCT |
| A11 | Barcode reader over                sec.(>10Sec) | `cbA11` | `bA11BarcodeTime` | bool | `bBarcodeTime` | -- | -- | Barcode reader over sec.(>10Sec) | 20140310 Wei  Barcode Reader持續時間 |
| A11 | Barcode reader over                sec.(>10Sec) | `edA11` | `iA11BarcodeTime` | int | `iBarcodeTime` | -- | -- | Barcode reader over sec.(>10Sec) | 20140310 wei  Barcode Reader持續時間 |
| A12 |  | `cbA12` | `bA12ClearLoaderDevice` | bool | `bClearLoaderDevice` |  | ECBool | Clear loader tray device | ChungHung 20140701 add AutoRetest |
| A12 |  | `edA12` | `iClearLoaderCount` | int | `iClearLoaderCount` |  | ECBool | Clear loader tray device | wei 20150810 拍拍Tray次數設定 |
| A12-1 |  | `edtA12_1` | `iCleanLoaderOffset` | int | `iCleanLoaderOffset` |  |  |  | wei 20150826 拍拍Tray X軸 Offset |
| A14 | Use barcode reader to change work file | `cbA14` | `bA14UseBarCodeSetWorkFile` | bool | `bUseBarCodeSetWorkFile` |  | ECBool | Use barcode reader to change work file | Frank 20150909 : CC_AMKOR 需要使用BarcodeReader讀取工作檔 |
| A15 | Use ESD auto decay function | `cbA15` | `bA15AutoDecayTest` | bool | `bAutoDecayTest` |  | ECBool | Use ESD auto decay function | Ifor 20150924 :Add Auto Decay Test |
| A15 | Use ESD auto decay function | `edA15_ESDReportTime` | `dESDDataReportTime` | double | `dESDDataReportTime` |  | ECBool | Use ESD auto decay function | Ifor 20160321 : add ESD Data Report Time |
| A15-1 |  | — | `bA15_1ESDGiveWayFunction` | bool | — |  | — |  | Ifor 20240830 add:ESD 讓位功能 |
| A16 | Contact test use drop contact and vacuum off mode | `chA16` | `bA16ContactTestDropContact` | bool | `bContactTestDropContact` |  | ECBool | Contact test use drop contact and vacuum off mode Normal Use DirectContactMode and VacuumOFFMode | wei 20150831 |
| A17-1 | Disable RESET button | `cbA17_1` | `bA17RESETButtonDisable` | bool | `bRESETButtonDisable` | 35015 | ECBool | Disable RESET button | kevin 20151113 RESET 按鍵使用 |
| A17-2 | RESET without testing until cleam out finish | `cbA17_2` | `bA17_1RESETCleanOutWithoutTest` | bool | `bA17_1RESETCleanOutWithoutTest` |  | ECBool | RESET without testing until cleam out finish | JimmyChiu 20211012 reset->clean out 不進行檢測排料 |
| A19 | Use PM alarm function | `cbA19` | `bA19UsePMAlarmFunction` | bool | `bUsePMAlarmFunction` |  | ECBool | Use PM alarm function | wei 20160225 PMAlarmFunction |
| A20-1 | Check RTC function | `cbA20_1` | `bA20_1CheckRTCFunction` | bool | `bA20_1CheckRTCFunction` |  | ECBool | Check RTC Function | wei 20171023 Disable Start比對 |
| A20-2 | Check Tray ID function | `cbA20_2` | `bA20_2CheckTrayIDFunction` | bool | `bA20_2CheckTrayIDFunction` |  | ECBool | Check Tray ID Function | wei 20171023 Disable Start比對 |
| A20-3 | Check auto clean function | `cbA20_3` | `bA20_3CheckAutocleanFunction` | bool | `bA20_3CheckAutocleanFunction` |  | ECBool | Check Auto clean Function | wei 20171023 Disable Start比對 |
| A20-4 | Check conts fail function | `cbA20_4` | `bA20_4CheckContsFailFunction` | bool | `bA20_4CheckContsFailFunction` |  | ECBool | Check Conts Fail Function | wei 20171023 Disable Start比對 |
| A20-5 | Check OCR function | `cbA20_5` | `bA20_5CheckOCRFunction` | bool | `bA20_5CheckOCRFunction` |  | ECBool | Check OCR Function | wei 20171023 Disable Start比對 |
| A21 | Rotator detect IC floating error, need to shake. | `cbA21` | `bA21RotateDetectErrNeedShake` | bool | `bRotateDetectErrNeedShake` | 35016 | ECBool | Rotate Detect Error Need Shake | JerryYang 20160825 Rotate sensor偵測異常,要先試著旋轉三次再跳alarm |
| A22-1 | Enable magnetic scale | `cbA22_1` | `bA22MagneticScale` | bool | `bA22MagneticScale` |  | ECBool | Enable Magnetic Scale | Frank 20161109 add 磁性尺 |
| A22-2 | Show message and keep running | `edA22_2` | `dA22MagneticScaleKeepRunRange` | double | `dMagneticScaleKeepRunRange` | 35017 | ECDouble | Show Message and keep running | Frank 20161109 add 磁性尺 |
| A22-3 | Show message and stop running | `edA22_3` | `dA22MagneticScaleStopRunRange` | double | `dMagneticScaleStopRunRange` | 35018 | ECDouble | Show Message and Stop running | Frank 20161109 add 磁性尺 |
| A23 | Check '#39'lot no'#39' in SLT report | `cbA23` | `bA23CheckLotNoInSLTReport` | bool | `bA23CheckLotNoInSLTReport` | 35019 | ECBool | Check 'lot no' in SLT report | JerryYang 20170421 (Steven) JCET吳如春要求必須輸入lot no才能start  //JerryYang 20170706 重新啟用A23 |
| A24 | Auto backup setup file | `cbA24` | `bA24AutoBackupSetupFile` | bool | `bA24AutoBackupSetupFile` |  | ECBool | Auto Backup Setup File | Ifor 20170508 (wei) add Auto BackUp Setup File & Last Data |
| A25-1 |  | `edA25_1` | `asA25RunExecutFile` | AnsiString | `asA25RunExecutFile` |  |  |  | Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045 |
| A25-2 |  | `edA25_2` | `asA25RunExecutButtonName` | AnsiString | `asA25RunExecutButtonName` |  |  |  | Sam 20170916 (Steven) 移植超豐外部呼叫執行檔功能 form HT-7045 |
| A26 | Motor speed sort display | `cbA26` | `bA26MotorSpeedSortDisplay` | bool | `bA26MotorSpeedSortDisplay` | 35020 | ECBool | Motor Speed Sort Display | KaiChen 20171225 (Steven)：Add Speed Display |
| A27 | Enable light scale | `cbA27` | `bA27EnableLightScale` | bool | `bA27EnableLightScale` | 35021 | ECBool | Enable Light Scale |  |
| A27-1 | Log light scale data | `cbA27_1` | `bA27_1LogEnableLightScaleData` | bool | `bA27_1LogEnableLightScaleData` | 35022 | ECBool | Log Enable Light Scale Data | KaiChen 20171228 ：Log Light Scale Data |
| A28-1 |  | `edtA28_1` | `asA28_1ShowPmSopReadFilePath` | AnsiString | `asA26_1ShowPmSopReadFilePath` |  |  |  | KaiChen 20171111 (Steven) ：超豐 開啟指定路徑的 HTML 檔案(PM SOP) |
| A29 | Enable auto clean function | `cbA29` | `bA29EnableAutoCleanFunction` | bool | `bA29EnableAutoCleanFunction` |  | ECBool | Enable Auto Clean Function | wei 20171023 Auto clean開關 |
| A30 | Setup teach function | `cbA30` | `bA30SetupTeachFunction` | bool | `bA30SetupTeachFunction` |  | ECBool | Setup Teach function | JerryYang 20180921 Setup Teach功能 |
| A31 | Enable auto clean ION fan | `cbA31` | `bA31EnableAutoCleanIonFanFunction` | bool | `bA33EnableAutoCleanIonFanFunction` | -- | -- | Auto Clean Ion Fan | Isaac 20210609 : IO觸發IonFan清針     //Steven 20230308 : A33 --> A31 |
| A31-1 |  | `cbA31_1` | `bA31AutoCleanIonFanInitialStart` | bool | `bA33AutoCleanIonFanFunctionInitialStart` |  |  | Initial start | Isaac 20210609 : IO觸發IonFan清針 |
| A32 | Enable FTP Automation | `cbA32` | `bA32EnableFTPAutomation` | bool | `bA32EnableFTPAutomation` | -- | -- | FTP Automation | KaiChen 20190530 ：Sigurd FTP Automation |
| A32-1 | Handler ID | `edA32_1` | `asA32_1_HandlerID` | AnsiString | `asA32_1_HandlerID` |  | ECText | Handler ID | KaiChen 20190530 ：Sigurd FTP Automation |
| A32-01 |  | `cbA32_01` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_Temperature` |  | ECBool | Check List Enable eCL_Temperature |  |
| A32-02 |  | `cbA32_02` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_Alarm` |  | ECBool | Check List Enable eCL_Alarm |  |
| A32-2 | Return Handler ID To OI | `cbA32_2` | `bA32_2ReturnHandlerID2OI` | bool | `bA32_2ReturnHandlerID2OI` |  | ECBool | Return Handler ID To OI | KaiChen 20200618 ：矽格，可以選擇是否回傳 HandlerID 給 OI |
| A32-03 |  | `cbA32_03` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_FT_Yield` |  | ECBool | Check List Enable eCL_FT_Yield |  |
| A32-3 | For 93K Function | `cbA32_3` | `bA32_3For93KFunction` | bool | `bA32_3For93KFunction` |  | ECBool | For 93K Function | Sam 20220620 : 中興廠新增 A32-3 功能 for 93K function |
| A32-04 |  | `cbA32_04` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_SiteMapping` |  | ECBool | Check List Enable eCL_SiteMapping |  |
| A32-05 |  | `cbA32_05` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_Speed` |  | ECBool | Check List Enable eCL_Speed |  |
| A32-06 |  | `cbA32_06` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_Contact` |  | ECBool | Check List Enable eCL_Contact |  |
| A32-07 |  | `cbA32_07` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_Category` |  | ECBool | Check List Enable eCL_Category |  |
| A32-08 |  | `cbA32_08` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_BinSetting` |  | ECBool | Check List Enable eCL_BinSetting |  |
| A32-09 |  | `cbA32_09` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_TrayForm` |  | ECBool | Check List Enable eCL_TrayForm |  |
| A32-10 |  | `cbA32_10` | `bA32EnableCheckList` | bool[] | `CheckList_Enable_HotPlate` |  | ECBool | Check List Enable eCL_HotPlate |  |
| A33 | Set machine IC to error bin after out shuttle loss IC | `cbA33` | `bA33SetICToErrBinAfterOutShtLossIC` | bool | `bA33SetMachineIC2ErrorBinAfterOutShuttleLossIC` |  | ECBool | Set machine IC to error bin after out shuttle loss IC | KaiChen 20200304 ：矽格-湖口，要求OutShuttle Loss IC 時機台上的IC放到R道 |
| A35 |  | `cbA35` | `bA35SetErrBinWhenOutShtLoseAndPickupErr` | bool | `bA35SetErrBinWhenOutShtLoseAndOutShtPickupErr` |  | ECBool | After out shuttle lose IC/out arm pickup error, set to error bin. | JerryYang 20220215 : 松諭要求的功能Out shuttle lose IC 以及 out arm pick up error set to error bin |
| A36 | Open door need device to Error bin.(index+output shuttle) | `cbA36` | `bA36OpenDoorSetErrBin` | bool | `bA36OpenDoorSetErrBin` |  | ECBool | Open door need device to Error bin.(index+output shuttle) | JerryYang 20210901 : Microchip要求開安全門要分ERROR BIN |
| A37 | Supported EAP 3.8 | `cbA37` | `bA37LotStartLotEnd` | bool | `bA32LotStartLotEnd` |  | ECBool | Supported EAP 3.8 | JerryYang 20220923 : add SPIL ART LOT START/LOT END timeout機制 |
| A38 | SLT summary. | `cbA38` | `bA38_SLT_Summary` | bool | `bA35_SLT_Summary` |  | ECBool | SLT summary. |  |
| A39 | Record run state                        sec. | `cbA39` | `bA39RecordRunState` | bool | `bA39RecordRunState` |  | ECBool | Record Run State | Ifor 20230420 add:ASEM要求新增Run Status Log Function |
| A39 | Record run state                        sec. | `edA39` | `iA39RecordTime` | int | `iA39RecordTime` |  | ECBool | Record Run State | Ifor 20230420 add:ASEM要求新增Run Status Log Function |
| A40 | Don'#39't record 2D data when contact page is opened | `cbA40` | `bA40DoNotRecord2DDataWhenContactShow` | bool | `bA40DoNotRecord2DDataWhenContactShow` |  | ECBool | Don't Record 2D Data When Contact Page is Opened | Ifor 20201124 add:Turn Off 2D Function When Contact Show |
| A50 | 1x4 bias mode can use Y-offset (Default is 30mm) | `chkA50` | `bA50Enable1x4BiasYOffset` | bool | `bA32Enable1x4BiasYOffset` |  | ECBool | 1x4 bias mode can use Y-offset (Default is 30mm) | Steven 20200715 : for Tinton                                              //Steven 20230308 : A32 --> A50 |
| A51 | EQC mode | `cbA51` | `bA51EnableEQCMode` | bool | `bA31EnableEQCMode` |  | ECBool | EQC mode | JerryYang 20200312 EQC mode新增function on/off，功能關閉時無法切EQC mode  //Steven 20230308 : A31 --> A51 |
| A55 | Enable | — | `asA55PMAlarmUpdateFromServerbyFTP` | AnsiString | — | -- | — | PM Alarm Update From Server by FTP | JimmyChiu 20230315 : PM Alarm Update From Server by FTP |
| A55 | Enable | `cbA55` | `bA55EnablePMAlarmUpdateFromServerbyFTP` | bool | `bA55EnablePMAlarmUpdateFromServerbyFTP` | -- | -- | PM Alarm Update From Server by FTP | JimmyChiu 20230315 : PM Alarm Update From Server by FTP |
| A56 | Auto Teach Funciton | — | `bA56AutoTeachShuttleYPositionWhenAutoHeight` | bool | — | -- | — | Auto Teach Funciton | JimmyChiu 20211020 : Auto alignment mode |
| A56-1 | Enable Auto Teach Funciton | `cbA56_1` | `bA56EnableAutoTeachFunciton` | bool | `bA56EnableAutoTeachFunciton` |  | ECBool | Enable Auto Teach Funciton | JimmyChiu 20211020 : Auto alignment mode |
| A56-2 |  | `edA56_2` | `iA56ShuttlePickUpOffsetWhenAutoTeach` | int | `iA56ShuttlePickUpOffsetWhenAutoTeach` |  | ECInteger | Shuttle Pick Up Offset When Auto Teach um | JimmyChiu 20211020 : Auto alignment mode |
| A56-3 |  | `edA56_3` | `iA56SocketPickUpOffsetWhenAutoTeachOnly` | int | `iA56SocketPickUpOffsetWhenAutoTeachOnly` |  | ECInteger | Socket Pick Up Offset When Auto Teach Only um | JimmyChiu 20211020 : Auto alignment mode |
| A57-1 | Save ArmSpeed By Machine | `cbA57_1` | `bA57_1SaveArmSpeedByMachine` | bool | `bA57_1SaveArmSpeedByMachine` |  | ECBool | Save ArmSpeed By Machine | JimmyChiu 20220618 : save by machine |
| A57-2 | Save Temperature By Machine | `cbA57_2` | `bA57_2SaveTemperatureByMachine` | bool | `bA57_2SaveTemperatureByMachine` |  | ECBool | Save Temperature By Machine | JimmyChiu 20220618 : save by machine |
| A57-3 | Save Offset By Machine | `cbA57_3` | `bA57_3SaveOffsetByMachine` | bool | `bA57_3SaveOffsetByMachine` |  | ECBool | Save Offset By Machine | JimmyChiu 20220618 : save by machine |
| A58 | Show Close Sites Alarm | — | `bA58DoNotRecord2DDataWhenContactShow` | bool | — |  | — | Show Close Sites Alarm | Ifor 20201126 add:Contact 頁面開啟時不記錄2D 資料 |
| A58 | Show Close Sites Alarm | `cbA58` | `bShowCloseSiteAlarmWhenStart` | bool | `bShowCloseSiteAlarmWhenStart` |  | ECBool | Show Close Sites Alarm | Jimmychiu 20230925 : Show Close Sites Alarm When Start |
| A60 | AMR | — | `iA60LoaderQtyAtOneTime` | int | — |  | — | Continuous Mode disable  clean MUBA |  |
| A60 | AMR | — | `iA60NotifyQty` | int[] | — |  | — | Continuous Mode disable  clean MUBA |  |
| A60-1 |  | `cbA60_1` | `bA60EnableAMR` | bool | `bA60EnableAMR` |  |  |  | Sam 20240304 : 新增 AMR 功能 |
| A61 | Continuous Mode disable  clean MUBA | `cbA61` | `bA61DisableCleanMUBA` | bool | `bA61DisableCleanMUBA` |  |  | Continuous Mode disable  clean MUBA | Jeff 20241001 add Continuous Mode disable  clean MUBA |
| A62 | Use Stop Machine In/Out Arm Need To Home | `cbA62` | `bA62bUseStopMachineArmHome` | bool | `bA62bUseStopMachineArmHome` |  |  | Use Stop Machine In/Out Arm Need To Home | Ztex 2024.10.30 Add Use Stop Machine In/Out Arm Need To Home |
| A65 | Support Bundle INFO | `cbA65` | `bA65_BundleIDList` | bool | `bA65_BundleIDList` | 35031 | ECBool | Support Bundle INFO |  |
| A66 | 2D Sort | `cbA66` | `bA66_2D_Sort` | bool | `bA66_2D_Sort` |  | ECBool | 2D Sort |  |
| A67 | Trigger ONE CYCLE After SKIP specific Alarm. | `cbA67` | `bA67TriggerOneCycleWhenAlarm` | bool | `bA67TriggerOneCycleWhenAlarm` |  | ECBool | Record Door Open at Process after | JerryYang 20241028 : 矽品彰化要求 特定ALARM要觸發ONE CYCLE |
| A68 | Enable Automatic Loading and Unloading | `cbA68` | `bA68_AutoLoadUnload` | bool | `bA68_AutoLoadUnload` |  | ECBool | Check No9 cylinder iup sensor | JerryYang 20250224 : add |
| A71 | Backup Now Recipe | `cbA71` | `bA71UseBackupNowRecipe` | bool | `bA71UseBackupNowRecipe` |  | ECBool | Backup Now Recipe |  |
| A71 | Backup Now Recipe | `edA71` | `sA71FolderPath` | AnsiString | `sA71FolderPath` |  | ECBool | Backup Now Recipe |  |

---


## 未綁定 UI 元件的欄位

_無。所有 `A##` 開頭的欄位均已在主表中列出。_
