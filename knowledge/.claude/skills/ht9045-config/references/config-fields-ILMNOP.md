# HT9045_CONFIG 欄位速查：群組 [I] ~ [P]

> 來源：`Config.h`（V3.33.900.0_20260331）
> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## [I] 測試介面 / 良率

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bI01TesterFinishThenHome` | bool | 測試完成後歸零 |
| `bI02HomeSetSocketICToErrBin` | bool | 歸零時把測試中 IC 設為 Error Bin |
| `bI03AmbientTempControl` | bool | 常溫恆溫控制 |
| `bI04EnableChangeBinDuringTesting` | bool | 測試中可更改 Bin |
| `bI05LowYieldForcedOneCycle` | bool | Low Yield 強制 One Cycle |
| `bI06_Active` | bool | I06 打勾（Lock by File，矽格北興） |
| `bI06_Enable` | bool | I06 使用者可操作（Lock by File） |
| `bI06TurnOnI01AfterHome` | bool | Home 完成後強制開啟 I01 |
| `bI07ResetGPIBAfterOneCycleCleanOut` | bool | One Cycle CleanOut 後 Reset GPIB |
| `bI08Check2DIDEnableWhenInitialStart` | bool | Initial Start 時檢查 2DID 是否開啟 |
| `bI09LowYieldOneCycleDontCleanShuttle` | bool | Yield Alarm 觸發 Half One Cycle（Shuttle 保留IC） |
| `bI12TesterTimerOutNotNeedReTest` | bool | 測試 Time Out 不需要重測 |
| `bI16TTLSaveInSetupFile` | bool | TTL 設定存到工作檔 |
| `bI18CanReceiveEchoStop` | bool | 可接收 Echo Stop（ASE_KR） |
| `bI19AuToSitMapPauseWaitBin` | bool | AutoSiteMap 暫停等待 Bin 設定 |
| `iI20ErrorBinAlphabet` | int | Error Bin 字母代號 |
| `bI21EnableASM` | bool | 啟用 Auto Site Mapping |
| `iI21FailRetryCount` | int | ASM Fail 重試次數（KenHsieh 20251002） |
| `bI21SkipSoakTime` | bool | ASM 跳過 Soak Time |
| `bI21ASMRunTimeCHeck` | bool | 邊生產邊 ASM |
| `bI21AutoSiteMappingUseHotplate` | bool | ASM 使用加熱盤 |
| `bI21AutoSiteMappingFailBinSetting` | bool | ASM Fail Bin 設定 |
| `iI21UseFailBinSetting` | int | ASM Fail Bin 代碼 |
| `iI21AutoSiteMappingErrCT` | int | ASM Error 計數（VTEST） |
| `bI22TimeOutCanSkip` | bool | 測試 Time Out 可以 Skip |
| `iI22TestTimeOutOption` | int | Time Out 可按的按鈕選項 |
| `bI24TestingNeedStopAllMotor` | bool | 測試中停止所有馬達 |
| `bI26TestCloseSiteHaveBin` | bool | 測試時關 Site 出現 Bin 資料 |
| `bI27_ManualSortMode` | bool | 手動整盤功能（TSMC） |
| `bI28_OnOffSiteOnTheFly` | bool | 隨時開關 Site 功能 |
| `bI29EnableYieldRecord` | bool | Yield 記錄功能 |
| `bI29_1SaveYieldBySocketByBin` | bool | By Socket By Bin 存 Yield |
| `fI29YieldRecordInterval` | double | Yield 記錄間隔時間 |
| `bI29YieldRecordIntervalIC` | bool | 記錄 Total Yield（By IC 數） |
| `iI29YieldRecordIntervalIC` | int | IC 間隔數量 |
| `bI30ContFailBin` | bool | Fail Bin 超過數量發警告 |
| `bI31_1GPIBLotEnd` | bool | TSMC GPIB Lot End |
| `bI31_2GPIBLotStart` | bool | GPIB Lot Start Command |
| `bI31_3GPIBReset` | bool | GPIB Reset Command |
| `bI34AllSiteAreSameFailBinShowAlarm` | bool | 所有 Site 同樣 Fail Bin 跳 Alarm |
| `bI35UseThirdSiteControlByEngineer` | bool | 第三組工程師開關 Site |
| `bI36TestTimeOut` | bool | 沒收到測試資料需手動取出 IC |
| `bI37_EnableFIFOMode` | bool | FIFO Mode |
| `bI37_EnableFIFOSiteOrder` | bool | FIFO Site Order |
| `bI37_LockLoaderDirection` | bool | FIFO 鎖定 Loader 方向 |
| `iI37_LockLoaderDirection` | int | FIFO Loader 方向值 |
| `bI39SpiroxTesterLotEnd` | bool | SPIROX Tester Lot End |
| `bI41EnableEmptySocketCheck` | bool | Empty Socket Check 功能（RFMD） |
| `bI41_1_StartOfLot` | bool | Lot Start 時做 ESC |
| `bI41_2_OpenChamberDoor` | bool | 開 Chamber 門時做 ESC |
| `bI41_3_AfterContactorTeminated` | bool | Contactor 終止後做 ESC |
| `bI41_4_AfterContactorJam` | bool | Contactor Jam 後做 ESC |
| `bI41_5_RegularExecutionCycle` | bool | 定期執行 ESC |
| `iI41_5_RegularExecutionCycleCount` | int | ESC 執行間隔次數 |
| `iI41_BinOfESC` | int | ESC 的 Bin 代碼 |
| `bI42_bEnableBarcodeFlowErr` | bool | BarCode Flow Error 檢查 |
| `bI43ResetGPIBAfterTrayFeedFinish` | bool | Tray Feed 完成後 Reset GPIB |
| `bI44_LowYieldAlarmIntervalTimeBySetting` | bool | Low Yield Alarm 間隔時間可設定 |
| `iI44_LowYieldAlarmIntervalTime` | int | Low Yield Alarm 間隔時間(min) |
| `bI45_Use2DIDSort` | bool | 2D ID Sort 功能（ASEKH） |
| `iI46_ActionWhenGpibFlowErr` | int | GPIB Flow Error 動作選項 |
| `bI49_TesterTimeOutResetAllIC` | bool | Tester Time Out 時 Reset All IC |
| `bI50_EnableAutoSiteMappingTrigger` | bool | Auto Site Mapping Trigger 功能 |
| `bI50_StartLot` | bool | Lot Start 觸發 ASM |
| `bI50_InitialStart` | bool | Initial Start 觸發 ASM |
| `bI50_OnyCycle` | bool | One Cycle 觸發 ASM |
| `bI50_Pause` | bool | Pause 觸發 ASM |
| `bI50_TrayFeed` | bool | Tray Feed 觸發 ASM |
| `bI51_bNotSetErrBinForInput` | bool | 入料不設 Error Bin |
| `bI52_bAQLSortMode` | bool | AQL Sort Mode |
| `bI53_bKLTInitial` | bool | KLT Initial 功能 |
| `fI53KLTInitialInterval` | double | KLT One Cycle 未超過時間不執行 |
| `bI54_Enable` | bool | 測試中溫度確認功能（Jimmychiu 20240916） |

---

## [L] 溫控

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bL03SocketAirCoolingCT` | bool | Socket 冷卻次數計數 |
| `iL03SocketAirCoolingCT` | int | Socket 冷卻次數 |
| `iL04TemptureRange` | int | 溫度允許範圍 |
| `iL05ChamberTemptureRange` | int | Chamber 溫度允許範圍 |
| `bL07UseSingleTenmpertureLimit` | bool | 使用單一溫度限制 |
| `bL09HotTempShuttleNoAddPos` | bool | 高溫 Shuttle 不補位置（ASEKH） |
| `iL10TempRecordInterval` | int | 溫度記錄間隔 |
| `bL11_1ATCTemperatureOverAlarm` | bool | ATC 溫度過高 Alarm |
| `iATCTemperatureRange` | int | ATC 溫度過高容許範圍 |
| `dATCTemperatureCheckTime` | double | ATC 溫度過高持續時間(s) |
| `iATCTemperatureOverLimit` | int | ATC 最高上限溫度（L11-4） |
| `bL11_2ATCChillerProtectedFunction` | bool | ATC Chiller 保護 |
| `bL11_6ATCUseTemperatureOutsideAlarm` | bool | ATC 突波超出範圍 Alarm |
| `iATCTemperatureOutside` | int | ATC 突波超出門限值 |
| `iATCTemperatureContinuous` | int | ATC 突波持續時間(s) |
| `bL11_7ATCUseMaxSurgeAlarm` | bool | ATC 突波最大值 Alarm |
| `iATCMaxSurgeAlarm` | int | ATC 最大突波容許值 |
| `bL11_8ATCUseTemperatureCompare` | bool | ATC Sensor1 vs Sensor2 差值 Alarm |
| `bL12TempErrNoCloseHeater` | bool | 溫度 Error 不關閉加熱（SCK） |
| `bL13HotPlateAndShuttleUseOneTempOffset` | bool | 加熱盤與蝦頭使用同一溫補 |
| `bL17HeadHeaterOnWhenCloseSite` | bool | 關 Site 時也開啟 Head 加熱（ATK） |
| `bL18NofullsiteaddTemperatureoffset` | bool | 非全 Site 加溫補 |
| `bL20AbientGuardBand` | bool | Ambient Guard Band |
| `bL21PowerOffTemperature` | bool | PowerOff 開 Chamber 門斷所有加熱電 |
| `bL22Enable3SigmaTempMonitor` | bool | 3 Sigma 溫度統計監控（ASEKH） |
| `bL24HeaterStableTime` | bool | 加熱穩定待溫功能 |
| `iL24HeaterStableTime` | int | 待溫時間(s) |
| `bL28TempOfsUseReadyTempRange` | bool | 溫補使用 Ready Temp 範圍 |
| `bL29AmbientNotShowTemp` | bool | 常溫模式不顯示溫度 |
| `bL30Use1CableLayoutKitByConfig` | bool | 1 Cable Layout Kit by Config |
| `bL32_1ManuDefrost` | bool | 手動除霜（TriTemp/HT-1032） |
| `bL32_2AutoDefrostFunction` | bool | 自動除霜 |
| `bL32_3OneCycleDefrost` | bool | One Cycle 時除霜 |
| `iL32_4SetDefrostTemp` | int | 除霜門限溫度 |
| `iL32_5SetDefrostTime` | int | 除霜時間(min) |
| `iL32_6LowTempRunAlarmDegree` | int | 低溫運行 Alarm 溫度門限 |
| `iL32_8SetAirStreamTemp` | int | 氣流溫度設定 |
| `bL33_1CheckDoorOpenForTriTemp` | bool | TriTemp 門開檢查 |
| `bL34_1DelayOfFixDoorOpen` | bool | Fix 門開 Delay |
| `iL34_2DelaySecOfFixDoorOpen` | int | Fix 門開 Delay 秒數 |
| `dL34_3DewPointOfFixDoorOpen` | double | Fix 門開露點門限 |
| `bL35_1OverSetTempOpenFan` | bool | 超過設定溫度開風扇 |
| `iL35_2OpenFanTemp` | int | 開風扇溫度門限 |
| `bL38_1AirCoolerToCoolDown_SetEnable` | bool | 氣冷降溫功能 |
| `iL38_2AirCoolerToCoolDown_SetTemperature` | int | 開始氣冷降溫的溫度 |
| `bL39_1AutoRunWhenTempOk` | bool | 溫度達標後自動跑 |
| `bL42_UseOutShuttleDesoakTime` | bool | Out Shuttle Desoak Time 功能 |
| `iL42_UseOutShuttleDesoakTime` | int | Out Shuttle Desoak 時間(s) |
| `bL43EnableATCPowerFollow` | bool | ATC 電力跟隨功能（KenHsieh 20240216） |
| `bL44_SetColdAirSwitchTemp` | bool | 冷氣切換溫度設定 |
| `bL45_SetDewPointOffset` | bool | 設定露點 Offset（Ztex 20250401） |
| `iL45_SetDewPointOffset` | int | 露點 Offset 值 |
| `bL46_AStreamErrorCompressOnecycle` | bool | 氣流 Error 壓縮 One Cycle |

---

## [M] Monitor 強制模式

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bM01EnableMonitorFunction` | bool | 啟用 Monitor 功能（SCK） |
| `bM0101ContactModeUseDifferentSpeed` | bool | Contact Mode 強制使用不同速度 |
| `bM0102SiteYieldDifferentMustOn` | bool | Site Yield 不同時強制 ON |
| `bM0103ContinueFailBySocketMustOn` | bool | Cont Fail By Socket 強制 ON |
| `bM0104ContinueFailByHeadMustOn` | bool | Cont Fail By Head 強制 ON |
| `bM0105InOutArmDeviceCheckMustOn` | bool | In/Out Arm Device Check 強制 ON |
| `bM0106IndexDeviceCheckDestoryMustOn` | bool | Index Device Check Destroy 強制 ON |
| `bM0107AutoSpeedMustOn` | bool | Auto Speed 強制 ON |
| `bM0108EveryFirstDeviceMustOn` | bool | 每次第一顆 Delay 強制 ON |
| `bM0109RTCOffCheckYieldPiggyBack` | bool | RTC OFF 時檢查 Yield Piggyback |
| `bM1010Disable_I12` | bool | 強制關閉 I12（SCK） |
| `bM1012EnableAutoClean` | bool | 強制啟用 Auto Clean |
| `bM1013Enable2DID` | bool | 強制啟用 2DID |
| `bM1014EnableATC` | bool | 強制啟用 ATC |
| `bM1015EnableBottom2DID` | bool | 強制啟用 Bottom 2DID（JerryYang 20250120） |

---

## [N] 網路 / 上傳

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bN05SCKWebService` | bool | SCK Web Service |
| `bEnableFTP` | bool | N06 FTP 開關 |
| `FtpHost` | AnsiString | N06 FTP 主機 |
| `FtpUserName` | AnsiString | N06 FTP 帳號 |
| `FtpPassword` | AnsiString | N06 FTP 密碼 |
| `FtpDownloadPath` | AnsiString | N06 FTP 下載路徑 |
| `FtpUplaodPath` | AnsiString | N06 FTP 上傳路徑 |
| `N06_FtpPort` | AnsiString | N06 FTP 埠號 |
| `FtpTransMode` | int | N06 FTP 傳輸模式（Steven 20230719） |
| `TasterType/No/Name` | AnsiString | Taster 型號 / 號碼 / 名稱 |
| `bN07_EnableEmployeeIdCheak` | bool | SECS GEM 員工 ID 確認 |
| `iN07_EmployeeIdCheakTime` | int | 員工 ID 確認時間(min) |
| `bN07_EnableSecsLotCheck` | bool | SECS Lot Check |
| `bN07_6EnableUploadOSRecipe` | bool | OS 測試機工作檔上傳（Steven 20230710） |
| `bN10Enable_FTPUpLoadLog` | bool | N10 Log FTP 上傳啟用 |
| `iN10UploadMethod` | int | Log 上傳方式選擇 |
| `sN10UploadDrivePath` | AnsiString | Log 上傳網路硬碟路徑 |
| `cN10FtpHost/UserName/Password` | AnsiString | N10 FTP 主機/帳號/密碼 |
| `bN10_UploadSummaryToFTP` | bool | Tray Feed 時上傳 Summary |
| `bN10_DailyUploadProdData` | bool | 每日上傳 Event Log 等資料（JCET） |
| `bN12_EnableSocketIdProductDataFTP` | bool | Socket ID 生產資料 FTP（力成） |
| `bN13_EnableARMSFunction` | bool | ARMS 功能 |
| `bN14_1_EnableOEEFunction` | bool | OEE 功能啟用（超豐） |
| `iN14_1_OEERecordCycleTime` | int | OEE 記錄週期時間 |
| `bN14_3_OEEFTPUpload` | bool | OEE FTP 上傳 |
| `bN14_4_OEEAutoLoadMOFile` | bool | OEE 自動載入 MO 檔 |
| `bN14_7_AutoMotive` | bool | 自動 Motive 功能 |
| `bN14_10_DownFileByMO` | bool | 依 MO 下載工作檔 |
| `bN14_11_CheckSiteMapByMO` | bool | 依 MO 比對 Site Map |
| `bN14_18_EnableTempOffset` | bool | 溫度 By Servo 補償 |
| `bN14_21_SetUpConfiguration` | bool | SetUp Configuration 上傳 |
| `bN14_23_ReadTextFileforPassword` | bool | 從文字檔讀取密碼 |
| `bN15UserLevelByTxt` | bool | 使用者等級從文字檔讀取 |
| `bN15UseESDControlMachine` | bool | ESD 控制機台 |
| `bEnableOffsetFTP` | bool | N16 Offset FTP |
| `bN17UploadLotSummary` | bool | N17 上傳 Lot Summary |
| `bN20_CheckMD5` | bool | 工作檔 MD5 比對 |
| `bN22Enable_EventLog` | bool | ASE-CL EventLog 功能 |
| `bN23_1_Enable2DIDCompare` | bool | 2DID 比對功能（Murata） |
| `bN24_EnableRTM` | bool | JSCC RTM 功能 |
| `bN25_1_EnableStartControl` | bool | 自動 Start 控制（南茂） |
| `bN26_UseJamRawDataUpdataToFTP` | bool | Jam Raw Data 上傳到 FTP |
| `bN27_UseAlarmLogXmlUpdataToFTP` | bool | Alarm Log XML 上傳到 FTP |
| `bN29_ParameterCheckForGMTest` | bool | GM Test 工作檔比對 |
| `bN30_UseGroundESDUpdataToFTP` | bool | Ground + ESD 資料上傳 FTP（每顆） |
| `iN31_UseAutoTempOfsByFTP` | int | 溫度自動補償 By FTP |
| `bN32_DownloadUpdatesAutomatically` | bool | FTP 自動下載安裝更新包 |
| `bN32_CheckForUpdatesOnceDay` | bool | 每天自動檢查更新 |
| `sN32_FTPDownloadPath` | AnsiString | N32 更新包 FTP 路徑 |
| `bN33_UpLoadOCRBinLogByNet` | bool | 上傳 OCR + BIN Log |
| `bN34_GenerateOEEAlarmRpt` | bool | 產生 OEE Alarm 報告（CYUEAN） |
| `bN35_Ground_ESD_Upload` | bool | Ground + ESD 定期上傳（Sam 20250609） |
| `bN40_1_HandlerDataBackUpUseFunction` | bool | Handler 資料備份到 FTP |
| `bN41_1_HandlerDataBackUpToDiskUseFunction` | bool | Handler 資料備份到本機 |

---

## [O] Log / 紀錄

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bO01_ResetNeedClearAndCheckHP` | bool | Reset 後清空 InArm 資料並做 HP 檢查 |
| `bO02ResetNotClearPlate` | bool | Reset 不清除 Plate 資料 |
| `bO05ResetNeedRemoveAllTray` | bool | Reset 需移除所有 Tray |
| `bO06_EventLogAutoSave` | bool | EventLog 自動存檔 |
| `bEnableAlarmHistroyAutoSave` | bool | Alarm History 自動存 |
| `bEnableAlarmStatistAutoSave` | bool | Alarm 統計自動存 |
| `bEnableProductionAutoSave` | bool | Production Data 自動存 |
| `asEventLogAutoSavePath` | AnsiString | EventLog 存檔路徑 |
| `asAlarmHistroyAutoSavePath` | AnsiString | Alarm History 路徑 |
| `asProductionAutoSavePath` | AnsiString | Production Data 路徑 |
| `bO06SaveLogTimePeriod` | bool | Production Data 週期時間存檔 |
| `iO06SaveLogTimePeriod` | int | 週期時間（0=10min, 1=30min） |
| `bO10UseEventLogSaver` | bool | 使用外掛 EventLog Saver 程式 |
| `iO15_SaveFilePeriod` | int | EventLog 檔案存檔週期設定 |
| `bO15_EventLogFileNameWithMachineID` | bool | EventLog 檔名含 Machine ID |
| `bO15_EventLogSaveSameFolder` | bool | EventLog 放同一個資料夾（矽格湖口） |
| `bO16ConAlarmNeedKeyInPasswordCT` | bool | 連續相同 Alarm 需輸入密碼 |
| `iO16ConAlarmNeedKeyInPasswordCT` | int | 連續 Alarm 次數上限 |
| `bO17EnableLevelUpWhenContiAlarm` | bool | 連續 Alarm 提昇解除等級（逸昌） |
| `iO17LevelUpWhenContiAlarmCount` | int | 連續 Alarm 觸發次數 |
| `iO17LevelUpWhenContiAlarmTime` | int | 連續 Alarm 計算時間(s) |
| `bO18SafeDoorOnOffDurationDetect` | bool | 安全門長時間未開檢查機制 |
| `iO18SafeDoorOnOffDurationHour` | int | 安全門未開時間門限(hr) |
| `bO19_AutoRecordReportByEveryDay` | bool | 每日自動記錄報告（Sam 20210107） |
| `bO19_AutoRecordReportByEveryWeek` | bool | 每週自動記錄報告 |
| `bO19_AutoRecordReportByEveryMonth` | bool | 每月自動記錄報告 |
| `asO19_SavePath` | AnsiString | 報告存檔路徑 |
| `bO20InOutArmPickerLifeTimeCount` | bool | In/Out Arm 吸嘴壽命計數 |
| `bO20_1ClearLifeTimeWhenInitialStart` | bool | Initial Start 時清除壽命計數 |
| `bO22_ClearSortCntByDoubleClick` | bool | 雙擊清除排序數量（Steven 20241206） |
| `bO23_InputLotIDByBarcode` | bool | LotID 只能用 Barcode 輸入 |
| `bO24_ProductionLogByLot` | bool | Production Log By Lot（Steven 20250519） |

---

## [P] Tray / 料流

| 欄位 | 型別 | 說明 |
|------|------|------|
| `bP04ColorIsEmptyUnloader` | bool | Color Tray 視為 Empty Unloader |
| `bP05_LoaderCylinderPreOn` | bool | 預先打兩下 Loader 氣缸 |
| `bP06_LoaderUseCarrierTray` | bool | Loader 使用 Carrier Tray |
| `bP07NoTrayaAutoTrayFeed` | bool | Loader 沒 Tray 時自動 TrayFeed（TSMC） |
| `bP08TrayFeedCleanLotID` | bool | Tray Feed 時清除 Lot ID |
| `bP09TrayEndCanSelectTray` | bool | CleanOut 後可選擇 Tray End |
| `bP11RecordUPH` | bool | 記錄 UPH |
| `bP13EnableAutoTrayEdgePushCylinderLoop` | bool | Tray 邊緣推氣缸循環（敲敲） |
| `iP13EdgePushCylinderLoopDelay` | int | 敲 Tray 間隔時間(ms) |
| `bP14EnableAutoTrayRecevieDelayCount` | bool | Auto Tray 接收 Delay 計數 |
| `bP16EnableHotplateEdgePushCylinderLoop` | bool | Hotplate 邊緣推氣缸（敲 HP） |
| `bP17InArmFullPickFromLoader` | bool | In Arm 每次滿吸 Loader |
| `bP19CatchTrayUpThenCheck` | bool | CatchTray 異常先上升再確認 |
| `bP20ManualClearFixTrayDataAfterInitialStart` | bool | Initial Start 後不清 Tray 資料，需手動清除（Amkor） |
| `bP21CheckFixTray` | bool | Tray Feed 時偵測 Fix Tray |
| `bP21_2_LoaderTrayFeed` | bool | Tray Feed 包含 Loader Tray |
| `bP22EnableFirstTrayNeedAlarm` | bool | 第一盤入料前報警（ATK） |
| `bP24_Active` | bool | P24 打勾（Lock by File） |
| `bP24_Enable` | bool | P24 使用者可操作（Lock by File） |
| `bP24SkipEventNeedRemoveEmptyAndColorTray` | bool | Skip 事件需取出 Empty/Color Tray |
| `bP25EmptyColorNoSuppleAutoNoLoadEmpty` | bool | Empty/Color 補充前不載入空盤 |
| `bP26_OCRCheckLot` | bool | OCR 比對 Lot |
| `bP27AutoSortingBinTrayByOutArmwhenCleanOut` | bool | CleanOut 時 Auto Sorting（Out Arm） |
| `bP29LoaderCheckIsFull` | bool | 檢查 Loader 滿盤 |
| `bP30FixTryCheckRemainingAmount` | bool | Fix Tray 剩餘 IC 提前 Alarm |
| `bP32EmptyColorTrayPreAlarm` | bool | Empty/Color Tray 預警 |
| `bP33AutoTrayPreAlarm` | bool | Auto Tray 預警 |
| `bP35TrayArm` | bool | Tray Arm 需遮住 Home Sensor |
| `bP36BufferTrayNoSame` | bool | Load/Unload 強制不用同一軌道 |
| `bP37bAutoCylinderUP` | bool | Auto 1/2/3 氣缸常態在上 |
| `bP39ClearUnloaderTrayWhenInitial` | bool | Initial Start 時清除 Unloader Tray 資料 |
| `bP40TrayYSpeedByMachine` | bool | Tray Y 步進馬達參數跟著機台 |
| `bP41UnloadTrayDisableEdit` | bool | Unload Tray Disable Edit |
| `bP42AlarmWhenExitTrayComplete` | bool | 退 Tray 完成時報警 |
| `bP44LockLoaderTrayToNone` | bool | Auto Skip 後 Tray Arm 可搬 Tray（矽品中山） |
| `bP45LastLoaderNoInSide` | bool | Loader 空盤不 Tray |
| `bP46_LoadTrayModeByHandler` | bool | Loader Tray Mode 跟著機台 |
| `bP47_UseTrayThickAdjustZHeight` | bool | Tray 厚度自動補償 Z 高度 |
| `bP48UnloaderCylinderLoop` | bool | Unloader 氣缸循環 |
| `bP49UseLocalTraySpeed` | bool | 使用本機 Tray 速度 |
| `bP50DisabledAutoTrackSensorDetect` | bool | 關閉 Auto Track Sensor 偵測（矽格中興） |
| `bP52EmptyColorLastTrayCheck` | bool | Empty/Color 最後一盤檢查 |
| `bP53_ForcedScanBinCodeOfUnloader` | bool | 強制掃描 Unloader Bin Code |
| `bP55LdUldUseEmptyAndColorTray` | bool | Load/Unload 使用 Empty + Color Tray |
| `bP56TrayArmWaitAtColorTrack` | bool | Tray Arm 等待位置改到 Color |
| `bP57LoaderAutoCleanOutByInputCT` | bool | Loader 達到顆數自動 Clean Out |
| `iP58RunModeAfterFT` | int | FT 模式結束後切換模式 |
| `bP59UnloaderICFloattingAlarmAfterExit` | bool | Unloader 偵測置偏 IC 退出後再報警 |
| `bP60ReadClipCodeFromUnloader` | bool | 從 Unloader 讀 Clip Code |
| `bP61UseTrayTap` | bool | 使用 Tray Tap 功能（Ztex 20241112） |
| `bP62FirstTrayCheckOnUnloader` | bool | First Tray Check On Unloader（Jimmychiu 20251205） |
| `bP62Auto1/Auto2/Auto3` | bool | 對應 Auto1/2/3 做 First Tray Check |
| `bP62AlwaysEnabledAtLotStart` | bool | Lot Start 時永遠啟用 |
