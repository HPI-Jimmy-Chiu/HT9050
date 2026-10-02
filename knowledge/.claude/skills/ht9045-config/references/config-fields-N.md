# HT9045_CONFIG 欄位速查：群組 [N] Network（網路 / 上傳）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Nxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 282 個欄位，分屬 143 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| N05 | RMS Setting for Recipe File | — | `asN05_RTCalarmUnload` | AnsiString | — | -- | — | RMS Setting for Recipe File | jou 20170210 (Steven) : RTC alarm image unload |
| N05 | RMS Setting for Recipe File | `cbN05_SCKWebService` | `bN05SCKWebService` | bool | `EnableWebService` | -- | -- | RMS Setting for Recipe File | Steven 20161201 : For SCK Web Service |
| N05 | RMS Setting for Recipe File | — | `bN05_RTCalarmUnload` | bool | — | -- | — | RMS Setting for Recipe File | jou 20170210 (Steven) : RTC alarm image unload |
| N05 | RMS Setting for Recipe File | — | `iN05_UpDLMethod` | int | — | -- | — | RMS Setting for Recipe File | Jimmychiu 20250707 : add RMS connect method |
| N06 | FTP Setting for Recipe File | — | `asN06_FileName` | AnsiString | — | -- | — | FTP Setting for Recipe File | RogerYang 20170406 (Steven) for 力成 Jam Alarm Data 帶入自定義檔案名稱 |
| N06 | FTP Setting for Recipe File | — | `asN06_TesterPath` | AnsiString | — | -- | — | FTP Setting for Recipe File | Steven 20250327 : OS測試機的工作檔也要上傳 |
| N06 | FTP Setting for Recipe File | `chkN06_Tester` | `bN06_CopyTesterFile` | bool | `bN06_CopyTesterFile` | -- | -- | FTP Setting for Recipe File | Steven 20250327 : OS測試機的工作檔也要上傳 |
| N06 | FTP Setting for Recipe File | `cbN06_UseBarcode` | `bN06_UseBarcode` | bool | `Use Barcode Reader` | -- | -- | FTP Setting for Recipe File | Ifor 20210421 add: Use |
| N06 | FTP Setting for Recipe File | — | `N06_FtpPort` | AnsiString | — |  |  |  | Ifor 20201015 add:使用者自定義 FTP Port |
| N06 | FTP Setting for Recipe File | — | `N06_TasterListFile` | AnsiString | — |  |  |  | Steven 20110305 : Taster名稱對照表 |
| N06 | FTP Setting for Recipe File | — | `N06_TasterListMap` | AnsiString | — |  |  |  | Steven 20121018 : Handler與測試機連線的IP |
| N07-1 | Enable SECS GEM | `cbN07_EnableSecs` | `bEnable_SECS_GEM` | bool | `Enable SECS GEM` |  |  | Enable SECS GEM | jou 2012-03-12 Enable SECS_GEM |
| N07-2 | Enable host control start | `cbN07_EnableHostStart` | `bRCMDStart` | bool | `Enable RCMD START` |  |  | Enable host control start | Steven 20141006 : SECS GEM使用Remote Start功能 |
| N07-2 | Enable host control start | `edN07_2` | `iN07RunCheckAlarmTime` | int | `iRunCheckAlarmTime` |  |  | Enable host control start | wei 20150512  Run Check Alarm Time |
| N07-3 | When SECS GEM disconnect will auto one cycle | `cbN07_EnableSecsOneCycle` | `bSECS_GEM_OneCycle` | bool | `SECS GEM OneCycle` |  |  | When SECS GEM disconnect will auto one cycle | wei 20150824 Secs_Gem 斷線Onecycle |
| N07-3 | SECS GEM Alarm | `chkN07_3_2` | `bN07_Alarm` | bool | `SECS GEM Alarm` |  |  | SECS GEM disconnect alarm (JSCC NetworkMonitor) | Steven 20260603 : Secs_Gem 斷線 alarm（連線監測，閃爍紅框/塔燈/蜂鳴，V3.33.905.3） |
| N07-4 | Enable lot check | `cbN07_EnableSecsLotCheck` | `bN07_EnableSecsLotCheck` | bool | `N07_SecsLotCheck` |  |  | Enable lot check |  |
| N07-5 | Enable employee ID check | `cbN07_EnableEmployeeCheak` | `bN07_EnableEmployeeIdCheak` | bool | `Enable Employee ID Cheak` |  |  | Enable employee ID check | Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID |
| N07-5 | Enable employee ID check | `edN07_5` | `iN07_EmployeeIdCheakTime` | int | `iEmployeeIdCheckTime` |  |  | Enable employee ID check | Ifor 20180227 (Steven) add SECS GEM Confirm the Employee ID |
| N07-6 | S7F3 / S7F5 include OS Tester Recipe | `cbN07_6CompressedFile` | `bN07_6CompressedFile` | bool | `bN07_6CompressedFile` |  |  | S7F3 / S7F5 include OS Tester Recipe | JimmyChiu 20250214 : OS測試機的工作檔上傳選擇要不要壓縮 |
| N07-6 | S7F3 / S7F5 include OS Tester Recipe | `chkN07_6` | `bN07_6EnableUploadOSRecipe` | bool | `bN07_6EnableUploadOSRecipe` |  |  | S7F3 / S7F5 include OS Tester Recipe | Steven 20230710 : OS測試機的工作檔也要上傳 |
| N07-6 | S7F3 / S7F5 include OS Tester Recipe | — | `sN07_6OSRecipePath` | AnsiString | — |  | — | S7F3 / S7F5 include OS Tester Recipe | Steven 20230710 : OS測試機的工作檔也要上傳 |
| N07-7 | S7F3 / S7F5 Recipe file send as binary. | `chkN07_7` | `bN07_7SendRecipeAsBinary` | bool | `bN07_7SendRecipeAsBinary` |  |  | S7F3 / S7F5 Recipe file send as binary. | Steven 20230710 : 工作檔使用二進制上傳下載 |
| N07-7 | S7F3 / S7F5 Recipe file send as binary. | `edtN07_7` | `dN07_7_DelayTime` | double | `iN07_7_DelayTime` |  |  | S7F3 / S7F5 Recipe file send as binary. |  |
| N08-1 | Save communication logs. | `cbN08_1` | `bN08_1SaveOLPLog` | bool | `Save Log` |  | ECBool | Save communication logs. | Steven 20141229 : OLP的Log要存檔 |
| N08-2 |  | `edN08_2` | `sN08OlpIP` | AnsiString | `OLP_IP` |  |  |  | Sam 20190429 : Add CC_PTI_NEWWORK |
| N08-3 |  | `edN08_3` | `sN08OlpPort` | AnsiString | `OLP_Port` |  |  |  | Sam 20190429 : Add CC_PTI_NEWWORK |
| N09 | Lot count automation | `edtN09_SearchTime` | `dN09_SearchTime` | double | `dN09_SearchTime` | -- | -- | Lot count automation |  |
| N09 | Lot count automation | `edtN09_TSV` | `iN09_TSV_Port` | int | `iN09_TSV_Port` | -- | -- | Lot count automation |  |
| N09 | Lot count automation | `edtN09_Handler` | `sN09_HandlerFolder` | AnsiString | `sN09_HandlerFolder` | -- | -- | Lot count automation |  |
| N09-1 | Enable lot count automation function | `chkN09` | `bN09_LotCountAutoFunc` | bool | `bN09_LotCountAutoFunc` |  |  | Enable lot count automation function | Steven 20190521 : ATK lot count |
| N09-2 | Waiting TSV reply time out time (Sec) | `chkN09_2` | `bN09_Enable_TSV` | bool | `bN09_Enable_TSV` |  |  | Waiting TSV reply time out time (Sec) | Steven 20231017 : add for ATK |
| N09-4 | Upload Method | `rgN09_4` | `iN09_4_UploadMethod` | int | `iN09_4_UploadMethod` |  |  | Upload Method |  |
| N09-5 | FTP Setting | `edtN09_5Host` | `sN09_5_Host` | AnsiString | `sN19_5_Host` |  |  | FTP Setting |  |
| N09-5 | FTP Setting | `edtN09_5Password` | `sN09_5_Password` | AnsiString | `sN19_5_Password` |  |  | FTP Setting |  |
| N09-5 | FTP Setting | `edtN09_5Path` | `sN09_5_Path` | AnsiString | `sN19_5_Path` |  |  | FTP Setting |  |
| N09-5 | FTP Setting | `edtN09_5User` | `sN09_5_User` | AnsiString | `sN19_5_User` |  |  | FTP Setting |  |
| N09-7 | Skip IP addr start from (CSV format): | — | `bN09_7_SkipIP` | bool | — |  | — | Skip IP addr start from (CSV format): |  |
| N09-7 | Skip IP addr start from (CSV format): | `edtN09_7` | `sN09_7_SkipIP` | AnsiString | `sN19_7_SkipIP` |  |  | Skip IP addr start from (CSV format): |  |
| N10 | Log File Upload to Server | `chkN10_Passive` | `bN10FtpPassive` | bool | `bN10FtpPassive` | -- | -- | Log File Upload to Server |  |
| N10 | Log File Upload to Server | `edN10Host` | `cN10FtpHost` | AnsiString | `cN10FtpHost` | -- | -- | Log File Upload to Server |  |
| N10 | Log File Upload to Server | `edN10Password` | `cN10FtpPassword` | AnsiString | `cN10FtpPassword` | -- | -- | Log File Upload to Server |  |
| N10 | Log File Upload to Server | `edN10UploadPath` | `cN10FtpUplaodPath` | AnsiString | `cN10FtpUplaodPath` | -- | -- | Log File Upload to Server |  |
| N10 | Log File Upload to Server | `edN10UserName` | `cN10FtpUserName` | AnsiString | `cN10FtpUserName` | -- | -- | Log File Upload to Server |  |
| N10 | Log File Upload to Server | — | `iN10DataType` | int | — | -- | — | Log File Upload to Server | Sam 20170823 (wei) : 矽格中興 FTP Log 上傳增加時間格式選擇 0:yyyy 1:yyyymm 2:yyyymmdd |
| N10 | Log File Upload to Server | `edtN10_Port` | `iN10FtpPort` | int | `iN10FtpPort` | -- | -- | Log File Upload to Server |  |
| N10-1 | Enable Temperature && EP && ESD log upload to server. | `cbN10_1` | `bN10Enable_FTPUpLoadLog` | bool | `bEnable_FTPUpLoadLog` |  |  | Enable Temperature && EP && ESD log upload to server. | Ifor 20160302 add Enable FTP Up Load Log |
| N10-2 | Enable upload summary to server | `cbN10_2` | `bN10_UploadSummaryToFTP` | bool | `bN10_UploadSummaryToFTP` |  |  | Enable upload summary to server | JerryYang 20170804 (Steven) 日月新要求tray feed時要上傳Summary到FTP |
| N10-3 | Enable daily upload production status to server | — | `bN10_3_UpLoadByLot` | bool | — |  | — | Enable daily upload production status to server | Steven 20250527 : Upload data by lot. |
| N10-3 | Enable daily upload production status to server | `cbN10_3` | `bN10_DailyUploadProdData` | bool | `bN10_DailyUploadProdData` |  |  | Enable daily upload production status to server | Steven 20180514 : JCET吳如春要求每日上傳Event Log, Jam統計表, MTBF, MUBF資料 |
| N10-3 | Enable daily upload production status to server | — | `dN10_3_1_SpecifiedTime` | double | — |  | — | Enable daily upload production status to server | Jimmychiu 20250912 : CYUEAN wants to upload the log at a specified time. |
| N10-3-1 | Upload Time Priod Method | `rgN10_3_1` | `iN10UploadProductMethod` | int | `iN10UploadProductMethod` |  |  |  | JerryYang 20190131 上傳production log可選擇00:00 or 08:00 |
| N10-4 | Upload Method | `rgN10_4` | `iN10UploadMethod` | int | `iN10UploadMethod` |  |  | Upload Method | Steven 20190119 : Log上傳方式改為可選擇的 |
| N10-6 |  | `edtN10_6` | `iN10UploadToHostIntervalTime` | int | `iUploadToHostIntervalTime` |  |  | Interval time for upload to host                       s | Ifor 20160302 add Enable FTP Up Load Log |
| N10-8 | Net derive path | `edtN10_8` | `sN10UploadDrivePath` | AnsiString | `sN10UploadDrivePath` |  |  | Net derive path |  |
| N10-9 | Upload unloader tray data to FTP | `chkN10_9` | `bN10_9_UploadUnloadTrayToFTP` | bool | `bN10_9_UploadUnloadTrayToFTP` |  |  | Upload unloader tray data to FTP | Steven 20200409 : production log by unloader tray存檔並上傳FTP |
| N10-11 | Enable Upload EventLog | `cbN10_11` | `bN10_11_Enable_UploadFTPEventLog` | bool | `bN10_11_Enable_UploadFTPEventLog` |  |  | Enable Upload EventLog | Jimmychiu 20251112 : add upload eventlog to FTP server |
| N10-12 | Enable Upload GPIB | `cbN10_12` | `bN10_12_Enable_UploadFTPGPIBLog` | bool | `bN10_12_Enable_UploadFTPGPIBLog` |  |  | Enable Upload GPIB | Jimmychiu 20251112 : add upload GPIBLog to FTP server |
| N11-1 | Remote control clean out and close site. | `cbN11_1` | `bN11_1CleanOutCloseSite` | bool | `bN11_1CleanOutCloseSite` |  | ECBool | Remote control clean out and close site. | kevin 20160802 ASE send command Clean out close Site |
| N12 | Socket ID Product Data Upload To FTP | `edN12_HostName` | `asN12_FtpHost` | AnsiString | `Second FTP Host` | -- | -- | Socket ID Product Data Upload To FTP | Sam 20170525 力成 add Socket ID Product Data Upload To FTP |
| N12 | Socket ID Product Data Upload To FTP | `edN12_Password` | `asN12_FtpPassword` | AnsiString | `Second FTP Password` | -- | -- | Socket ID Product Data Upload To FTP | Sam 20170525 力成 add Socket ID Product Data Upload To FTP |
| N12 | Socket ID Product Data Upload To FTP | `edN12_UpLdPath` | `asN12_FtpUplaodPath` | AnsiString | `Second FTP Upload Path` | -- | -- | Socket ID Product Data Upload To FTP | Sam 20170525 力成 add Socket ID Product Data Upload To FTP |
| N12 | Socket ID Product Data Upload To FTP | `edN12_UserName` | `asN12_FtpUserName` | AnsiString | `Second FTP User Name` | -- | -- | Socket ID Product Data Upload To FTP | Sam 20170525 力成 add Socket ID Product Data Upload To FTP |
| N12 | Socket ID Product Data Upload To FTP | `chkN12` | `bN12_EnableSocketIdProductDataFTP` | bool | `Second Enable FTP` | -- | -- | Socket ID Product Data Upload To FTP | Sam 20170525 力成 add Socket ID Product Data Upload To FTP |
| N13 | ASEM Network Drive | `cbN13_EnableARMSFunction` | `bN13_EnableARMSFunction` | bool | `bN13_EnableARMSFunction` | -- | -- | ASEM Network Drive | Ifor 20170621 (wei) add ARMS Function |
| N14-1 |  | `cbN14_1` | `bN14_1_EnableOEEFunction` | bool | `N14_HandlerOEEUseFunction` |  |  | Use handler OEE function       Record cycle time |  |
| N14-1 |  | `edtN14_1` | `iN14_1_OEERecordCycleTime` | int | `N14_HandlerOEERecordCycleTime` |  |  | Use handler OEE function       Record cycle time |  |
| N14-2 | Save production data to path | `edtN14_2` | `asN14_2_OEESaveProdPath` | AnsiString | `N14_HandlerOEESaveProductionDataToPath` |  |  | Save production data to path |  |
| N14-2 | Save production data to path | `cbN14_2` | `bN14_2_OEEUseSaveProdData` | bool | `N14_HandlerOEEUseSaveProductionDataToPath` |  |  | Save production data to path |  |
| N14-3 | OEE FTP | `edtN14_3Host` | `asN14_3_OEEFTPHost` | AnsiString | `N14_HandlerOEEHost` |  |  | OEE FTP |  |
| N14-3 | OEE FTP | `edtN14_3Password` | `asN14_3_OEEFTPPassword` | AnsiString | `N14_HandlerOEEPassword` |  |  | OEE FTP |  |
| N14-3 | OEE FTP | `edtN14_3Path` | `asN14_3_OEEFTPUploadPath` | AnsiString | `N14_HandlerOEEUploadPath` |  |  | OEE FTP |  |
| N14-3 | OEE FTP | `edtN14_3UserName` | `asN14_3_OEEFTPUserName` | AnsiString | `N14_HandlerOEEUserName` |  |  | OEE FTP |  |
| N14-3 | OEE FTP | `cbN14_3` | `bN14_3_OEEFTPUpload` | bool | `N14_HandlerOEEFTPUpload` |  |  | OEE FTP |  |
| N14-4 | Handler OEE auto load MO file | `edtN14_4` | `asN14_4_MODownloadPath` | AnsiString | `N14_HandlerMODownloadPath` |  |  | Handler OEE auto load MO file |  |
| N14-4 | Handler OEE auto load MO file | `cbN14_4` | `bN14_4_OEEAutoLoadMOFile` | bool | `N14_HandlerOEEAutoLoadMOFile` |  |  | Handler OEE auto load MO file |  |
| N14-5 | Pause interval time (Sec) | `edtN14_5` | `iN14_5_PauseIntervalTime` | int | `N14_PauseIntervalTimeSec` |  |  | Pause interval time (Sec) |  |
| N14-6-1 |  | `edtN14_6_1` | `iN14_6_BySiteContactCnt` | int | `iN14_6_BySiteContactCnt` |  |  |  |  |
| N14-6-2 |  | `edtN14_6_2` | `dN14_6_BySiteLowYieldRate` | double | `dN14_6_BySiteLowYieldRate` |  |  |  |  |
| N14-6-3 |  | `edtN14_6_3` | `dN14_6_BySiteCmpYield` | double | `dN14_6_BySiteCmpYield` |  |  |  |  |
| N14-6-4 |  | `edtN14_6_4` | `iN14_6_BySiteAlarmYieldRate` | int | `iN14_6_BySiteAlarmYieldRate` |  |  |  |  |
| N14-7 | Check auto motive approve | `edtN14_7` | `asN14_7_AutoMotivePath` | AnsiString | `asN14_7_AutoMotivePath` |  |  | Check auto motive approve |  |
| N14-7 | Check auto motive approve | `cbN14_7` | `bN14_7_AutoMotive` | bool | `bN14_7_AutoMotive` |  |  | Check auto motive approve |  |
| N14-8 | Upload setup condition | `edtN14_8` | `asN14_8_ULSetupPath` | AnsiString | `asN14_8_ULSetupPath` |  |  | Upload setup condition |  |
| N14-8 | Upload setup condition | `cbN14_8` | `bN14_8_ULSetup` | bool | `bN14_8_ULSetup` |  |  | Upload setup condition |  |
| N14-9 | Upload device quantity compare report | `edtN14_9` | `asN14_9_ULQtyReportPath` | AnsiString | `asN14_9_ULQtyReportPath` |  |  | Upload device quantity compare report |  |
| N14-9 | Upload device quantity compare report | `cbN14_9` | `bN14_9_ULQtyReport` | bool | `bN14_9_ULQtyReport` |  |  | Upload device quantity compare report |  |
| N14-10 | Auto download setup file by MO | `cbN14_10` | `bN14_10_DownFileByMO` | bool | `N14_AutoDownloadSetupFileByMO` |  |  | Auto download setup file by MO |  |
| N14-11 | Do auto check site map by MO | `cbN14_11` | `bN14_11_CheckSiteMapByMO` | bool | `bN14_11_CheckSiteMapByMO` |  |  | Do auto check site map by MO |  |
| N14-12 | Temperature log upload to FTP | `edtN14_12_Path` | `asN14_12_ULTempLogPath` | AnsiString | `asN14_12_ULTempLogPath` |  |  | Temperature log upload to FTP |  |
| N14-12 | Temperature log upload to FTP | `cbN14_12` | `bN14_12_ULTempLogToFTP` | bool | `bN14_12_ULTempLogToFTP` |  |  | Temperature log upload to FTP |  |
| N14-12 | Temperature log upload to FTP | `edtN14_12` | `iN14_12_ULTempLogInterval` | int | `iN14_12_ULTempLogInterval` |  |  | Temperature log upload to FTP |  |
| N14-13 | Test bin quantity upload to FTP | `edtN14_13_Path` | `asN14_13_ULBinQtyPath` | AnsiString | `asN14_13_ULBinQtyPath` |  |  | Test bin quantity upload to FTP |  |
| N14-13 | Test bin quantity upload to FTP | `cbN14_13` | `bN14_13_ULBinQtyToFTP` | bool | `bN14_13_ULBinQtyToFTP` |  |  | Test bin quantity upload to FTP |  |
| N14-13 | Test bin quantity upload to FTP | `edtN14_13` | `iN14_13_ULBinQtyInterval` | int | `iN14_13_ULBinQtyInterval` |  |  | Test bin quantity upload to FTP |  |
| N14-14 | Use alarm control machine | `cbN14_14` | `bN14_14_AlarmCtrlMachine` | bool | `bN14_14_AlarmCtrlMachine` |  |  | Use alarm control machine |  |
| N14-14-1 |  | `edtN14_14_1` | `asN14_14_ExecutFilePath` | AnsiString | `asN14_14_ExecutFilePath` |  |  |  |  |
| N14-14-2 |  | `edtN14_14_2` | `asN14_14_MessageFilePath` | AnsiString | `asN14_14_MessageFilePath` |  |  |  |  |
| N14-14-3 |  | `edtN14_14_3` | `asN14_14_FlagFilePath` | AnsiString | `asN14_14_FlagFilePath` |  |  |  |  |
| N14-15 | Socket life time count control report upload | `cbN14_15` | `bN14_15_SocketLifeTime` | bool | `bN14_15_SocketLifeTime` |  |  | Socket life time count control report upload |  |
| N14-15-1 |  | `edtN14_15_1` | `asN14_15_ExecutFilePath` | AnsiString | `asN14_15_ExecutFilePath` |  |  |  |  |
| N14-15-2 |  | `edtN14_15_2` | `asN14_15_MessageFilePath` | AnsiString | `asN14_15_MessageFilePath` |  |  |  |  |
| N14-16 | IPSC control funciton | `cbN14_16` | `bN14_16_EnableIPSC` | bool | `bN14_16_EnableIPSC` |  | ECBool | IPSC control function |  |
| N14-16-1 |  | `edtN14_16_1` | `asN14_16_ExecutFilePath` | AnsiString | `asN14_16_ExecutFilePath` |  |  |  |  |
| N14-16-2 |  | `edtN14_16_2` | `asN14_16_FlagFilePath` | AnsiString | `asN14_16_FlagFilePath` |  |  |  |  |
| N14-16-3 |  | `edtN14_16_3` | `asN14_16_ProductionFilePath` | AnsiString | `asN14_16_ProductionFilePath` |  |  |  |  |
| N14-16-4 |  | `edtN14_16_4` | `iN14_16_IPSCInterval` | int | `iN14_16_IPSCInterval` |  |  |  |  |
| N14-17 | Ambient temperature upper (Temp) | `edtN14_17` | `iN14_17_AmbientULTemp` | int | `iN14_17_AmbientULTemp` |  |  | Ambient temperature upper (Temp) | Sam 20200525 : 正賢要求溫度 by Config 以下都是常溫。 |
| N14-18 | Temperature Tj offset by  tool database | `edtN14_18` | `asN14_18_TempOffsetPath` | AnsiString | `asN14_18_TempOffsetPath` |  |  | Temperature Tj offset by tool database | Sam 20200806 : 溫度 By Servo |
| N14-18 | Temperature Tj offset by  tool database | `cbN14_18` | `bN14_18_EnableTempOffset` | bool | `bN14_18_EnableTempOffset` |  |  | Temperature Tj offset by tool database | Sam 20200806 : 溫度 By Servo |
| N14-19 | Tray mapping log upload to FTP | `edtN14_19` | `asN14_19_TrayMappingPath` | AnsiString | `asN14_19_TrayMappingPath` |  |  | Tray mapping log upload to FTP | Sam 20201209 : 增加資料上傳 |
| N14-19 | Tray mapping log upload to FTP | `cbN14_19` | `bN14_19_TrayMappingToFTP` | bool | `bN14_19_TrayMappingToFTP` |  |  | Tray mapping log upload to FTP | Sam 20201209 : 增加資料上傳 |
| N14-20 | Default Recipe Chane Log | `edN14_20` | `asN14_20_ChangeLogPath` | AnsiString | `asN14_20_DefaultRecipeChangeLogPath` |  |  | Default Recipe Change Log | Sam 20201209 : Default Recipe ChangeLog |
| N14-20 | Default Recipe Chane Log | `cbN14_20` | `bN14_20_DefaultRecipeChangeLog` | bool | `bN14_20_DefaultRecipeChangeLog` |  |  | Default Recipe Change Log | Sam 20201209 : Default Recipe ChangeLog |
| N14-20-1 |  | `cbN14_20_1` | `bN14_20_DefaultRecipeChangeLogCycleRecord` | bool | `bN14_20_DefaultRecipeChangeLogCycleRecord` |  |  |  | Sam 20201209 : Default Recipe ChangeLog |
| N14-20-1 |  | `rgN14_20_1` | `iN14_20_CycleTime` | int | `iN14_20_CycleTime` |  |  |  | Sam 20201209 : Default Recipe ChangeLog |
| N14-21 | Set Up File Download | `edN14_21_BC` | `asN14_21_BinCategory` | AnsiString | `asN14_21_BinCategory` |  |  | Set Up File Download | JimmyChiu 20221226 : SetUpConfiguration HandlerMode splite to SITE MAP and BIN CATEGORY |
| N14-21 | Set Up File Download | `edN14_21_HP` | `asN14_21_HP_SetUpConfig` | AnsiString | `asN14_21_HP_SetUpConfiguration` |  |  | Set Up File Download | JimmyChiu 20220303 : Add class SetUpConfiguration |
| N14-21 | Set Up File Download | `edN14_21` | `asN14_21_SetUpConfiguration` | AnsiString | `asN14_21_SetUpConfiguration` |  |  | Set Up File Download | JimmyChiu 20220303 : Add class SetUpConfiguration |
| N14-21 | Set Up File Download | `edN14_21_TF` | `asN14_21_TF_SetUpConfig` | AnsiString | `asN14_21_TF_SetUpConfiguration` |  |  | Set Up File Download | JimmyChiu 20220303 : Add class SetUpConfiguration |
| N14-21 | Set Up File Download | `cbN14_21` | `bN14_21_SetUpConfiguration` | bool | `bN14_21_SetUpConfiguration` |  |  | Set Up File Download | JimmyChiu 20220303 : Add class SetUpConfiguration |
| N14-22 | Enable Config Update From Server | `edtN14_22Exp` | `asN14_22_ConfigUpdateFromServerExport` | AnsiString | `asN14_22_ConfigUpdateFromServerExport` |  |  | Enable Config Update From Server | JimmyChiu 20230410 : Config update from server |
| N14-22 | Enable Config Update From Server | `edtN14_22Imp` | `asN14_22_ConfigUpdateFromServerImport` | AnsiString | `asN14_22_ConfigUpdateFromServerImport` |  |  | Enable Config Update From Server | JimmyChiu 20230410 : Config update from server |
| N14-22 | Enable Config Update From Server | `cbN14_22` | `bN14_21_ConfigUpdateFromServerExport` | bool | `bN14_21_ConfigUpdateFromServerExport` |  |  | Enable Config Update From Server | JimmyChiu 20230410 : Config update from server |
| N14-23 | Read Text File for Password | `cb14_23` | `bN14_23_ReadTextFileforPassword` | bool | `bN14_23_ReadTextFileforPassword` |  |  | Read Text File for Password | JimmyChiu 20240115 : Read Text File for Password |
| N14-24 | Dynamic multiplier for Continual Pass Bin( Socket ) | `cb14_24` | `bN14_24_DynaMultiContinuPassSocket` | bool | `bN14_24_DynaMultiContinuPassSocket` |  |  | Dynamic multiplier for Continual Pass Bin( Socket ) | JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket ) |
| N14-24 | Dynamic multiplier for Continual Pass Bin( Socket ) | — | `iN14_24_DyMultiPassPower` | int | — |  | — | Dynamic multiplier for Continual Pass Bin( Socket ) | JimmyChiu 20240411 : Dynamic multiplier for Continual Pass Bin( Socket ) |
| N15 | User Level By txt And ESD Control Machine | `edN15_Host` | `asN15ESDFTP_Host` | AnsiString | `N15_ESDControlFTP_Host` | -- | -- | User Level By txt And ESD Control Machine |  |
| N15 | User Level By txt And ESD Control Machine | `edN15_Password` | `asN15ESDFTP_Password` | AnsiString | `N15_ESDControlFTP_Password` | -- | -- | User Level By txt And ESD Control Machine |  |
| N15 | User Level By txt And ESD Control Machine | `edN15_UserName` | `asN15ESDFTP_UserName` | AnsiString | `N15_ESDControlFTP_UserName` | -- | -- | User Level By txt And ESD Control Machine |  |
| N15-1 |  | `edtN15_1` | `asN15UserLevelByTxtReadFilePath` | AnsiString | `N15_ESDControlUserLevelByTxtReadFilePath` |  |  |  |  |
| N15-1 |  | `cbN15_1` | `bN15UserLevelByTxt` | bool | `N15_ESDControlUserLevelByTxt` |  |  |  |  |
| N15-2 |  | `edtN15_2` | `asN15UseESDControlMachineReadFilePath` | AnsiString | `N15_ESDControlUseMachineReadFilePath` |  |  |  |  |
| N15-2 |  | `cbN15_2` | `bN15UseESDControlMachine` | bool | `N15_ESDControlUseMachine` |  |  |  |  |
| N15-3 |  | `edN15_3` | `asN15ESDControlMachineSaveRecordFilePath` | AnsiString | `N15_ESDControlMachineSaveRecordFilePath` |  |  |  |  |
| N15-4 |  | `edtN15_4` | `asN15HandlerAUTOMOTIVEDownloadPath` | AnsiString | `N15_HandlerAUTOMOTIVEDownloadPath` |  |  |  |  |
| N16 | Offset FTP | `chkN16` | `bEnableOffsetFTP` | bool | `bEnableOffsetFTP` | -- | -- | Offset FTP | wei 20170821 offset ftp |
| N16 | Offset FTP | `edN16_DownPath` | `cN16FtpDownloadPath` | AnsiString | `cN16FtpDownloadPath` | -- | -- | Offset FTP |  |
| N16 | Offset FTP | `edN16_HostName` | `cN16FtpHost` | AnsiString | `cN16FtpHost` | -- | -- | Offset FTP |  |
| N16 | Offset FTP | `edN16_Password` | `cN16FtpPassword` | AnsiString | `cN16FtpPassword` | -- | -- | Offset FTP |  |
| N16 | Offset FTP | `edN16_UpLdPath` | `cN16FtpUplaodPath` | AnsiString | `cN16FtpUplaodPath` | -- | -- | Offset FTP |  |
| N16 | Offset FTP | `edN16_UserName` | `cN16FtpUserName` | AnsiString | `cN16FtpUserName` | -- | -- | Offset FTP |  |
| N17-1 | Upload lot summary | `cbN17_1` | `bN17UploadLotSummary` | bool | `bN17UploadLotSummary` |  |  | Upload lot summary | JerryYang 20220923 : [N17] upload lot summary |
| N17-2 | Net derive path | `edN17_2` | `asN17LotSummaryPath` | AnsiString | `asN17LotSummaryPath` |  |  | Net derive path | JerryYang 20220923 : [N17] upload lot summary |
| N17-3 | Upload production log daily | `cbN17_3` | `bN17UploadProdLog` | bool | `bN17UploadProdLog` |  |  | Upload production log daily |  |
| N17-4 | Net derive path | `edN17_4` | `asN17ProductionLogPath` | AnsiString | `asN17ProductionLogPath` |  |  | Net derive path |  |
| N19-5 |  | — | `sN19_5_Host` | AnsiString | — |  | — |  |  |
| N19-5 |  | — | `sN19_5_Password` | AnsiString | — |  | — |  |  |
| N19-5 |  | — | `sN19_5_Path` | AnsiString | — |  | — |  |  |
| N19-5 |  | — | `sN19_5_User` | AnsiString | — |  | — |  |  |
| N19-7 |  | — | `bN19_7_SkipIP` | bool | — |  | — |  |  |
| N19-7 |  | — | `sN19_7_SkipIP` | AnsiString | — |  | — |  |  |
| N20 | Check Sum Function | `chkN20` | `bN20_CheckMD5` | bool | `bN20_CheckMD5` |  |  | Check Sum Function | Steven 20170927 (wei) : 比對工作檔的檢查碼是否正確 |
| N21-1 |  | `chkN21_1` | `bN21_HandlerChangeStateUploadServer` | bool | `bN21_HandlerChangeStateUploadServer` |  |  |  |  |
| N21-1 |  | `edN21_1` | `sN21_FTPUserName` | AnsiString | `sN21_FTPUserName` |  |  |  |  |
| N21-2 |  | `edN21_2` | `sN21_FTPPassword` | AnsiString | `sN21_FTPPassword` |  |  |  |  |
| N21-3 |  | `edN21_3` | `sN21_FTPHost` | AnsiString | `sN21_FTPHost` |  |  |  |  |
| N21-4 |  | `edN21_4` | `sN21_FTPUploadPath` | AnsiString | `sN21_FTPUploadPath` |  |  |  |  |
| N22 | ASE-CL FTP Function | — | `sN22ASE_CL_FTPDownlaodPath` | AnsiString | — | -- | — | ASE-CL FTP Function |  |
| N22 | ASE-CL FTP Function | — | `sN22ASE_CL_FTPHost` | AnsiString | — | -- | — | ASE-CL FTP Function |  |
| N22 | ASE-CL FTP Function | — | `sN22ASE_CL_FTPPassword` | AnsiString | — | -- | — | ASE-CL FTP Function |  |
| N22 | ASE-CL FTP Function | — | `sN22ASE_CL_FTPUserName` | AnsiString | — | -- | — | ASE-CL FTP Function |  |
| N22-1 | Enable FTP Function | `cbN22` | `bN22Enable_ASE_CL_FTP` | bool | `bN22Enable_ASE_CL_FTP` |  |  | Enable FTP Function |  |
| N22-1 | Enable FTP Function | `cbN22` | `bN22_1_HANA_TrayMapFTP` | bool | `bN22_1_HANA_TrayMapFTP` |  |  | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1ASE_CL_FTPHost` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1ASE_CL_FTPPassword` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1ASE_CL_FTPUplaodPath` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1ASE_CL_FTPUserName` | AnsiString | — |  | — | Enable FTP Function | JerryYang 20250120 : add |
| N22-1 | Enable FTP Function | — | `sN22_1_FTPHost` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1_FTPPassword` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1_FTPUploadPath` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-1 | Enable FTP Function | — | `sN22_1_FTPUserName` | AnsiString | — |  | — | Enable FTP Function |  |
| N22-2 | Enable Event Log | `cbN22_EveltLog` | `bN22Enable_EventLog` | bool | `bN22Enable_EventLog` |  |  | Enable Event Log | Steven 20181224 : For ASE-CL |
| N22-3 | Upload JHT format log | `cbN23` | `bN23UploadJHT_Log` | bool | `bN23UploadJHT_Log` |  |  |  | JerryYang 20250120 : add |
| N23 | 2DID Sorting Mode | — | `sN23_JHT_UploadPath` | AnsiString | — | -- | — | 2DID Sorting Mode |  |
| N23-1 | Enable Function | `chkN23_1` | `bN23_1_Enable2DIDCompare` | bool | `bN23_1_Enable2DIDCompare` |  |  | 2DID comparasion function |  |
| N23-1 | Enable Function | `rgN23_1_2DSorting` | `iN23DownloadMethod` | int | `iN23DownloadMethod` |  |  | 2DID comparasion function | JerryYang 20190313 : 2D sorting |
| N23-1 | Enable Function | — | `sN23_1_URL` | AnsiString | — |  | — | 2DID comparasion function |  |
| N23-2 | Production setting | `edN23_2FTPPath` | `cN23FtpDownloadPath` | AnsiString | `cN23FtpDownloadPath` |  |  | Production setting |  |
| N23-2 | Production setting | `edN23_2FTPHost` | `cN23FtpHost` | AnsiString | `cN23FtpHost` |  |  | Production setting |  |
| N23-2 | Production setting | `edN23_2Pwd` | `cN23FtpPassword` | AnsiString | `cN23FtpPassword` |  |  | Production setting |  |
| N23-2 | Production setting | `edN23_2UserName` | `cN23FtpUserName` | AnsiString | `cN23FtpUserName` |  |  | Production setting |  |
| N23-2 | Production setting | `edtN23_2_LineID` | `sN23_2_Line` | AnsiString | `sN23_2_Line` |  |  | Production setting |  |
| N23-2 | Production setting | `edtN23_2_ProcessName` | `sN23_2_Process` | AnsiString | `sN23_2_Process` |  |  | Production setting |  |
| N23-2 | Production setting | `edtN23_2_Product` | `sN23_2_Product` | AnsiString | `sN23_2_Product` |  |  | Production setting |  |
| N23-3 | Net derive path | `chkN23_3` | `bN23_3_UploadTestResult` | bool | `bN23_3_UploadTestResult` |  |  | Upload test result |  |
| N23-3 | Net derive path | `edN23_3NetDrivePath` | `sN23DownloadDrivePath` | AnsiString | `sN23DownloadDrivePath` |  |  | Upload test result |  |
| N23-3 | Net derive path | — | `sN23_3_URL` | AnsiString | — |  | — | Upload test result |  |
| N23-4 | Get lot info from file | `chkN23_4` | `bN23UseLotInfoFile` | bool | `bN23UseLotInfoFile` |  |  | Download 2DID list path | Steven 20240829 : Lot info從檔案讀取 |
| N23-4 | Get lot info from file | `edtN23_4` | `sN23LotInfoPath` | AnsiString | `sN23LotInfoPath` |  |  | Download 2DID list path |  |
| N23-4 | Get lot info from file | — | `sN23_4ASE_CL_FTPDownlaodPath` | AnsiString | — |  | — | Download 2DID list path |  |
| N23-4 | Get lot info from file | — | `sN23_4ASE_CL_FTPHost` | AnsiString | — |  | — | Download 2DID list path |  |
| N23-4 | Get lot info from file | — | `sN23_4ASE_CL_FTPPassword` | AnsiString | — |  | — | Download 2DID list path |  |
| N23-4 | Get lot info from file | — | `sN23_4ASE_CL_FTPUserName` | AnsiString | — |  | — | Download 2DID list path |  |
| N23-4 | Get lot info from file | — | `sN23_4_URL` | AnsiString | — |  | — | Download 2DID list path | JerryYang 20241104 : 支援2DID白名單功能 |
| N23-5 | Summary folder without YY\MM | `chkN23_5` | `bN25FolderWithoutYYMM` | bool | `bN25FolderWithoutYYMM` |  |  | Upload 2DID white list | Steven 20240830 : Summary資料夾不要有年月 |
| N23-5 | Summary folder without YY\MM | — | `sN23_5_UploadPath` | AnsiString | — |  | — | Upload 2DID white list |  |
| N24 | Enable function | `chk24` | `bN24_EnableRTM` | bool | `bN24_EnableRTM` | -- | -- | RTM control function |  |
| N24 | Enable function | — | `iN24_RTMPort` | int | — | -- | — | RTM control function |  |
| N25-1 | Auto Start FTP | `chkN25_1` | `bN25_1_EnableStartControl` | bool | `bN25_1_EnableStartControl` |  |  | Auto Start FTP |  |
| N25-1 | Auto Start FTP | `edtN25_1_Host` | `sN25_1_FTPHost` | AnsiString | `sN25_1_FTPHost` |  |  | Auto Start FTP |  |
| N25-1 | Auto Start FTP | `edtN25_1_Password` | `sN25_1_FTPPassword` | AnsiString | `sN25_1_FTPPassword` |  |  | Auto Start FTP |  |
| N25-1 | Auto Start FTP | `edtN25_1_Path` | `sN25_1_FTPPath` | AnsiString | `sN25_1_FTPPath` |  |  | Auto Start FTP |  |
| N25-1 | Auto Start FTP | `edtN25_1_Name` | `sN25_1_FTPUserName` | AnsiString | `sN25_1_FTPUserName` |  |  | Auto Start FTP |  |
| N25-2 | Temperature log FTP | `chkN25_2` | `bN25_2_EnableUploadLog` | bool | `bN25_2_EnableUploadLog` |  |  | Temperature log FTP |  |
| N25-2 | Temperature log FTP | `edtN25_2_Interval` | `iN25_2_UploadInterval` | int | `iN25_2_UploadInterval` |  |  | Temperature log FTP |  |
| N25-2 | Temperature log FTP | `edtN25_2_Host` | `sN25_2_FTPHost` | AnsiString | `sN25_2_FTPHost` |  |  | Temperature log FTP |  |
| N25-2 | Temperature log FTP | `edtN25_2_Password` | `sN25_2_FTPPassword` | AnsiString | `sN25_2_FTPPassword` |  |  | Temperature log FTP |  |
| N25-2 | Temperature log FTP | `edtN25_2_Path` | `sN25_2_FTPPath` | AnsiString | `sN25_2_FTPPath` |  |  | Temperature log FTP |  |
| N25-2 | Temperature log FTP | `edtN25_2_Name` | `sN25_2_FTPUserName` | AnsiString | `sN25_2_FTPUserName` |  |  | Temperature log FTP |  |
| N25-3 | Jam log FTP | `chkN25_3` | `bN25_3_EnableULJamLog` | bool | `bN25_3_EnableULJamLog` |  |  | Jam log FTP | JimmyChiu 20241009 : for 南茂Jam List上傳 |
| N25-3 | Jam log FTP | `edtN25_3_LogJamPath` | `sN25_3_JamLogFTPPath` | AnsiString | `sN25_3_JamLogFTPPath` |  |  | Jam log FTP |  |
| N25-4 | Summary Count FTP | `chkN25_4` | `bN25_4_EnableUpload` | bool | `bN25_4_EnableUpload` |  |  | Summary Count FTP | JimmyChiu 20241009 : for 南茂Jam List上傳 |
| N25-4 | Summary Count FTP | `edtN25_4_UploadPath` | `sN25_4_UploadPath` | AnsiString | `sN25_4_UploadPath` |  |  | Summary Count FTP |  |
| N25-5 | Upload EventLog | `chkN25_5` | `bN25_5_EnableUpload` | bool | `bN25_5_EnableUpload` |  |  | Upload EventLog | JimmyChiu 20241009 : for 南茂Jam List上傳 |
| N25-5 | Upload EventLog | `edtN25_5_UploadPath` | `sN25_5_UploadPath` | AnsiString | `sN25_5_UploadPath` |  |  | Upload EventLog |  |
| N26-1 |  | `chkN26_1` | `bN26_UseJamRawDataUpdataToFTP` | bool | `bN26_UseJamRawDataUpdataToFTP` |  |  |  |  |
| N26-1 |  | `edtN26_1` | `sN26_FTPUserName` | AnsiString | `sN26_FTPUserName` |  |  |  |  |
| N26-2 |  | `chkN26_2` | `bN26_UseJamRawDataRecord` | bool | `bN26_UseJamRawDataRecord` |  |  |  |  |
| N26-2 |  | `edtN26_2` | `sN26_FTPPassword` | AnsiString | `sN26_FTPPassword` |  |  |  |  |
| N26-3 |  | `edtN26_3` | `sN26_FTPHost` | AnsiString | `sN26_FTPHost` |  |  |  |  |
| N26-4 |  | `edtN26_4` | `sN26_FTPUplaodPath` | AnsiString | `sN26_FTPUplaodPath` |  |  |  |  |
| N27-1 |  | `cbN27_1` | `bN27_UseAlarmLogXmlUpdataToFTP` | bool | `bN27_UseAlarmLogXmlUpdataToFTP` |  |  |  |  |
| N27-1 |  | `edN27_1` | `sN27_FTPUserName` | AnsiString | `sN27_FTPUserName` |  |  |  |  |
| N27-2 |  | `edN27_2` | `sN27_FTPPassword` | AnsiString | `sN27_FTPPassword` |  |  |  |  |
| N27-3 |  | `edN27_3` | `sN27_FTPHost` | AnsiString | `sN27_FTPHost` |  |  |  |  |
| N27-4 |  | `edN27_4` | `sN27_FTPUplaodPath` | AnsiString | `sN27_FTPUplaodPath` |  |  |  |  |
| N27-6 |  | `edN27_6` | `sN27_TesterID` | AnsiString | `sN27_TesterID` |  |  |  |  |
| N28 | OEE Function | `chkN28` | `bN28_SCK_OEE` | bool | `bN26_SCK_OEE` | -- | -- | OEE Function |  |
| N28 | OEE Function | `edtN28_IP` | `sN28_IP` | AnsiString | `sN26_IP` | -- | -- | OEE Function |  |
| N28 | OEE Function | `edtN28_Path` | `sN28_Path` | AnsiString | `sN26_Path` | -- | -- | OEE Function |  |
| N29 | Enable important  parameter check function | `chkN29` | `bN29_ParameterCheckForGMTest` | bool | `bN29_ParameterCheck` | -- | -- | Enable important parameter check function | Steven 20220311 : GM Test工作檔比對功能 |
| N29 | Enable important  parameter check function | `edtN29` | `sN29_FilePath` | AnsiString | `sN29_FilePath` | -- | -- | Enable important parameter check function |  |
| N30-1 |  | `cbN30_1` | `bN30_UseGroundESDUpdataToFTP` | bool | `bN30_UseGroundESDUpdataToFTP` |  |  |  | Sam 20211223 : 每顆 IC 測試完畢都要記錄當時的 Ground & ESD 數值。 |
| N30-1 |  | `edN30_1` | `sN30_FTPUserName` | AnsiString | `sN30_FTPUserName` |  |  |  |  |
| N30-2 |  | `edN30_2` | `sN30_FTPPassword` | AnsiString | `sN30_FTPPassword` |  |  |  |  |
| N30-3 |  | `edN30_3` | `sN30_FTPHost` | AnsiString | `sN30_FTPHost` |  |  |  |  |
| N30-4 |  | `edN30_4` | `sN30_FTPUploadPath` | AnsiString | `sN30_FTPUplaodPath` |  |  |  |  |
| N30-5 |  | `edN30_5` | `sN30_FTPUploadPath2` | AnsiString | `sN30_FTPUplaodPath2` |  |  |  | Sam 20220816 : GroundESD 新增第二組上傳 |
| N31 | Auto temperature offset | `edN31_MaxOffset` | `dN31_MaxOffset` | double | `dN31_MaxOffset` | -- | -- | Auto temperature offset |  |
| N31 | Auto temperature offset | `edN31_MinOffset` | `dN31_MinOffset` | double | `dN31_MinOffset` | -- | -- | Auto temperature offset | Jimmychiu 20241226 : add N31 temp offset limit |
| N31 | Auto temperature offset | `cbN31_1` | `iN31_UseAutoTempOfsByFTP` | int | `bN31_UseAutoTempOfsByFTP` | -- | -- | Auto temperature offset | Sam 20220406 : 溫度自動補償功能 By FTP |
| N31-1 |  | `edN31_1` | `sN31_FTPUserName` | AnsiString | `sN31_FTPUserName` |  |  |  |  |
| N31-2 |  | `edN31_2` | `sN31_FTPPassword` | AnsiString | `sN31_FTPPassword` |  |  |  |  |
| N31-3 |  | `edN31_3` | `sN31_FTPHost` | AnsiString | `sN31_FTPHost` |  |  |  |  |
| N31-4 |  | `edN31_4` | `sN31_FTPDownloadPath` | AnsiString | `sN31_FTPDownloadPath` |  |  |  |  |
| N31-5 |  | `edN31_5` | `iN31_ContactCnt` | int | `iN31_ContactCnt` |  |  |  |  |
| N32 | Download updates HT9045 automatically | — | `asN32_UploadLogPath` | AnsiString | — | -- | — | Download updates HT9045 automatically |  |
| N32 | Download updates HT9045 automatically | `rgN32` | `iN32_DownloadMode` | int | `iN32_DownloadMode` | -- | -- | Download updates HT9045 automatically |  |
| N32-1 |  | `cbN32_1` | `bN32_DownloadUpdatesAutomatically` | bool | `bN32_DownloadUpdatesAutomatically` |  |  |  | Sam 20220824 : FTP 自動下載安裝更新包 |
| N32-1 |  | `edN32_1` | `sN32_FTPUserName` | AnsiString | `sN32_FTPUserName` |  |  |  |  |
| N32-2 |  | `cbN32_2` | `bN32_CheckForUpdatesOnceDay` | bool | `bN32_CheckForUpdatesOnceDay;` |  |  |  |  |
| N32-2 |  | `edN32_2` | `sN32_FTPPassword` | AnsiString | `sN32_FTPPassword` |  |  |  |  |
| N32-3 |  | `cbN32_3` | `bN32_CheckAtInitailStart` | bool | `bN32_CheckAtInitailStart;` |  |  |  |  |
| N32-3 |  | `edN32_3` | `sN32_FTPHost` | AnsiString | `sN32_FTPHost` |  |  |  |  |
| N32-4 |  | `cbN32_4` | `bN32_CheckAtTrayFeedFinish` | bool | `bN32_CheckAtTrayFeedFinish;` |  |  |  |  |
| N32-4 |  | `edN32_4` | `sN32_FTPDownloadPath` | AnsiString | `sN32_FTPDownloadPath` |  |  |  |  |
| N32-5 |  | `edN32_5` | `sN32_NetDownloadPath` | AnsiString | `sN32_NetDownloadPath` |  |  |  | Steven 20221216 : 使用網路硬碟下載安裝包 |
| N32-6 |  | `edN32_6` | `sN32_FTPDownloadPath2` | AnsiString | `sN32_FTPDownloadPath2` |  |  |  | Sam 20230328 : 改使用更新包的產品版本來判別是否更新。 |
| N33 | Upload OCR and Bin log | `edN33` | `asN33_UploadLogPath` | AnsiString | `asN33_UploadLogPath` | -- | -- | Upload OCR and Bin log |  |
| N33 | Upload OCR and Bin log | `cbN33` | `bN33_UpLoadOCRBinLogByNet` | bool | `bN33_UpLoadOCRBinLogByNet` | -- | -- | Upload OCR and Bin log | KenHsieh 20230502 : 利揚要求上傳OCR + BIN Log上傳至Host |
| N33-1 | Net Driver change file and data | `cbN33_1` | `bN33_1_NetChangeFileAndData` | bool | `bN33_1_NetChangeFileAndData` |  | ECBool | Enable to change file and data by Net Driver | KenHsieh 20230727 : 更改工作檔與資料 By NetFile |
| N34 | OEE And Failure Report | `cbN34` | `bN34_GenerateOEEAlarmRpt` | bool | `bN34_GenerateOEEAlarmRpt` |  |  | OEE And Failure Report | Jimmychiu 20250324 : CC_CYUEAN OEE report |
| N34 | OEE And Failure Report | `edN34` | `sN34_OEEAlarmRptPath` | AnsiString | `sN34_OEEAlarmRptPath` |  |  | OEE And Failure Report | Jimmychiu 20250324 : CC_CYUEAN OEE report |
| N35 | Record Ground and ESD at intervals and upload | — | `sN35_JHT_UploadPath` | AnsiString | — |  | — | Record Ground and ESD at intervals and upload |  |
| N35-1 | Upload JHT format log 2 | `cbN35` | `bN35UploadJHT_Log` | bool | `bN35UploadJHT_Log` |  |  |  |  |
| N35-1 | Upload JHT format log 2 | `cbN35_1` | `bN35_Ground_ESD_Upload` | bool | `bN35_Ground_ESD_Upload` |  |  |  | Sam 20250609 : Record Ground and ESD at intervals and upload |
| N35-1 | Upload JHT format log 2 | `rgN35_1` | `iN35_Interval` | int | `iN35_Interval` |  |  |  |  |
| N35-1 | Upload JHT format log 2 | — | `sN35_1ASE_CL_FTPHost` | AnsiString | — |  | — |  |  |
| N35-1 | Upload JHT format log 2 | — | `sN35_1ASE_CL_FTPPassword` | AnsiString | — |  | — |  |  |
| N35-1 | Upload JHT format log 2 | — | `sN35_1ASE_CL_FTPUplaodPath` | AnsiString | — |  | — |  |  |
| N35-1 | Upload JHT format log 2 | — | `sN35_1ASE_CL_FTPUserName` | AnsiString | — |  | — |  |  |
| N35-1 | Upload JHT format log 2 | `edN35_1` | `sN35_FTPUserName` | AnsiString | `sN35_FTPUserName` |  |  |  |  |
| N35-2 |  | `edN35_2` | `sN35_FTPPassword` | AnsiString | `sN35_FTPPassword` |  |  |  |  |
| N35-3 |  | `edN35_3` | `sN35_FTPHost` | AnsiString | `sN35_FTPHost` |  |  |  |  |
| N35-4 |  | `edN35_4` | `sN35_FTPUploadPath` | AnsiString | `sN35_FTPUploadPath` |  |  |  |  |
| N40 |  | — | `sN40_FTPHost` | AnsiString | — |  | — |  |  |
| N40 |  | — | `sN40_FTPPassword` | AnsiString | — |  | — |  |  |
| N40 |  | — | `sN40_FTPUploadPath` | AnsiString | — |  | — |  |  |
| N40 |  | — | `sN40_FTPUserName` | AnsiString | — |  | — |  |  |
| N40-1 |  | — | `bN40_1_HandlerDataBackUpUseFunction` | bool | — |  | — |  |  |
| N41 |  | — | `sN41_DiskUploadPath` | AnsiString | — |  | — |  |  |
| N41-1 |  | — | `bN41_1_HandlerDataBackUpToDiskUseFunction` | bool | — |  | — |  |  |

---

## 未綁定 UI 元件的欄位

_無。所有 `N##` 開頭的欄位均已在主表中列出。_
