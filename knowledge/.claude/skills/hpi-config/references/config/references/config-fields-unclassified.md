> 保存來源：`.claude/skills/ht9045-config/references/config-fields-unclassified.md`，main `f57d93f15`。原文機型、版本與日期維持原標註；原程式碼行號僅為歷史定位，新查證依function／關鍵變數；目前共同項與差異先看 [共用對照](../../common.md)。

<!-- preserved-content:start -->
# HT9045_CONFIG 欄位速查：未分類欄位

> 來源：`Config.h`（V3.33.903.0_20260417）
> 在 `cConfiguration.dfm` 中**找不到對應 UI 元件**，且變數名稱**不符合 `X##` 群組命名規則**。
> 多為早期歷史欄位、內部狀態變數，或由 `ReadIniData` 直接讀寫的設定值。

---

共 255 個欄位。

| 變數名 | 型別 | 說明 |
|--------|------|------|
| `ContactConditionName` | AnsiString[] | wei 20160509 Life Time Count |
| `FtpDownloadPath` | AnsiString |  |
| `FtpHost` | AnsiString |  |
| `FtpPassword` | AnsiString |  |
| `FtpPasswordDownloadPath` | AnsiString | Sam 20210526 : 從 N06 DownloadPath 下載密碼本 |
| `FtpTransMode` | int | Steven 20230719 : 加入FTP傳輸模式 |
| `FtpUplaodPath` | AnsiString |  |
| `FtpUseSystemCallToUnZip` | bool | Steven 20140609 |
| `FtpUserName` | AnsiString |  |
| `GaliPosRange` | int | Isaac 20201012 : index Y超過範圍，做一次Tmode   //Isaac 20210604 : IndexY偵測範圍名子統一成IniConfig.GaliPosRange |
| `LotID_ART` | AnsiString |  |
| `OCRLightChange` | bool | wei 20181225 光源auto change |
| `OCRLightNoDown` | bool | wei 20181225 光源不下降 |
| `OperatorID_ART` | AnsiString |  |
| `RMSTesterID` | AnsiString | Ifor 20231219 add Tester ID |
| `SocketHandlerID` | AnsiString | ChungHung 20130204 add for ASE_KR Socket Tester |
| `SocketIP` | AnsiString | ChungHung 20130112 add for ASE_KR Socket Tester |
| `SocketPort` | AnsiString | ChungHung 20130112 add for ASE_KR Socket Tester |
| `SocketTimeOut` | int | ChungHung 20130112 add for ASE_KR Socket Tester |
| `TasterInputMethod` | int | Steven 20110311 : Taster輸入方式 |
| `TasterName` | AnsiString | Steven 20110311 : Taster名稱 |
| `TasterNo` | AnsiString | Steven 20110305 : Taster號碼 |
| `TasterType` | AnsiString | Steven 20110305 : Taster型號 |
| `aAlarmCodeByTcpIp_Adress` | AnsiString | ChungHung 20150518 add for SCK Send JamCode By TcpIp |
| `aAlarmCodeByTcpIp_Port` | AnsiString | ChungHung 20150518 add for SCK Send JamCode By TcpIp |
| `asByTimeOEEPath` | AnsiString | jou 20220301 : VTEST By time OEE path |
| `asCreateManualEOCAP_URL` | AnsiString | jou 20221104 : VTest CreateManualEOCAP function; |
| `asGetRcsCheckingResultACode` | AnsiString | jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult |
| `asGetRcsCheckingResultAction` | AnsiString | jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult |
| `asGetRcsCheckingResultUrl` | AnsiString | jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult |
| `asMesSyACodePath` | AnsiString | jou 20200409 : VTest Mes system |
| `asMesSyActionPath` | AnsiString | jou 20200409 : VTest Mes system |
| `asMesSyURLPath` | AnsiString | jou 20200409 : VTest Mes system |
| `asOCRWordType` | AnsiString | wei 20161128 確認字串各自Type是否正確 |
| `asQueryEocapStatusURL` | AnsiString | jou 20221104 : VTest CreateManualEOCAP function; |
| `asSummaryReportPath` | AnsiString | jou 20220120 : VTEST Summary report path |
| `asUPHReportPath` | AnsiString | jou 20220525 : VTEST UPH report path |
| `bASE_Report` | bool | kevin 20140918 ASE Kaoshsiung 每一顆IC履歷資料 |
| `bAbnormalStartCheck` | bool | Steven 20111216 : 不正常關程式偵測 |
| `bAlarmMustRedColor` | bool | Steven 20111116 : 特殊Alarm需要改紅底 |
| `bAlarmNeedServoOff` | bool | Steven 20110802 : Alarm時,相關位置要可以Servo Off |
| `bAmbRunChamberFanCanStop` | bool | jou 2012-01-30 機台生產 & 常溫時，Chamber風扇可以選擇不轉動 |
| `bAnyLevelCanGetStateRecode` | bool | ChungHung 20120922 add |
| `bAutoCleanShuttleDisable` | bool | jou 2013-02-27 Auto Clean disable shuttle sensor detect |
| `bAutoSaveLogWeek` | bool[] | jou 2012-10-15 Auto Save Log 支援 Week 選擇 |
| `bAutoTrayLink` | bool | jou 2012-06-14 Auto Tray Link |
| `bBackUpAutoFeed` | bool | 備份CleanOut模式 |
| `bBackUpInArmMode` | bool | 備份In Arm使用的吸嘴模式 |
| `bBinBox` | bool | jou 2012-12-11 support Bin Box |
| `bCanByPassIonFan` | bool | Steven 20111013 : 可以不檢查離子風扇 |
| `bCanDisableTempMonitor` | bool | 能夠關閉溫控器監視。 |
| `bChangeKitNoHardStop` | bool | jou 2015-12-08 Xilinx 驗證用 |
| `bChangeTempAutoSetDown` | bool | Steven 20110707 : 更換溫度時,自動按下Set鈕 |
| `bCheckBarCodeMap` | bool | Frank 20161025 確認四個角落的OCR Code |
| `bCheckFile` | bool | Steven 20101209 : 是否要檢查溫度 |
| `bCleanOutCanTrayEnd` | bool | Steven : CleanOut後,可以選擇Tray End |
| `bClearLotInfoWhenTrayFeed` | bool | Steven 20240916 : Tray Feed之後, 要不要清除Device Name |
| `bCompareOCRData` | bool | KenHsieh 20220825 : 新增OCR比對功能 |
| `bContactAlwaysIncludeShuttle` | bool | Steven 20090926 for KYEC 29818 : Never Enable and Always True |
| `bControlTorque` | bool | jou 2013-11-05 Index Control Torque |
| `bDisableSelectSearchLast` | bool | jou 2012-01-10 取消Setup，Search Last Mode功能。 |
| `bDisabledKeyin` | bool | wei 20161004 No IC 不能Keyin |
| `bDisibleResetButton` | bool | ChungHung 20111208 Disable Reset Button |
| `bDoorOpenHeadContinueHeat` | bool | ChungHung 20120719 add DoorOpenHeadContinueHeat |
| `bDoorOpenShuttleContinueHeat` | bool | Steven 20110709 : Chamber門打開時不關閉加熱電源，只將Temp=0，Hotplate & shuttle除外。 |
| `bDownLoadAutoCountClear` | bool | jou 2012-04-10 Download之後Auto Count Clear,避免未清除導致數量記數錯誤 |
| `bDualSiteCloseAbCanFullHotplate` | bool | ChungHung 20120911 add 與OneCycle can desable site 衝突 |
| `bDualSiteSupply4CH` | bool | jou 2012-11-20 Dual Site supply 4's Channel |
| `bDutOnOffNeedASM` | bool | Steven 20120628 : 開關Site, 強制啟動Auto Site Mapping |
| `bEnableAutoCleanFunction` | bool | Steven 20110528 : 開啟Auto Clean功能 |
| `bEnableCCDUSETCPIP` | bool | Steven 20110528 : 開啟CCD功能 |
| `bEnableEPCheckFuntion` | bool | KYEC要每次都檢查EP |
| `bEnableErms` | bool | Steven 20160711 : 使用進階版RMS |
| `bEnableFTP` | bool | Steven 20110216 : 加一個開關 |
| `bEnableHotplateVibration` | bool | jou 2011-08-09 : Hotplate也要敲敲敲 |
| `bEnableInOutArmPlaceSkipSuckDetect` | bool | skip in/out arm drop error |
| `bEnableKT4HAlarm1` | bool | Steven 20120809 : KT4H使用Alarm1作加熱保護 |
| `bEnableOCRMoveSRead` | bool | wei 20161118 OCR S型讀取 |
| `bEnableRms` | bool | 是否使用RMS |
| `bEnableRmsCheckSetupFile` | bool | 是否使用RMS |
| `bEnableSocketCommunication` | bool | ChungHung 20130112 add for ASE_KR Socket Tester |
| `bEnableStartposshift` | bool | wei 20181225 初始點位 |
| `bEnableStepShuttle` | bool | jou 2013-07-16 Step Shuttle check Index -> Input |
| `bEnableTestingNeedStopAllMotor` | bool | jou 2013-09-25 Testing Need Stop All Motor |
| `bEnableUnloadTrayFree` | bool | unload tray汽缸開門時也要可以打開 |
| `bEnabledOCRCheckIC` | bool | wei 20161228 確認Tray是否有IC |
| `bEnabledOCRCheckWordCount` | bool | wei 20220413 開關字數判斷功能 |
| `bEventLogAutoSaveFunction` | bool | 自動存EventLog |
| `bFTBin2RTBin` | bool | jou 2012-03-19 //Steven 20120131 : 當FT Bin存檔時,把RT Bin設定跟FT一樣 |
| `bFTPJamCodeUpload` | bool | ChungHung 20140108 add FTP unload jam code |
| `bFTTrayAss2RTTrayAss` | bool | jou 2012-05-02 當FT Tray Assignment存檔時,把RT Tray Assignment設定跟FT一樣 |
| `bFinishSuckAfterPause` | bool | Hung 20110901 : finish suck and destroy 後才暫停 |
| `bFix3PutAllFullIC` | bool | ChungHung 201111215 Fix3盡量擺滿 |
| `bFtpPasswordDownload` | bool | Sam 20210526 : 從 N06 DownloadPath 下載密碼本 |
| `bGetRcsCheckingResult` | bool | jou 20230621 : VTEST Handler即時監控 GetRcsCheckingResult |
| `bHandlerCanUse8PickAtHotMode` | bool | Steven 20151117 : 2x2 8Picker at Hot mode |
| `bHaveRTCCheckSiteMap` | bool | Steven 20140513 : [D35] |
| `bHaveRotateShuttle` | bool | 旋轉蝦頭 |
| `bHeadChamberSocketMode` | bool | 2013-11-20    Dell    for TSMC Add Chamber + Head +Socket |
| `bHeadSocketMode` | bool | jou 2012-05-30 增加 Head + Socket Mode |
| `bHighModeCanOffTemp` | bool | Steven 20110524 : 加熱模式要可以隨時關加熱 |
| `bHotPlateMove1CM` | bool | Hung 20110812 : HotPlate使用1CM轉板 |
| `bIOFormCanControlHeaterFan` | bool | Steven 20120607 : 當chamber門開啟時,需要能開關風扇 |
| `bInOutArmCanPushHome` | bool | In/Out Arm Jam時,可以按Home對吸嘴歸零 |
| `bIndexAddPressEP` | bool | jou 20171026 (wei) : 測試中加壓EP |
| `bIndexArm2SupplyLight` | bool | jou 2012-10-19 Index Arm 2 供應光源 for CMOS |
| `bIndexDropErrCanMove` | bool | jou 2012-02-24 index drop error,have button can move arm Y front or Rear |
| `bIndexDropNeedPwdByIni` | bool | Steven 20110627 : Index掉料時,需要輸入密碼,密碼放在D:\HT9045\system\SpecialErrNote.ini |
| `bIndexDropOnlyReset` | bool | jou 2013-12-02 Index Drop Only Reset |
| `bIndexDropOnlySKIP` | bool | jou 2012-02-13 index drop error only skip |
| `bIndexEveryTimeCheckEP` | bool | Index每一次都確認EP是否有充飽氣。 |
| `bIndexJamInArmAway` | bool | Index掉料時,In Arm要移開 |
| `bIndexPickErrOnlySKIP` | bool | jou 2012-02-13 index pick-up error only skip |
| `bIndexPickupErrStop` | bool | jou 2012-02-29 index pick up error,index arm move to center & alarm |
| `bIndexPickupWait` | bool | jou 2012-06-29 Index Pick up need wait Soak Time |
| `bInitialStartDelayCount` | bool | jou 2012-11-30 高溫動作下希望增加顆數記數,在前幾顆下壓到Socket後,都要等待Delay time |
| `bKoreaFunction` | bool | Steven 20110831 : 韓國代理商的需求 |
| `bLastLoaderAutoCleanOut` | bool | Steven 20111028 : 搭配"最後一盤不入料"功能,要不要自動CleanOut |
| `bLastLoaderNoInSide` | bool | Steven 20110922 : 最後一盤不入料 |
| `bLowYieldAlarmSameNS` | bool | jou    2011-07-16 : Low Yield Alarm模式與NS機台相同,skip會清除單獨Site. |
| `bMaximFunction` | bool | JerryYang 20190522 Maxim統一軟體功能 |
| `bNewResetFunction` | bool | Steven 20130625 : 新的Reset方式 |
| `bNoHeadaddChamberOption` | bool | Steven :Head + Chamber disable |
| `bNoTrayAutoCleanOut` | bool | jou 2013-08-01 Loader No Tray Auto Clean |
| `bOCRAndBinLog` | bool | KenHsieh 20230406 : 新增OCR Data + Bin Log功能 |
| `bOCRBinLogAddMark` | bool | KenHsieh 20230412 : 新增OCR Data + Bin Log功能 + Mark |
| `bOneCycleCanTrayFeed` | bool | Steven : OneCycle時,要可以選擇Tray Feed |
| `bOneCycleDoQuickCleanOut` | bool | Steven 20110524 : OneCycle後,只做快速CleanOut |
| `bOneCycleNeedPowerOff` | bool | Steven : One Cycle後要關電 |
| `bOnlyRoomOrHot` | bool | Steven 20110814 : 只要兩種,不要Ambient/Hot模式 |
| `bOpenDoorNotStopFan` | bool | Steven 20110725 : Chamber門打開時不關閉風扇。 |
| `bOurArmDropICSkip` | bool | kevin 20171005 (wei) out arm drop ic 只能強至取出ic 開6號門 |
| `bOutShLoseNeedOpenChamber` | bool | jou 2013-12-12 Out Shuttle Lose Device Need Open Chamber Door and press Z1 |
| `bPasswordSecret` | bool | jou 2013-01-04 Password Txt 加密 |
| `bPowerSaveFunction` | bool | 省電模式 |
| `bQAMode` | bool | Steven 20111005 : QA模式 |
| `bQAModeFirstIn` | bool | 判斷是不是第一次進入QA模式 |
| `bRTC_Active` | bool | Sam 20240311 : 新增 RTC Lock by file 功能 |
| `bRTC_Enable` | bool | Sam 20240311 : 新增 RTC Lock by file 功能 |
| `bRTCbySystem` | bool | ChungHung 20120716 RTC by System |
| `bRecordPiggyBackStartEnd` | bool | jou 2011-11-14 start : 紀錄piggyback時間 |
| `bRecordSkipPosition` | bool | jou 2013-05-30 Record Skip position |
| `bRemeberAutoHeight` | bool | ChungHung 20120725 Amkor_K 要可以記住AutoHeight的值，除非重新K高度 但選單Arm時只移動-50 |
| `bResetCanServoOff` | bool | Steven 20111228 : 按下Reset時,若In/Out Arm沒IC,就Servo Off |
| `bResetClearArmIC` | bool | Steven 20120830 : Reset按下時,把吸嘴上的IC清空 |
| `bResetPutUntestToErrorBin` | bool | Steven 20120924 : For SCS, 使用Reset Mode,但是已測的要繼續分Bin |
| `bRetryNoNeedRestartGpib` | bool | Steven 20111220 : 測試TimeOut Retry時,不需要重開GPIB |
| `bRotateShNeedSensorCheck` | bool | ChungHung 20110922 : 轉轉蝦頭要檢查有沒有轉頭 Check Sensor |
| `bSIGURDFunction` | bool | KaiChen 20200506 ：矽格統一軟體功能 |
| `bSPILFunction` | bool | JerryYang 20170328 (Jou) 矽品客戶碼統一用SPILFunction |
| `bShowBinCT` | bool |  |
| `bShowContactCT` | bool |  |
| `bShowContactHeight` | bool |  |
| `bShowFTandRTButton` | bool | jou 2013-04-27 Show FT & RT Buttion |
| `bShowFTandRTButtonCanClick` | bool | Steven 20131224 : FT & RT Buttion 可以按 |
| `bShowFormByInitPos` | bool | jou 2013-12-02 |
| `bShowFunctionWindow` | bool | 顯示在溫度值下面的功能開關畫面 |
| `bShowIndexTime` | bool |  |
| `bShowLoaderCT` | bool |  |
| `bShowLotInfo` | bool | jou 2013-01-18 Show Lot Info |
| `bShowMainDebugRecord` | bool | jou 2013-02-25 Show Main form Debug Record |
| `bShowOffYieldBlink` | bool | Steven 20120609 : 功能關畫面要不要閃爍 |
| `bShowScanCate` | bool |  |
| `bShowTemper` | bool |  |
| `bShowTestCate` | bool |  |
| `bShowTimeInfo` | bool |  |
| `bShowTrayAndDeviceDir` | bool | jou 2013-03-25 show Tray & Device Direction |
| `bShowUPH` | bool |  |
| `bShuttleMode50` | bool | Dell 20111212   : 使用關Arm模式後,將被關的Arm測試高度設在-50mm |
| `bShuttleModeAccseeLevel` | bool | jou 2012-01-30 Yuedong Chen [Yuedong.Chen@amkor.com] 請將Setup裡面的Shuttle mode在password control單獨弄一個level，類似之前修改的contact force |
| `bSingaporeFunction` | bool | Steven 20120910 : 新加坡代理商的需求 |
| `bSiteMappingDisable` | bool | jou 2011-10-04 Site Mapping Disable，"Security_new.def", "Setup", "Machine Setup=1 \|\| 4" |
| `bSiteMappingFastSetDisable` | bool | jou 2011-10-11 Site Mapping 快速設定Disable |
| `bSocketCommunication` | bool | ChungHung 20130112 add for ASE_KR Socket Tester |
| `bStartProductOnLine` | bool | kevin 20140407 生產前OP OFF_LINE 強制 On line |
| `bTModeMotorFree` | bool | Hung 20110923 : 開啟index T Mode時，開啟馬達煞車(測試驗證使用) |
| `bTemp25degControl` | bool | jou 2014-06-07 Temperature 25 deg. control |
| `bTempOverCannotRun` | bool | Steven 20110607 : 溫度過高時,機器不能動 |
| `bTestIcCheckInContact` | bool | ChungHung 20140327 add by Customer |
| `bTesterTimeUpErrorNeedPassword` | bool | jou 2012-08-28 Tester Time up Error Need Password |
| `bTrayAssignUseGraphic` | bool | Steven 20111121 : 使用圖片去顯示Tray Assign |
| `bUseAutoOffsetFunction` | bool | jou 2013-08-29 Use Auto Offset Funtion |
| `bUseAutoSiteMapping` | bool | 開啟AutoSiteMapping功能 |
| `bUseBinFailCount` | bool | ChungHung 20120724 add BinFailCount |
| `bUseFix3` | bool | kevin 20110916 暫時性ic超過 15mm就不使用Fix3 tray |
| `bUseTemperatureReferSensor` | bool | Steven 20150108 : [L11-5] For海思使用兩組感溫 |
| `bUseTrayBlockMode` | bool |  |
| `bVTESTFunction` | int |  |
| `dATCAmbientTemperature` | double | Steven 20130122 : [L11] ATC常溫的溫度 |
| `dESDDelayTime` | double | Ifor 20160321 : add ESD Data Report Time |
| `dHeatGunTempATC` | double | JerryYang 20220408 : add for ATC3.5 |
| `dIndex30mmLoadRate` | double | jou 2011-06-10 |
| `dIndex30mmLoadRate_NS` | double | wei 20150303   京元NS浮動頭 |
| `dIndex30mmLoadRate_Offset` | double | 2014-06-26    Dell    for TSMC 高溫Load cell offset |
| `dIndex40mmLoadRate` | double | Steven 20110704 |
| `dIndex40mmLoadRate_NS` | double | wei 20150303   京元NS浮動頭 |
| `dIndex40mmLoadRate_Offset` | double | 2014-06-26    Dell    for TSMC 高溫Load cell offset |
| `dIndex56mmLoadRate` | double | wei 20151005 add 56mm |
| `dIndex56mmLoadRate_NS` | double | wei 20151005 add 56mm |
| `dIndex56mmLoadRate_Offset` | double | 2014-06-26    Dell    for TSMC 高溫Load cell offset        //wei 20151005 add 56mm |
| `dIndex60mmLoadRate` | double | jou 2011-06-10 |
| `dIndex60mmLoadRate_NS` | double | wei 20150303   京元NS浮動頭 |
| `dIndex60mmLoadRate_Offset` | double | 2014-06-26    Dell    for TSMC 高溫Load cell offset |
| `dIndexAddPressEP_Kg` | double | jou 20171026 (wei) : 測試中加壓EP |
| `dIndexVibrateEP_Kg` | double | jou 20171026 (wei) : 測試中加壓EP |
| `dSingleTempLimit` | double[] | Steven 20111013 : 個別溫度Offset |
| `dTrayXScale` | double[] |  |
| `dTrayXScale_Cold` | double[] |  |
| `dTrayXScale_Hot` | double[] |  |
| `dTrayYScale` | double[] |  |
| `dTrayYScale_Cold` | double[] | Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature <-- |
| `dTrayYScale_Hot` | double[] |  |
| `fAmbientTemp` | double | Steven 20101215 : 常溫的定義 |
| `iATCMaxAlarmContinuous` | int | Ifor 20200803 add:Hisi V2.4 最大峰值Alarm計時 |
| `iBackUpTesterMode` | int | 備份測試機連線狀態 |
| `iBlueLight` | int | wei 20160803 |
| `iByLotInputCount` | int[] |  |
| `iEP_Min_KG` | int | jou 2013-07-19 EP Min KG |
| `iEnableFirstTrayNeedAlarmNum` | int | ChungHung 20140521 add for ATK |
| `iGalilSpeedAcc` | int | jou 20171102 (Steven) : Galil Acc 改由文件修改 |
| `iGalilSpeedDec` | int | jou 20171102 (Steven) : Galil Dec 改由文件修改 |
| `iHDEnable` | int |  |
| `iImageRotate` | int |  |
| `iInOutSaveStartTime` | int | 進入省電模式 0:尚未開始 1:開始計時 2:計時到 |
| `iIndexAddPressEP_Time` | int | jou 20171026 (wei) : 測試中加壓EP |
| `iLotIDLength` | int | Frank 20170531 (Steven) add LotID 7碼 |
| `iNextEventLogRecordSpace` | int |  |
| `iOCRByAllDevice` | int | wei 20150713 BarCode掃全部  //wei 20150924 |
| `iOCRConditions` | int | ByNewTray:0 ByInitialStart:1 |
| `iOCRPort` | int | wei 20181225 ocr port |
| `iOCRRetry` | int | wei 20150723 |
| `iOCRSkip` | int | wei 20150721 |
| `iOCRWordCount` | int | wei 20160803 |
| `iRedLight` | int | wei 20160803 |
| `iServerEnable` | int |  |
| `iShowCateByArm` | int |  |
| `iSiteMapDirection` | int | Jimmychiu 20230807 : #R230804-ATK-H9-01 , V3.21.792.1 ,Add the Sitemap items in information.txt |
| `iStartposshift` | int | wei 20181225 初始點位 |
| `iTempeAlarmSecond_Below` | int | Steven 20111027 : 溫度過低的Alarm時間 |
| `iTempeAlarmSecond_Over` | int | Steven 20111027 : 溫度過高的Alarm時間 |
| `iUserLanguage` | int | Steven 20120203 : 使用者的語系 |
| `iVibratorHP1` | int | JerryYang 20200612 振動馬達作動時間累計 |
| `iVibratorSht1` | int |  |
| `iVibratorSht2` | int |  |
| `iVibratorUnloader` | int |  |
| `sErmsPath` | AnsiString |  |
| `sEvenLogDataTime` | AnsiString | Ifor 20160621 新增Even Log Record Date Time 字串 |
| `sEventLogLastRecordDate` | AnsiString | Steven 20140702 : For ecs Gem |
| `sGPIBMachineID` | AnsiString | kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND |
| `sLotID` | AnsiString | Steven 20140814 : Add for ASE_M |
| `sMachineType` | AnsiString | kevin 20130425 //2013.01.11 Q_Q TSMC GPIB COMMAND |
| `sProductName` | AnsiString | Steven 20110117 : 目前使用的工作檔 |
| `sProductTemp` | AnsiString | Steven 20110117 : 檢查用的溫度值 |
| `sRmsDownPath` | AnsiString | RMS download的位置 |
| `sRmsPath` | AnsiString | RMS的位置 |

<!-- preserved-content:end -->
