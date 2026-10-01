/* ht9045_wire_configconfiguration.js -- Config.Configuration.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cConfiguration.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * ⚠ 這一頁不是配方頁：它讀的是機台層設定檔，不是作用中配方。
 *   抽取到 1011 個欄位，其中 990 個在任何配方檔裡都不存在。
 *   所以本檔只提供小鍵盤，不提供讀寫。
 * 機台設定檔（/api/system/）config
 *   文字欄位     163 個（逐鍵比對過實檔確認存在）
 *   非文字控制項 0 個（radio group / checkbox / combo）
 *   契約有、本機實檔沒有 181 個（見檔尾 SYS ABSENT）
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 0 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 117 個欄位有 golden 依據
 * ⚠ 其中 4 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Config.Configuration.html',
  slug: 'configconfiguration',
  fields: {
  },
  sysFields: {
    EdF12:                         ['config', 'Rotate Shuttle', 'dRoShCheckDelayTime'],
    edA01:                         ['config', 'Function', 'iChangeOpTime'],
    edA10_2:                       ['config', 'Auto Retest Parameter', 'iAutoRetestLimit'],
    edA10_3:                       ['config', 'Auto Retest Parameter', 'iFailYieldRate_ART'],
    edA10_4:                       ['config', 'Auto Retest Parameter', 'A10_4_ARTTrayArmSpeed'],
    edA11:                         ['config', 'Barcode Reader', 'iBarcodeTime'],
    edA12:                         ['config', 'Function', 'iClearLoaderCount'],
    edA15_ESDReportTime:           ['config', 'Index', 'dESDDataReportTime'],
    edA22_2:                       ['config', 'Function', 'dMagneticScaleKeepRunRange'],
    edA22_3:                       ['config', 'Function', 'dMagneticScaleStopRunRange'],
    edA25_1:                       ['config', 'Function', 'asA25RunExecutFile'],
    edA25_2:                       ['config', 'Function', 'asA25RunExecutButtonName'],
    edA32_1:                       ['config', 'Function', 'asA32_1_HandlerID'],
    edA39:                         ['config', 'Function', 'iA39RecordTime'],
    edB01:                         ['config', 'PrecautionRecord', 'iB01_AutoWakeupPrecautionRecordFormTime'],
    edB01_1:                       ['config', 'PrecautionRecord', 'asB01_PrecautionRecordSavePath'],
    edB02:                         ['config', 'PrecautionRecord', 'asB02_HanderMajorMaintenanceRecordSavePath'],
    edC05_Motor:                   ['config', 'OutPowerSave', 'HaltTime_Motor'],
    edC05_Temp:                    ['config', 'OutPowerSave', 'HaltTime_Temp'],
    edC09:                         ['config', 'Car Record', 'iCarRecordDelayTime'],
    edD01:                         ['config', 'Index', 'iD01ReadTorqueTimeCount'],
    edD06:                         ['config', 'Contact Force', 'dD06_ContactOffsetDefaultValue'],
    edD14:                         ['config', 'Index', 'iD14_AutoHeightUseSetTorque'],
    edD21_Sec:                     ['config', 'Tester', 'iD21FinishTestUpWaitTime'],
    edD21_mm:                      ['config', 'Tester', 'iD21FinishTestUpWaitHeight'],
    edD23:                         ['config', 'Index', 'iMultiContactCount'],
    edD26:                         ['config', 'Index', 'iEPEncoderRange'],
    edD44:                         ['config', 'Index', 'iD44TestHeadCheckVacuumTime'],
    edD44_Height:                  ['config', 'Index', 'TestHeadCheckVacuumHeight'],
    edD46:                         ['config', 'Index', 'iD46WaitIndexDestroyTime'],
    edD47_Count:                   ['config', 'Index', 'iD47SocketPurgeCount'],
    edD47_Time:                    ['config', 'Index', 'iD47SocketPurgeTime'],
    edD53:                         ['config', 'Index', 'iD53LightOnMin'],
    edD54:                         ['config', 'Index', 'iSlowDownScale'],
    edD63:                         ['config', 'Index', 'iD63IndexZHomeToZPhaseRange'],
    edD68:                         ['config', 'Index', 'dD68DistanceRange'],
    edE36:                         ['config', 'In/Out Arm', 'iInArm60mmOffset'],
    edE37:                         ['config', 'In/Out Arm', 'iOutArm60mmOffset'],
    edE51:                         ['config', 'In/Out Arm', 'iE51_EnableInArmZADC'],
    edE52:                         ['config', 'In/Out Arm', 'iE52_EnableOutArmZADC'],
    edE68:                         ['config', 'In/Out Arm', 'iE68RecheckInOutArmICFallDown'],
    edE73:                         ['config', 'In/Out Arm', 'iE73StepMotorCheckCnt'],
    edF01:                         ['config', 'Shuttle', 'iF01ShuttleShakeSpeed'],
    edF05:                         ['config', 'Shuttle', 'iF05ShuttlePurgeCount'],
    edF13_ADC:                     ['config', 'Configuration', 'Rotate ADC'],
    edF13_Ini:                     ['config', 'Configuration', 'RotateInitSpeed'],
    edF13_Jog:                     ['config', 'Configuration', 'RotateJogHighSpeed'],
    edF14:                         ['config', 'Shuttle', 'dKnockShuttleInterval'],
    edF14_1:                       ['config', 'Shuttle', 'dF14KnockShuttleIntervalFirst'],
    edF14_1_No:                    ['config', 'Shuttle', 'iF14KnockShuttleNoFirst'],
    edF14_No:                      ['config', 'Shuttle', 'iKnockShuttleNo'],
    edF23:                         ['config', 'Shuttle', 'iF23ShuttleVibrationTime'],
    edF25:                         ['config', 'Shuttle', 'iF25VibrateTime'],
    edF31_1:                       ['config', 'F_Shuttle', 'F31_1_iShuttleMotorMoveCountSet'],
    edF31_2:                       ['config', 'F_Shuttle', 'F31_2_iShuttleMotorMoveCountSet'],
    edF31_3:                       ['config', 'F_Shuttle', 'F31_3_iShuttleMotorMoveCountNow'],
    edF31_4:                       ['config', 'F_Shuttle', 'F31_4_iShuttleMotorMoveCountNow'],
    edF31_5:                       ['config', 'F_Shuttle', 'F31_5_iShuttleMotorMoveCountHistroy'],
    edF31_6:                       ['config', 'F_Shuttle', 'F31_6_iShuttleMotorMoveCountHistroy'],
    edG14:                         ['config', 'Visible', 'iStartWarrTime'],
    edG18Auto1Count:               ['config', 'Visible', 'iAuto1Count'],
    edG18Auto2Count:               ['config', 'Visible', 'iAuto2Count'],
    edG18Auto3Count:               ['config', 'Visible', 'iAuto3Count'],
    edHeadCondition1:              ['config', 'O_Count', 'O14_ContactConditionName1'],
    edHeadCondition2:              ['config', 'O_Count', 'O15_ContactConditionName2'],
    edHeadCondition3:              ['config', 'O_Count', 'O16_ContactConditionName3'],
    edI21:                         ['config', 'Auto Site Mapping', 'Use Same Soak Time Sec'],
    edI21_6:                       ['config', 'Auto Site Mapping', 'iI21AutoSiteMappingErrCT'],
    edI21_9:                       ['config', 'Auto Site Mapping', 'iI21UseFailBinSetting'],
    edI22_1:                       ['config', 'Tester', 'fI22HomeDelay'],
    edI29:                         ['config', 'Function', 'fYieldRecordInterval'],
    edI29_3:                       ['config', 'Function', 'iI29YieldRecordIntervalIC'],
    edL03:                         ['config', 'Tempture', 'iL03SocketAirCoolingCT'],
    edL04:                         ['config', 'Tempture', 'iL04TemptureRange'],
    edL05:                         ['config', 'Tempture', 'iL05ChamberTemptureRange'],
    edL08_Over:                    ['config', 'Tempture', 'iSocketTemptureRange'],
    edL08_Under:                   ['config', 'Tempture', 'iSocketTemptureRangeUnder'],
    edL11_1:                       ['config', 'Index', 'iATCTemperatureRange'],
    edL11_1_2:                     ['config', 'Index', 'dATCTemperatureCheckTime'],
    edL11_2:                       ['config', 'Index', 'iATCChillerCheckTime'],
    edL11_4:                       ['config', 'Index', 'iATCTemperatureOverLimit'],
    edL11_6_Continuous:            ['config', 'Tempture', 'iATCTemperatureContinuous'],
    edL11_6_Outside:               ['config', 'Tempture', 'iATCTemperatureOutside'],
    edL11_7_MaxSurge:              ['config', 'Tempture', 'iATCMaxSurgeAlarm'],
    edL24:                         ['config', 'Index', 'iL24HeaterStableTime'],
    edN07_2:                       ['config', 'Function', 'iRunCheckAlarmTime'],
    edN07_5:                       ['config', 'SECS GEM', 'iEmployeeIdCheckTime'],
    edN08_2:                       ['config', 'OLP', 'OLP_IP'],
    edN08_3:                       ['config', 'OLP', 'OLP_Port'],
    edN10Host:                     ['config', 'FTPUpLoad', 'cN10FtpHost'],
    edN10UploadPath:               ['config', 'FTPUpLoad', 'cN10FtpUplaodPath'],
    edN10UserName:                 ['config', 'FTPUpLoad', 'cN10FtpUserName'],
    edN15_3:                       ['config', 'ESD_Control', 'N15_ESDControlMachineSaveRecordFilePath'],
    edN15_Host:                    ['config', 'ESD_Control', 'N15_ESDControlFTP_Host'],
    edN15_UserName:                ['config', 'ESD_Control', 'N15_ESDControlFTP_UserName'],
    edN17_2:                       ['config', 'Lot_Summary', 'asN17LotSummaryPath'],
    edN17_4:                       ['config', 'Production_Log', 'asN17ProductionLogPath'],
    edN23_2FTPHost:                ['config', '2DSotingFunction', 'cN23FtpHost'],
    edN23_2FTPPath:                ['config', '2DSotingFunction', 'cN23FtpDownloadPath'],
    edN23_2UserName:               ['config', '2DSotingFunction', 'cN23FtpUserName'],
    edN23_3NetDrivePath:           ['config', '2DSotingFunction', 'sN23DownloadDrivePath'],
    edN34:                         ['config', 'N34 Function', 'sN34_OEEAlarmRptPath'],
    edN35_1:                       ['config', 'Rround_ESD_Upload', 'sN35_FTPUserName'],
    edN35_3:                       ['config', 'Rround_ESD_Upload', 'sN35_FTPHost'],
    edN35_4:                       ['config', 'Rround_ESD_Upload', 'sN35_FTPUploadPath'],
    edO06_FilePath:                ['config', 'Event Log', 'AutoSaveEventLogPath'],
    edO07:                         ['config', 'Count', 'iFTMAXValue'],
    edP13_1:                       ['config', 'Tray', 'iP13EdgePushCylinderLoopDelay'],
    edP13_2:                       ['config', 'Tray', 'iP13EdgePushCylinderOnDelay'],
    edP14_1:                       ['config', 'Tray', 'iP14AutoTrayRecevieDelayCount'],
    edP14_2:                       ['config', 'Tray', 'iP14AutoTrayRecevieLoopDelayTime'],
    edP16_1:                       ['config', 'Hotplate', 'iHotplateEdgePushCylinderLoopDelay'],
    edP16_2:                       ['config', 'Hotplate', 'iHotplateEdgePushCylinderOnDelay'],
    edP23_1:                       ['config', 'P23', 'iOCRByNewTrayIntrvalTray'],
    edP23_2:                       ['config', 'P23', 'iOCRMaxInspDevices'],
    edP29:                         ['config', 'Function', 'dP29LoaderCheckIsFullInterval'],
    edP30:                         ['config', 'Function', 'iP30FixCheckRemainingAmountInterval'],
    edP48DelayTime:                ['config', 'Tray', 'iP48UnloaderCylinderLoopDelay'],
    edP48Times:                    ['config', 'Tray', 'iP48UnloaderCylinderLoopTimes'],
    edtA12_1:                      ['config', 'Function', 'iCleanLoaderOffset'],
    edtA28_1:                      ['config', 'Function', 'asA26_1ShowPmSopReadFilePath'],
    edtB05:                        ['config', 'Report', 'sB05_OSReportPath'],
    edtD04:                        ['config', 'Contact Force', 'dD04MinForceByFile'],
    edtD04_20mm:                   ['config', 'Contact Force', 'dD04MinForceByFile_20mm'],
    edtD04_30mm:                   ['config', 'Contact Force', 'dD04MinForceByFile_30mm'],
    edtD04_40mm:                   ['config', 'Contact Force', 'dD04MinForceByFile_40mm'],
    edtD04_60mm:                   ['config', 'Contact Force', 'dD04MinForceByFile_60mm'],
    edtD04_80mm:                   ['config', 'Contact Force', 'dD04MinForceByFile_80mm'],
    edtD17_3:                      ['config', 'Index', 'dD17_3CheckEPLeakage'],
    edtD47_5:                      ['config', 'Index', 'dD47_5_ContactOffset'],
    edtD47_6:                      ['config', 'Index', 'dD47_6_ShuttleOffset'],
    edtF23_1:                      ['config', 'Shuttle', 'iF23ShuttleVibrationCount'],
    edtI41_5:                      ['config', 'Tester', 'iI41_5_RegularExecutionCycleCount'],
    edtL06:                        ['config', 'Temperature', 'iAmbTemperatureRange'],
    edtN10_6:                      ['config', 'FTPUpLoad', 'iUploadToHostIntervalTime'],
    edtN10_8:                      ['config', 'FTPUpLoad', 'sN10UploadDrivePath'],
    edtN10_Port:                   ['config', 'FTPUpLoad', 'iN10FtpPort'],
    edtN14_1:                      ['config', 'Handler_OEE', 'N14_HandlerOEERecordCycleTime'],
    edtN14_2:                      ['config', 'Handler_OEE', 'N14_HandlerOEESaveProductionDataToPath'],
    edtN14_3Host:                  ['config', 'Handler_OEE', 'N14_HandlerOEEHost'],
    edtN14_3Path:                  ['config', 'Handler_OEE', 'N14_HandlerOEEUploadPath'],
    edtN14_3UserName:              ['config', 'Handler_OEE', 'N14_HandlerOEEUserName'],
    edtN14_4:                      ['config', 'Handler_OEE', 'N14_HandlerMODownloadPath'],
    edtN14_5:                      ['config', 'Handler_OEE', 'N14_PauseIntervalTimeSec'],
    edtN15_1:                      ['config', 'ESD_Control', 'N15_ESDControlUserLevelByTxtReadFilePath'],
    edtN15_2:                      ['config', 'ESD_Control', 'N15_ESDControlUseMachineReadFilePath'],
    edtN15_4:                      ['config', 'ESD_Control', 'N15_HandlerAUTOMOTIVEDownloadPath'],
    edtN25_2_Host:                 ['config', 'ChipMos Function', 'sN25_2_FTPHost'],
    edtN25_2_Name:                 ['config', 'ChipMos Function', 'sN25_2_FTPUserName'],
    edtN25_3_LogJamPath:           ['config', 'ChipMos Function', 'sN25_3_JamLogFTPPath'],
    edtN25_4_UploadPath:           ['config', 'ChipMos Function', 'sN25_4_UploadPath'],
    edtN25_5_UploadPath:           ['config', 'ChipMos Function', 'sN25_5_UploadPath'],
    edtO06AlarmHistroy:            ['config', 'Event Log', 'AutoSaveAlarmHistroyPath'],
    edtO06AlarmStatist:            ['config', 'Event Log', 'AutoSaveAlarmStatistPath'],
    edtO06Production:              ['config', 'Event Log', 'AutoSaveProductionPath'],
    edtO06_Local:                  ['config', 'Event Log', 'asAlarmRemoteDirectory'],
    edtO06_Remote:                 ['config', 'Event Log', 'asAlarmLocalDirectory'],
    edtO11:                        ['config', 'Count', 'Record Jam Rate Interval Time'],
    edtO16:                        ['config', 'Count', 'Con Alarm Need KeyIn Password CT'],
    edtO18:                        ['config', 'Count', 'iO18SafeDoorOnOffDurationHour'],
    edtO19_6:                      ['config', 'Event Log', 'asO19_SavePath'],
    lbledtN23_4_URL:               ['config', '2DID Search Function', 'sN23_4_URL'],
    lbledtN23_5_UploadPath:        ['config', '2DID White list', 'sN23_5_UploadPath'],
  },
  optional: {
  },
  pending: {
  },
  kb: {
    btnModifyHP:                 ['NO_SYMBOL', 0, false, 0, 0],
    btnModifyTray:               ['NO_SYMBOL', 0, false, 0, 0],
    edA22_2:                     ['DOUBLE', 2, true, 0.01, 10.0],
    edA22_3:                     ['DOUBLE', 2, false, 0, 0],
    edD25_30mm:                  ['DOUBLE', 2, false, 0, 0],
    edD25_40mm:                  ['DOUBLE', 2, false, 0, 0],
    edD25_60mm:                  ['DOUBLE', 2, false, 0, 0],
    edD60_56mm:                  ['DOUBLE', 2, false, 0, 0],
    edE30_HP1X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE30_HP1Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE30_HP2X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE30_HP2Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE30_LodX:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE30_LodY:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au3X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au3Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au4X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au4Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au5X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au5Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au6X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Au6Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi3X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi3Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi4X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi4Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi5X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi5Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi6X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_1_Fi6Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au3X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au3Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au4X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au4Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au5X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au5Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au6X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Au6Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi3X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi3Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi4X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi4Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi5X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi5Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi6X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_2_Fi6Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au1X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au1Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au2X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au2Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au3X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au3Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au4X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au4Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au5X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au5Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au6X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Au6Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi1X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi1Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi2X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi2Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi3X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi3Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi4X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi4Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi5X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi5Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi6X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE31_Fi6Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_IS1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_IS1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_IS2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_IS2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_OS1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_OS1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_OS2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_1_OS2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_IS1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_IS1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_IS2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_IS2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_OS1X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_OS1Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_OS2X:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_2_OS2Y:                ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_IS1X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_IS1Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_IS2X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_IS2Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_OS1X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_OS1Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_OS2X:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edE32_OS2Y:                  ['DOUBLE', 6, true, 0.95, 1.05],
    edN04_ID:                    ['NO_SPACE', 0, false, 0, 0],
    edN04_Model:                 ['NO_SPACE', 0, false, 0, 0],
    edN06_Port:                  ['PORT', 0, false, 0, 0],
    edN08_3:                     ['PORT', 0, false, 0, 0],
    edSetTemp:                   ['DOUBLE', 2, true, 0.0, 300.0],
    edtN09_TSV:                  ['PORT', 0, false, 0, 0],
    lbledtN24:                   ['PORT', 0, false, 0, 0],
  }
});

/* --- SYS ABSENT ------------------------------------------------------------
 * 對照表沒問題（來源是 golden 同一行的 WriteIniData*），但這台機器的
 * 實體設定檔裡目前沒有這個鍵 —— 多半是機型沒有那些 site/軸。
 * 與 PENDING 的差別：PENDING 是抽取抽錯，接了會寫錯地方；
 * 這裡只是本機缺鍵，上機台後鍵存在就該接上，重跑產生器即可。
 *
 *   ed14_24                    config     [Handler_OEE] iN14_24_DyMultiPassPower
 *   edA56_2                    config     [Function] iA56ShuttlePickUpOffsetWhenAutoTeach
 *   edA56_3                    config     [Function] iA56SocketPickUpOffsetWhenAutoTeachOnly
 *   edA60_1                    config     [Function] iA60NotifyQtyLoader
 *   edA60_2                    config     [Function] iA60NotifyQtyAuto1
 *   edA60_3                    config     [Function] iA60NotifyQtyAuto2
 *   edA60_4                    config     [Function] iA60NotifyQtyAuto3
 *   edA60_5                    config     [Function] iA60QtyAtOneTime
 *   edA71                      config     [Function] sA71FolderPath
 *   edB03_1                    config     [Report] sB03_Customer
 *   edB03_2                    config     [Report] sB03_DeviceID
 *   edB11PATServerPath         config     [PrecautionRecord] sB11PATServerPath
 *   edB12Path                  config     [PrecautionRecord] sB12PATSetupPath
 *   edB13DownloadPath          config     [PrecautionRecord] sB13PATJobDownloadPath
 *   edB13UploadPath            config     [PrecautionRecord] sB13PATJobUploadPath
 *   edB14IntervalTime          config     [PrecautionRecord] iB14IntervalTime
 *   edB14ReportRealTime        config     [PrecautionRecord] sB14RealTimePath
 *   edC05_ATC                  config     [OutPowerSave] HaltTime_ATC
 *   edC05_Vacuum               config     [OutPowerSave] HaltTime_Vacuum
 *   edD01DelayTime_Xilinx      config     [Index] dD01ReadTorqueDelayTime
 *   edD01_Xilinx               config     [Index] dD01ReadTorque
 *   edD05                      config     [Contact Force] iD05_ContactCountAlarm
 *   edD26_3                    config     [Index] iD26_3DualEPEncoderRange
 *   edD41                      config     [Index] fD41SocketInitialICCheckPositionOffset
 *   edD41_2                    config     [Index] fIndexCheckOffset
 *   edD62                      config     [Index] iD62IndexBlowAirTime
 *   edF32                      config     [Shuttle] iF30InSHSenBypassValue
 *   edI21_1                    config     [Auto Site Mapping] iI21FailRetryCount
 *   edI44                      config     [Tester] iI44_LowYieldAlarmIntervalTime
 *   edI49                      config     [Tester] fI49_ChangeAboveSocket
 *   edL32_4                    config     [Tempture] iL32_4SetDefrostTempature
 *   edL32_5                    config     [Tempture] iL32_5SetDefrostTime
 *   edL32_6                    config     [Tempture] iL32_6LowTempRunAlarmDegree
 *   edL32_7_1                  config     [Tempture] iL32_7LowTempRunAlarmHour
 *   edL32_7_2                  config     [Tempture] iL32_7LowTempRunAlarmMin
 *   edL32_8                    config     [Tempture] iL32_8SetAirStreamTemp
 *   edL33_3                    config     [Tempture] iL33_3BDoorOpenTimeForLowTemp
 *   edL33_4                    config     [Tempture] iL33_4SDoorOpenTimeForLowTemp
 *   edL33_5                    config     [Tempture] iL33_5DoorOpenTempForLowTemp
 *   edL33_6                    config     [Tempture] iL33_6DoorOpenTempForHotTemp
 *   edL34_2                    config     [Tempture] iL34_2DelaySecOfFixDoorOpen
 *   edL34_3                    config     [Tempture] dL34_3DewPointOfFixDoorOpen
 *   edL34_4                    config     [Tempture] iL34_4OpenAuto3TrackGateSec
 *   edL35_2                    config     [Tempture] iL35_2OpenFanTemp
 *   edL40                      config     [Tempture] iL40ImmediateTempExceedsAlarm
 *   edL41                      config     [Tempture] iL41TemperatureAlarmSecond
 *   edL49                      config     [Tempture] iL49UseBySetupOffsetRange
 *   edN06_FileName             config     [FTP] FTP File Name
 *   edN12_HostName             config     [Automation] Second FTP Host
 *   edN12_UpLdPath             config     [Automation] Second FTP Upload Path
 *   edN12_UserName             config     [Automation] Second FTP User Name
 *   edN14_20                   config     [Handler_OEE] asN14_20_DefaultRecipeChangeLogPath
 *   edN14_21                   config     [Handler_OEE] asN14_21_SetUpConfiguration
 *   edN14_21_BC                config     [Handler_OEE] asN14_21_BinCategory
 *   edN14_21_HP                config     [Handler_OEE] asN14_21_HP_SetUpConfiguration
 *   edN14_21_TF                config     [Handler_OEE] asN14_21_TF_SetUpConfiguration
 *   edN16_DownPath             config     [N16] cN16FtpDownloadPath
 *   edN16_HostName             config     [N16] cN16FtpHost
 *   edN16_UpLdPath             config     [N16] cN16FtpUplaodPath
 *   edN16_UserName             config     [N16] cN16FtpUserName
 *   edN21_1                    config     [bHandlerStateChangeUploadServer] sN21_FTPUserName
 *   edN21_3                    config     [bHandlerStateChangeUploadServer] sN21_FTPHost
 *   edN21_4                    config     [bHandlerStateChangeUploadServer] sN21_FTPUploadPath
 *   edN27_1                    config     [bUseAlarmLogXml] sN27_FTPUserName
 *   edN27_3                    config     [bUseAlarmLogXml] sN27_FTPHost
 *   edN27_4                    config     [bUseAlarmLogXml] sN27_FTPUplaodPath
 *   edN27_6                    config     [bUseAlarmLogXml] sN27_TesterID
 *   edN30_1                    config     [bRecordGroundESDByTestIC] sN30_FTPUserName
 *   edN30_3                    config     [bRecordGroundESDByTestIC] sN30_FTPHost
 *   edN30_4                    config     [bRecordGroundESDByTestIC] sN30_FTPUplaodPath
 *   edN30_5                    config     [bRecordGroundESDByTestIC] sN30_FTPUplaodPath2
 *   edN31_1                    config     [bAutoTmpeOfsByFTP] sN31_FTPUserName
 *   edN31_3                    config     [bAutoTmpeOfsByFTP] sN31_FTPHost
 *   edN31_4                    config     [bAutoTmpeOfsByFTP] sN31_FTPDownloadPath
 *   edN31_5                    config     [bAutoTmpeOfsByFTP] iN31_ContactCnt
 *   edN31_MaxOffset            config     [bAutoTmpeOfsByFTP] dN31_MaxOffset
 *   edN31_MinOffset            config     [bAutoTmpeOfsByFTP] dN31_MinOffset
 *   edN32_1                    config     [bDownloadUpdateAutomatically] sN32_FTPUserName
 *   edN32_3                    config     [bDownloadUpdateAutomatically] sN32_FTPHost
 *   edN32_4                    config     [bDownloadUpdateAutomatically] sN32_FTPDownloadPath
 *   edN32_5                    config     [bDownloadUpdateAutomatically] sN32_NetDownloadPath
 *   edN32_6                    config     [bDownloadUpdateAutomatically] sN32_FTPDownloadPath2
 *   edN33                      config     [LEADYO Function] asN33_UploadLogPath
 *   edN40_1                    config     [LEADYO Function] sN40FtpUserName
 *   edN40_3                    config     [LEADYO Function] sN40_FTPHost
 *   edN40_4                    config     [LEADYO Function] sN40_FTPUploadPath
 *   edN41_1                    config     [LEADYO Function] sN41_DiskUploadPath
 *   ed_N22_1_HostAddress       config     [ASECL_FTP] sN22_1ASE_CL_FTPHost
 *   ed_N22_1_UploadPath        config     [ASECL_FTP] sN22_1ASE_CL_FTPUplaodPath
 *   ed_N22_1_UserName          config     [ASECL_FTP] sN22_1ASE_CL_FTPUserName
 *   ed_N22_DownloadPath        config     [ASECL_FTP] sN22ASE_CL_FTPDownlaodPath
 *   ed_N22_HostAddress         config     [ASECL_FTP] sN22ASE_CL_FTPHostc
 *   ed_N22_UserName            config     [ASECL_FTP] sN22ASE_CL_FTPUserName
 *   ed_N35_1_HostAddress       config     [ASECL_FTP] sN35_1ASE_CL_FTPHost
 *   ed_N35_1_UploadPath        config     [ASECL_FTP] sN35_1ASE_CL_FTPUplaodPath
 *   ed_N35_1_UserName          config     [ASECL_FTP] sN35_1ASE_CL_FTPUserName
 *   edtC17                     config     [Function] iC20DelayTimes
 *   edtC21_Auto1               config     [C16Vibrate] C16VibrateMotSp_AUTO1
 *   edtC21_Auto1_b             config     [C16Vibrate] C16VibrateMotSp_AUTO1_B
 *   edtC21_Auto1_m             config     [C16Vibrate] C16VibrateMotSp_AUTO1_M
 *   edtC21_Auto2               config     [C16Vibrate] C16VibrateMotSp_AUTO2
 *   edtC21_Auto2_b             config     [C16Vibrate] C16VibrateMotSp_AUTO2_B
 *   edtC21_Auto2_m             config     [C16Vibrate] C16VibrateMotSp_AUTO2_M
 *   edtC21_Auto3               config     [C16Vibrate] C16VibrateMotSp_AUTO3
 *   edtC21_Auto3_b             config     [C16Vibrate] C16VibrateMotSp_AUTO3_B
 *   edtC21_Auto3_m             config     [C16Vibrate] C16VibrateMotSp_AUTO3_M
 *   edtC21_HP1                 config     [C16Vibrate] C16VibrateMotSp_HP1
 *   edtC21_HP1_b               config     [C16Vibrate] C16VibrateMotSp_HP1_B
 *   edtC21_HP1_m               config     [C16Vibrate] C16VibrateMotSp_HP1_M
 *   edtC21_HP2                 config     [C16Vibrate] C16VibrateMotSp_HP2
 *   edtC21_HP2_b               config     [C16Vibrate] C16VibrateMotSp_HP2_B
 *   edtC21_HP2_m               config     [C16Vibrate] C16VibrateMotSp_HP2_M
 *   edtC21_SHT1                config     [C16Vibrate] C16VibrateMotSp_SHT1
 *   edtC21_SHT1_b              config     [C16Vibrate] C16VibrateMotSp_SHT1_B
 *   edtC21_SHT1_m              config     [C16Vibrate] C16VibrateMotSp_SHT1_M
 *   edtC21_SHT2                config     [C16Vibrate] C16VibrateMotSp_SHT2
 *   edtC21_SHT2_b              config     [C16Vibrate] C16VibrateMotSp_SHT2_B
 *   edtC21_SHT2_m              config     [C16Vibrate] C16VibrateMotSp_SHT2_M
 *   edtL09_1                   config     [Tempture] iL09_1HightTemp_Sht_Shift
 *   edtL09_2                   config     [Tempture] iL09_2LowTemp_Sht_Shift
 *   edtL36_1                   config     [Tempture] iL36_1Tri_Temp_Rang_ATC
 *   edtL36_2                   config     [Tempture] iL36_2Tri_Temp_Rang_Heater
 *   edtL39_2                   config     [Tempture] iL39_2WaitTempstabilize
 *   edtL42                     config     [Tempture] iL42_UseOutShuttleDesoakTime
 *   edtL44                     config     [Tempture] iL44_SetColdAirSwitchTemp
 *   edtL45                     config     [Tempture] iL45_SetDewPointOffset
 *   edtN07_7                   config     [SECS GEM] iN07_7_DelayTime
 *   edtN09_5Host               config     [Automation] sN19_5_Host
 *   edtN09_5Path               config     [Automation] sN19_5_Path
 *   edtN09_5User               config     [Automation] sN19_5_User
 *   edtN09_7                   config     [Automation] sN19_7_SkipIP
 *   edtN09_Handler             config     [Automation] sN09_HandlerFolder
 *   edtN09_SearchTime          config     [Automation] dN09_SearchTime
 *   edtN09_TSV                 config     [Automation] iN09_TSV_Port
 *   edtN14_12                  config     [Handler_OEE] iN14_12_ULTempLogInterval
 *   edtN14_12_Path             config     [Handler_OEE] asN14_12_ULTempLogPath
 *   edtN14_13                  config     [Handler_OEE] iN14_13_ULBinQtyInterval
 *   edtN14_13_Path             config     [Handler_OEE] asN14_13_ULBinQtyPath
 *   edtN14_14_1                config     [Handler_OEE] asN14_14_ExecutFilePath
 *   edtN14_14_2                config     [Handler_OEE] asN14_14_MessageFilePath
 *   edtN14_14_3                config     [Handler_OEE] asN14_14_FlagFilePath
 *   edtN14_15_1                config     [Handler_OEE] asN14_15_ExecutFilePath
 *   edtN14_15_2                config     [Handler_OEE] asN14_15_MessageFilePath
 *   edtN14_16_1                config     [Handler_OEE] asN14_16_ExecutFilePath
 *   edtN14_16_2                config     [Handler_OEE] asN14_16_FlagFilePath
 *   edtN14_16_3                config     [Handler_OEE] asN14_16_ProductionFilePath
 *   edtN14_16_4                config     [Handler_OEE] iN14_16_IPSCInterval
 *   edtN14_17                  config     [Handler_OEE] iN14_17_AmbientULTemp
 *   edtN14_18                  config     [Handler_OEE] asN14_18_TempOffsetPath
 *   edtN14_19                  config     [Handler_OEE] asN14_19_TrayMappingPath
 *   edtN14_22Exp               config     [Handler_OEE] asN14_22_ConfigUpdateFromServerExport
 *   edtN14_22Imp               config     [Handler_OEE] asN14_22_ConfigUpdateFromServerImport
 *   edtN14_6_1                 config     [Handler_OEE] iN14_6_BySiteContactCnt
 *   edtN14_6_2                 config     [Handler_OEE] dN14_6_BySiteLowYieldRate
 *   edtN14_6_3                 config     [Handler_OEE] dN14_6_BySiteCmpYield
 *   edtN14_6_4                 config     [Handler_OEE] iN14_6_BySiteAlarmYieldRate
 *   edtN14_7                   config     [Handler_OEE] asN14_7_AutoMotivePath
 *   edtN14_8                   config     [Handler_OEE] asN14_8_ULSetupPath
 *   edtN14_9                   config     [Handler_OEE] asN14_9_ULQtyReportPath
 *   edtN23_2_LineID            config     [Murata Function] sN23_2_Line
 *   edtN23_2_ProcessName       config     [Murata Function] sN23_2_Process
 *   edtN23_2_Product           config     [Murata Function] sN23_2_Product
 *   edtN23_4                   config     [2DSotingFunction] sN23LotInfoPath
 *   edtN25_1_Host              config     [ChipMos Function] sN25_1_FTPHost
 *   edtN25_1_Name              config     [ChipMos Function] sN25_1_FTPUserName
 *   edtN25_1_Path              config     [ChipMos Function] sN25_1_FTPPath
 *   edtN25_2_Interval          config     [ChipMos Function] iN25_2_UploadInterval
 *   edtN25_2_Path              config     [ChipMos Function] sN25_2_FTPPath
 *   edtN26_1                   config     [JamRawDataUpdataToFTP] sN26_FTPUserName
 *   edtN26_3                   config     [JamRawDataUpdataToFTP] sN26_FTPHost
 *   edtN26_4                   config     [JamRawDataUpdataToFTP] sN26_FTPUplaodPath
 *   edtN28_IP                  config     [JSCK Function] sN26_IP
 *   edtN28_Path                config     [JSCK Function] sN26_Path
 *   edtN29                     config     [N29] sN29_FilePath
 *   edtO17_Count               config     [Count] iO17LevelUpWhenContiAlarmCount
 *   edtO17_Time                config     [Count] iO17LevelUpWhenContiAlarmTime
 *   edtP65                     config     [Function] iP65ArmQAModeValue
 *   lbledtN07_6                config     [SECS GEM] sN07_6OSRecipePath
 *   lbledtN23_1_URL            config     [Murata Function] sN23_1_URL
 *   lbledtN23_2_URL            config     [Murata Function] sN23_3_URL
 *   lbledtN24                  config     [Murata Function] iN24_RTMPort
 * --------------------------------------------------------------------------- */
