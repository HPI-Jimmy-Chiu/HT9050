# HT9045_CONFIG 欄位速查：群組 [A] ~ [G]

> 來源：`Config.h`（V3.33.900.0_20260331）
> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 全域欄位（非字母群組）

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bEnable_SECS_GEM` | bool | 啟用 SECS/GEM 通訊 |
| `bRCMDStart` | bool | SECS GEM Remote Start |
| `bSPILFunction` | bool | 矽品客戶統一功能旗標 |
| `bMaximFunction` | bool | Maxim 統一功能旗標 |
| `bSIGURDFunction` | bool | 矽格統一功能旗標 |
| `bVTESTFunction` | int | VTEST MES 系統 (0=停用) |
| `bKoreaFunction` | bool | 韓國代理商需求 |
| `bSingaporeFunction` | bool | 新加坡代理商需求 |
| `bASE_Report` | bool | ASE 高雄每顆 IC 履歷 |
| `bSmachineType` | AnsiString | 機台類型（for GPIB command） |
| `sGPIBMachineID` | AnsiString | GPIB Machine ID |
| `bPowerSaveFunction` | bool | 省電模式總開關 |
| `bEventLogAutoSaveFunction` | bool | 自動存 EventLog |
| `bHaveRotateShuttle` | bool | 旋轉蝦頭 |
| `bIndexEveryTimeCheckEP` | bool | Index 每次確認 EP 充飽 |
| `iUserLanguage` | int | 使用者語系 |
| `bUseAutoSiteMapping` | bool | 開啟 AutoSiteMapping |
| `bQAMode` | bool | QA 模式 |

---

## [A] 自動化 / 流程控制

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bA01AutoSwitchToOperatorMode` | bool | 超過時間自動切換 OP 模式 |
| `bA01PressStartAutoSwitchToOperatorMode` | bool | 按 Start 要切為 OP 權限（JCET） |
| `bA02DisableSaveParsWhenSwitchToOp` | bool | 登出後不可更改設定（A01_2） |
| `iA01ChangeOpTime` | int | 自動切換 OP 的閒置秒數 |
| `bA02BinModelPrime` | bool | Bin Setting like Epson |
| `bA05UseAutoDocking` | bool | Auto Docking 感測 |
| `bA09_ByArmCloseSite` | bool | 可用 Index Arm 關 Site |
| `bA09_1_AutoCloseArm` | bool | 單 Arm Site 全關時自動關 Arm |
| `bA10_AutoReTest` | bool | Auto Retest (ART) |
| `bA10TestModeForART` | bool | ART 需設為 32binGS |
| `iA10TestModeForART` | int | ART mode 代碼 |
| `bA10_5SCKART_AutoCorrection` | bool | SCK ART 自動修正 |
| `bA10_6_HANA_ART_TestMode_Enable` | bool | HANA ART 功能 |
| `bA10_7_Renesas_FTCT` | bool | 瑞薩 FTCT |
| `bA11BarcodeTime` | bool | BarCode Reader 持續時間啟用 |
| `iA11BarcodeTime` | int | BarCode Reader 持續時間(ms) |
| `bA12ClearLoaderDevice` | bool | Auto Retest 清除 Loader Device |
| `bA14UseBarCodeSetWorkFile` | bool | BarCode 讀取工作檔（AMKOR） |
| `bA15AutoDecayTest` | bool | Auto Decay Test |
| `bA15_1ESDGiveWayFunction` | bool | ESD 讓位功能 |
| `bA17RESETButtonDisable` | bool | RESET 按鍵功能控制 |
| `bA17_1RESETCleanOutWithoutTest` | bool | Reset→CleanOut 不進行檢測排料 |
| `bA19UsePMAlarmFunction` | bool | PM Alarm 功能 |
| `bA20_1CheckRTCFunction` | bool | Start 前比對 RTC |
| `bA20_2CheckTrayIDFunction` | bool | Start 前比對 Tray ID |
| `bA20_3CheckAutocleanFunction` | bool | Start 前比對 AutoClean |
| `bA20_4CheckContsFailFunction` | bool | Start 前比對 Cont Fail |
| `bA20_5CheckOCRFunction` | bool | Start 前比對 OCR |
| `bA22MagneticScale` | bool | 磁性尺功能 |
| `dA22MagneticScaleKeepRunRange` | double | 磁性尺容許繼續跑的範圍 |
| `dA22MagneticScaleStopRunRange` | double | 磁性尺需停機的範圍 |
| `bA23CheckLotNoInSLTReport` | bool | 必須輸入 LotNo 才能 Start |
| `bA24AutoBackupSetupFile` | bool | 自動備份工作檔 + Last Data |
| `asA25RunExecutFile` | AnsiString | 外部執行檔路徑（超豐） |
| `asA25RunExecutButtonName` | AnsiString | 外部執行檔按鈕名稱 |
| `bA26MotorSpeedSortDisplay` | bool | 馬達速度排序顯示 |
| `bA29EnableAutoCleanFunction` | bool | Auto Clean 開關 |
| `bA30SetupTeachFunction` | bool | Setup Teach 功能 |
| `bA31EnableAutoCleanIonFanFunction` | bool | IO 觸發 IonFan 清針 |
| `bA32EnableFTPAutomation` | bool | FTP 自動化（矽格） |
| `bA32EnableCheckList[eCL_Total]` | bool[] | FTP Automation 各項開關 |
| `asA32_1_HandlerID` | AnsiString | FTP Automation Handler ID |
| `bA32_2ReturnHandlerID2OI` | bool | 回傳 HandlerID 給 OI |
| `bA33SetICToErrBinAfterOutShtLossIC` | bool | OutShuttle Loss IC 放到 Error Bin |
| `bA35SetErrBinWhenOutShtLoseAndPickupErr` | bool | Out Sht Lose + Pick Err → Err Bin |
| `bA36OpenDoorSetErrBin` | bool | 開安全門分 Error Bin |
| `bA37LotStartLotEnd` | bool | SPIL ART LOT START/END Timeout |
| `bA39RecordRunState` | bool | Run Status Log（ASEM） |
| `iA39RecordTime` | int | Run Status 記錄間隔 |
| `bA50Enable1x4BiasYOffset` | bool | 1x4 Bias Y Offset（Tinton） |
| `bA51EnableEQCMode` | bool | EQC 模式功能開關 |
| `bA55EnablePMAlarmUpdateFromServerbyFTP` | bool | PM Alarm 由 Server FTP 更新 |
| `asA55PMAlarmUpdateFromServerbyFTP` | AnsiString | PM Alarm 更新路徑 |
| `bA56EnableAutoTeachFunciton` | bool | Auto Teach 自動對位功能 |
| `bA57_1SaveArmSpeedByMachine` | bool | Arm 速度存檔跟著機台 |
| `bA57_2SaveTemperatureByMachine` | bool | 溫度設定存檔跟著機台 |
| `bA57_3SaveOffsetByMachine` | bool | Offset 存檔跟著機台 |
| `bA60EnableAMR` | bool | AMR 自走車功能 |
| `iA60LoaderQtyAtOneTime` | int | AMR 一次補盤數量 |
| `bA61DisableCleanMUBA` | bool | Continuous Mode 關閉 Clean MUBA |
| `bA62bUseStopMachineArmHome` | bool | 停機時 In/Out Arm 歸零 |
| `bA65_BundleIDList` | bool | Bundle ID List 功能 |
| `bA66_2D_Sort` | bool | 2D Sort 功能 |
| `bA67TriggerOneCycleWhenAlarm` | bool | 特定 ALARM 觸發 ONE CYCLE（矽品彰化） |
| `bA68_AutoLoadUnload` | bool | Auto Load/Unload 功能 |
| `bA71UseBackupNowRecipe` | bool | 立即備份 Recipe |
| `sA71FolderPath` | AnsiString | Recipe 備份路徑 |

---

## [B] 報表 / 紀錄

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bB01_UsePrecautionRecordFunction` | bool | 保養記錄功能 |
| `iB01_AutoWakeupPrecautionRecordFormTime` | int | 自動喚醒保養記錄視窗時間 |
| `asB01_PrecautionRecordSavePath` | AnsiString | 保養記錄儲存路徑 |
| `bB02_HanderMajorMaintenanceRecordFunction` | bool | 主要保養記錄功能 |
| `asB02_HanderMajorMaintenanceRecordSavePath` | AnsiString | 主要保養記錄路徑 |
| `bB03_TesterReport` | bool | Tester Report（PTI） |
| `sB03_Customer` | AnsiString | Tester Report 客戶名 |
| `sB03_DeviceID` | AnsiString | Tester Report Device ID |
| `bB05_OSReport` | bool | OS Report |
| `sB05_OSReportPath` | AnsiString | OS Report 路徑 |
| `bB11UsePATServerFile` | bool | PAT Class 伺服器檔案 |
| `sB11PATServerPath` | AnsiString | PAT Server 路徑 |
| `bB12UsePATSetup` | bool | PAT Setup 功能 |
| `sB12PATSetupPath` | AnsiString | PAT Setup 路徑 |
| `sB13PATJobUploadPath` | AnsiString | PAT Job 上傳路徑 |
| `sB13PATJobDownloadPath` | AnsiString | PAT Job 下載路徑 |
| `iB14IntervalTime` | int | PAT 間隔時間 |
| `sB14RealTimePath` | AnsiString | PAT 即時路徑 |

---

## [C] 硬體選配

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bC01_FanDirection` | bool | 大風扇方向 |
| `bC02InstallCCD` | bool | 安裝 CCD |
| `bC03UseCatchTray` | bool | 使用 CatchTray |
| `bC04EnableTestTempIC` | bool | 溫測 IC |
| `bC05_PowerSaveTemp` | bool | 溫度省電模式 |
| `bC05_PowerSaveMotor` | bool | 馬達省電模式 |
| `iHaltTime_Motor` | int | 馬達省電進入時間(s) |
| `iHaltTime_Temp` | int | 溫控省電進入時間(s) |
| `bC05_PowerSaveVacuum` | bool | 真空幫浦省電（Steven 20221215） |
| `iC05HaltTime_Vacuum` | int | 真空幫浦省電計時(s) |
| `bC05_PowerSaveATC` | bool | ATC 省電（Ifor 20240401） |
| `iHaltTime_ATC` | int | ATC 省電計時(s) |
| `bC06_ByPassIonFan[MAX_IONFAN]` | bool[] | 不檢查離子風扇陣列 |
| `bC08_SocketSensor` | bool | Socket Sensor Detect（PiggyBack） |
| `bC08_1_CheckSocketSensorDetectON` | bool | Start 後偵測 Socket Sensor 啟動 |
| `bC09_CarRecord` | bool | Car Record 功能 |
| `bC11UseMonitorView` | bool | 錄影監視功能 |
| `bC12UsePEMode` | bool | PE Mode |
| `bC13NeedToRestartGroundWhenInitialStart` | bool | Initial Start 重啟 GroundMan（矽格北興） |
| `bC14SaveBinDisplayLog` | bool | BinDisplay Log 存檔 |
| `bC16UseBarCoderChangeSetupFile` | bool | 用 BarCode 切換工作檔 |
| `bC17UseLoaderColorSensor` | bool | Loader Color Sensor（MU-N） |
| `iC17DelayTimes` | int | Color Sensor Delay 次數 |
| `bC17_1_SkipAlarm` | bool | Color Sensor Skip Alarm |
| `bC20_1EnableEnergySavingDryAir` | bool | 省電乾燥空氣（HT-1032 TriTemp） |
| `iC20_2LowTempOffEnergySaving` | int | 低溫關省電門限溫度 |
| `iC20_3TempOverUseEnergySaving` | int | 高溫才開省電門限溫度 |
| `bC20_4IndexUseForstSensor` | bool | 露點計感測 |
| `iC21MotSp_HP1/HP2/SHT1/SHT2/AUTO1/2/3[3]` | int[] | 震動馬達調速（各位置三段速） |
| `bC24InitialStartCheckCylinder` | bool | Initial Start 檢查氣缸（JerryYang 20250120） |

---

## [D] Index / 下壓設定

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bD01EnableReadTorque` | bool | 扭力讀取 |
| `iD01ReadTorqueDelayTime` | int | 扭力讀取延遲時間 |
| `bD01_1EnableReadAndCheckTorque` | bool | 讀取並檢查扭力 |
| `bD04MinForceByFile` | bool | 最小壓力由檔案讀取 |
| `dD04MinForceByFile` | double | 最小壓力值 |
| `bD05ContactCountAlarm` | bool | 接觸次數超限報警 |
| `iD05_ContactCountAlarm` | int | 接觸次數上限 |
| `bD05_1SaveSocketCntByHandler` | bool | 接觸次數存檔跟著機台 |
| `bD06ContactOffsetDefaultValue` | bool | Contact Offset 預設值 |
| `bD11NoIcSkipAutoHeight` | bool | Shuttle 沒 IC 時跳過 Auto Height |
| `bD13CheckIndexHomeSensor` | bool | 歸零後檢查 Index 位置 |
| `bD14_AutoHeightUseSetTorque` | bool | Auto Height 用指定扭力 |
| `bD15_AutoContactTest` | bool | Auto Contact Test |
| `bD16_StepContactTest` | bool | Step by Step Contact Test |
| `bD21EnableFinishTestUpWait` | bool | 測試完成後上升等待 |
| `bD22SupportMultiDoubleContact` | bool | 支援多次 Double Contact |
| `bD22VerifyMode` | bool | Verify Mode（Sam 20221012） |
| `bD22DoubleContactUseDiffSRQ` | bool | Double Contact 使用不同 SRQ |
| `bD24EnableEPCheckFuntion` | bool | EP 充氣確認 |
| `bD26EnableEPEncoderRange` | bool | EP Encoder 範圍檢查 |
| `bD26_3EnableDualEPEncoderRange` | bool | Dual Force EP 範圍檢查 |
| `iD26EPEncoderRange` | int | EP Encoder 容許範圍 |
| `bD29EnableIndexContactDelay` | bool | Index 下壓後多 Delay（SPIL Low Yield） |
| `bD31RTCChangeRecipeNeedreCreateModel` | bool | RTC 換工作檔需重建 Model |
| `bD33RTCInitStartVerify` | bool | Initial Start 時 RTC 驗證 |
| `bD35RTCCheckSiteMap` | bool | RTC 比對 Site Map |
| `bD36EnableRTCAutoModelVerify` | bool | RTC 自動 Model 驗證 |
| `bD36_2AfterOneCycleNeedAutoVerify` | bool | One Cycle 後執行 RTC Auto Verify |
| `bD38IndexPutICToShtNoWaitMotion` | bool | Index 放 IC 到 Shuttle 不等馬達 |
| `bD41_Active` | bool | D41 Socket Position Check 勾選（Lock by File） |
| `bD41_Enable` | bool | D41 使用者可操作（Lock by File） |
| `iD41_Position` | int | D41 位置（Inside/Above） |
| `dD41_Offset` | double | D41 檢查高度 |
| `bD41CheckbySetup` | bool | D41 只能用一次（by setup） |
| `bD42_Active` | bool | D42 打勾（Lock by File） |
| `bD43IndexDropErrorCanRetryandSkip` | bool | Index 掉料可 Retry + Skip |
| `bD43AutoRetryWhenIndexPickErr` | bool | Index 吸取異常自動 Retry |
| `bD44_Active` | bool | D44 打勾（Lock by File） |
| `bD47EnableSocketPurgeFunction` | bool | Socket 吹氣清潔 |
| `iD47SocketPurgeCount` | int | Socket 吹氣次數 |
| `bD49RTCAlarmSetIndexToErrBin` | bool | RTC Alarm 時 Index 上 IC 放 Error Bin |
| `bD54SlowDown` | bool | 慢速模式 |
| `iD54SlowDownScale` | int | 慢速比例 |
| `bD55DisableIndexCheck` | bool | 關閉 Index Check |
| `bD59_32SitePnpTogether` | bool | 32Site 雙 Arm 一起吸放 |
| `bD62PickUpErrorNeedPurge` | bool | 吸取異常需吹氣一次 |
| `bD66Initialstartautoheight` | bool | Initial Start Auto Height |
| `bD70IndexCycleTimeRecord` | bool | Index Cycle Time 記錄 |
| `bD72NNModeMoveShtAfterContact` | bool | Index 下壓後才能移 Shuttle |
| `bD74RTCAutoTuning` | bool | RTC Auto Tuning（Sam 20230419） |
| `bD75OneCycleFinishedAlawayLearnRTCGolden` | bool | One Cycle 完成後 RTC Learn Golden |
| `bRTC_Active` | bool | RTC Lock by File 打勾（Sam 20240311） |
| `bRTC_Enable` | bool | RTC Lock by File 使用者可操作 |
| `bD78EnableIndexCheckHasICNeedPurge` | bool | Index Check 有 IC 需吹氣 |
| `bD79EnableIndexPickShuttleErrNeedPurge` | bool | Index 從 Shuttle 吸取異常需吹氣 |
| `bD82CheckIndexHasIC` | bool | 每次下壓確認 Socket 有 IC（Jimmychiu 20250826） |
| `GaliPosRange` | int | Index Y 超過範圍做一次 T-mode |

---

## [E] In/Out Arm 設定

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bE30InArmUseDifferentScale` | bool | In Arm 使用不同 Scale |
| `bE31OutArmUseDifferentScale` | bool | Out Arm 使用不同 Scale |
| `bE32ShuttleUseDifferentScale` | bool | Shuttle 使用不同 Scale |
| `bE33InOutArmZOffsetSameOne` | bool | In/Out Arm Z Offset 共用 |
| `bE41_35TrayPitchInOutSpeedSmall80Percent` | bool | Tray Pitch >35mm 時速度 ≤80% |
| `bE43AutoCleanUseHotplate` | bool | Auto Clean 使用加熱盤 |
| `bE45_AllSetupFileUseOneFile` | bool | 所有工作檔共用同一 Offset 檔 |
| `bE46_LoaderUse2Offset` | bool | Loader 前後排分別調 Offset |
| `bE47_ShuttleUse4Offset` | bool | Shuttle 前後排分別調 Offset |
| `bE49_LoaderOnlyRetryAndCleanOut` | bool | Loader 吸取異常只能 Retry/CleanOut |
| `iE50_OutArmPickUpErrorOption` | int | Out Arm 吸取異常的動作選項 |
| `bE51_EnableInArmZADC` | bool | In Arm Z 使用固定 ADC |
| `iE51_EnableInArmZADC` | int | In Arm Z ADC 值 |
| `bE52_EnableOutArmZADC` | bool | Out Arm Z 使用固定 ADC |
| `iE52_EnableOutArmZADC` | int | Out Arm Z ADC 值 |
| `bE53LowYieldAutoClean` | bool | Low Yield 時觸發 Auto Clean |
| `bE55UseFix3FullTray` | bool | 使用 Fix3 Full Tray |
| `bE56LoaderRetryAtSamePosition` | bool | Loader 吸取異常在同一位置 Retry |
| `bE59GroupOffsetFile` | bool | Offset 檔案用中括號做群組 |
| `bE64_50TrayPitchInOutSpeedSmall50Percent` | bool | Tray Pitch >50mm 時速度 ≤50% |
| `bE65_ClearTrayDataWhenOutArmDestoryErr` | bool | 掉料時清除 Unloader Tray 資料 |
| `bE66_LogHotPlateAction` | bool | 紀錄加熱盤動作 |
| `bE68RecheckInOutArmICFallDown` | bool | IC 掉落多檢查幾次再報警 |
| `iE68RecheckInOutArmICFallDown` | int | IC 掉落重複檢查次數 |
| `bE70_UseTrayThickAdjustZHeight` | bool | 用 Tray 厚度自動補償 Z 高度 |
| `bE73_InOutZStepMotorLossCheck` | bool | In/Out Z 步進馬達失步檢查 |
| `bE85_FillTray_Enable` | bool | Out Arm 放料後補滿 Tray |
| `bE85_FillTray_Auto1` | bool | 補滿 Auto1 |
| `bE85_FillTray_Auto3` | bool | 補滿 Auto3 |
| `bE30_1InArmUseDifferentScale_Hot` | bool | In Arm 高溫 Scale 不同（Ztex 20241118） |
| `bE31_1OutArmUseDifferentScale_Hot` | bool | Out Arm 高溫 Scale 不同 |
| `bE32_1ShuttleUseDifferentScale_Hot` | bool | Shuttle 高溫 Scale 不同 |

---

## [F] Shuttle 設定

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bF01ShakeShuttleWhenJam` | bool | Shuttle Jam 時搖一搖 |
| `iF01ShuttleShakeSpeed` | int | 搖一搖速度 |
| `bF05EnableShtPurgeFunction` | bool | Shuttle 吹氣清潔 |
| `iF05ShuttlePurgeCount` | int | Shuttle 吹氣次數 |
| `bF06InitialICCheck` | bool | Initial IC Check（Lock by File） |
| `bF06_Active` | bool | F06 打勾（Lock by File） |
| `bF06_Enable` | bool | F06 使用者可操作（Lock by File） |
| `bF11_Active` | bool | F11 打勾（Lock by File，Sam 20240202） |
| `bF11_Enable` | bool | F11 使用者可操作（Lock by File） |
| `bF11OutShtUseFrontRearSensor` | bool | Out Shuttle 使用前後感測器 |
| `bF14KnockShuttle` | bool | Shuttle 敲敲（防 IC 卡住） |
| `dF14KnockShuttleInterval` | double | Shuttle 敲敲間隔時間 |
| `iF14KnockShuttleNo` | int | Shuttle 敲敲次數 |
| `bF15OutShuttleLoseICNeedPWD` | bool | Out Shuttle Lose IC 需輸入密碼 |
| `bF16CheckShuttleSensorBroken` | bool | 確認 Shuttle 感測器是否斷線 |
| `bF17Sht1First` | bool | One Cycle 後先跑蝦頭 1 |
| `bF19OutShuttleLoseICNeedPiggyback` | bool | Out Shuttle Lose IC 觸發 Piggyback |
| `bF20InShuttleProminentDetect` | bool | In Shuttle 偵測 IC 放到外圍 |
| `bF22InShuttleDetectOutNoIC` | bool | Shuttle 出來後偵測有無殘 IC |
| `bF23ShuttleVibration` | bool | Shuttle 震動馬達 |
| `iF23ShuttleVibrationTime` | int | 震動時間 |
| `iF23ShuttleVibrationCount` | int | 震動次數 |
| `bF26_Enable` | bool | F26 Out Shuttle Jam 選項 Enable |
| `iF26OutShuttleJamSelectSkipOrRetry` | int | Out Shuttle Jam 選 Skip 或 Retry |
| `bF29AlwaysVibrateOnShuttle` | bool | 每次都強制震動 Shuttle |
| `bF31_CheckShtMoveCnt` | bool | 檢查 Shuttle 移動計數（HT-1032 TriTemp） |
| `bF32CheckInSHSenBypass` | bool | In SH Sensor Bypass 開關 |
| `bF33_Check2DHardware` | bool | 2DID 硬體順序檢查 |
| `bF35OutShtLoseNeedSetErrBin` | bool | Out Sht Lose → Error Bin |
| `bF36OutShtLoseICResetSetAllToErr` | bool | Out Sht Lose IC Reset 時全設 Error（JerryYang 20250120） |

---

## [G] UI 顯示

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bG01Show_TestRate` | bool | 顯示 Test Rate |
| `bG04ShowFailAlarmCount` | bool | 顯示 Fail Alarm 次數 |
| `bG05ShowSpeedMessage` | bool | 顯示速度訊息 |
| `bG07MultiColorForFailBin` | bool | 彩色 Fail Bin 顯示 |
| `bG08VisibleAreyousure` | bool | 顯示確認對話框 |
| `bG09NeedPasswordWhenEditSiteMap` | bool | 修改 Site Map 需要密碼 |
| `bG10ShowImmediateUPH` | bool | 顯示即時 UPH |
| `bG12ContractModeManualMessage` | bool | Contract Mode 手動提示訊息 |
| `bG14UseStartSoundAlarm` | bool | Start 時發出聲音提醒 |
| `iG14StartWarrTime` | int | Start 聲音持續時間(s) |
| `bG15LoadInputCount` | bool | 輸入達到顆數自動 Clean Out |
| `iG15KeyInTotal` | int | 目標抽測顆數 |
| `bG16BinDispNeedAlarm` | bool | Bin 顯示器異常 Alarm（JSCC） |
| `bG22NoticeTakeoutTray` | bool | 提醒人員取 Tray（JerryYang 20231218） |
| `bG23DiasbleFuncStatusView` | bool | 關閉功能狀態顯示（VTEST 開關） |
| `bG24DisableSECSGEMStatus` | bool | 關閉 SECS/GEM 狀態顯示（丁曉東要求） |
| `iUnloaderTrayCount[3]` | int[3] | Auto1/2/3 滿 Tray 補空盤判斷數量 |
