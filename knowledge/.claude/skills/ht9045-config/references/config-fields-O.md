# HT9045_CONFIG 欄位速查：群組 [O] Count（Log / 計數 / 統計）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Oxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 47 個欄位，分屬 33 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| O01 | [ RESET ] Clear and auto check hot plate matrix | `cbO01` | `bO01_ResetNeedClearAndCheckHP` | bool | `bResetNeedClearAndCheckHotPlate` |  | ECBool | [ RESET ] Clear and auto check hot plate matrix | Steven 20120322 : RESET後,清空In Arm吸嘴資料與HotPlate資料,然後作HotPlate檢查 |
| O02 | [ RESET ] do not need clear hot plate | `cbO02` | `bO02ResetNotClearPlate` | bool | `bO02ResetNotClearPlate` |  | ECBool | [ RESET ] do not need clear hot plate |  |
| O05 | [ RESET ] Need remove all tray on the handler | `cbO05` | `bO05ResetNeedRemoveAllTray` | bool | `bO05ResetNeedRemoveAllTray` |  | ECBool | [ RESET ] Need remove all tray on the handler |  |
| O06 | Auto save event log | `edtO06AlarmHistroy` | `asAlarmHistroyAutoSavePath` | AnsiString | `AutoSaveAlarmHistroyPath` | -- | -- | Auto save event log |  |
| O06 | Auto save event log | `edtO06_Local` | `asAlarmLocalDirectory` | AnsiString | `asAlarmRemoteDirectory` | -- | -- | Auto save event log | Steven 20140306 : 使用網路硬碟 |
| O06 | Auto save event log | `edtO06_Remote` | `asAlarmRemoteDirectory` | AnsiString | `asAlarmLocalDirectory` | -- | -- | Auto save event log | Steven 20140306 : 使用網路硬碟 |
| O06 | Auto save event log | `edtO06AlarmStatist` | `asAlarmStatistAutoSavePath` | AnsiString | `AutoSaveAlarmStatistPath` | -- | -- | Auto save event log |  |
| O06 | Auto save event log | `edO06_FilePath` | `asEventLogAutoSavePath` | AnsiString | `AutoSaveEventLogPath` | -- | -- | Auto save event log |  |
| O06 | Auto save event log | `edtO06Production` | `asProductionAutoSavePath` | AnsiString | `AutoSaveProductionPath` | -- | -- | Auto save event log | Steven 20140816 : Production Data |
| O06 | Auto save event log | `chkO06TimePeriod` | `bO06SaveLogTimePeriod` | bool | `EnanleTimePeriodSaveLog` | -- | -- | Auto save event log | JerryYang 20151026 : 使用ProductionData週期時間存檔 |
| O06 | Auto save event log | `cbO06TimePeriod` | `iO06SaveLogTimePeriod` | int | `TimePeriodSaveLog` | -- | -- | Auto save event log | JerryYang 20151026 : ProductionData週期時間(0->10mins,1->30mins) |
| O06-1 | Enable Event log | `cbO06` | `bO06_EventLogAutoSave` | bool | `EnableAutoSaveEventLog` |  | ECBool | Enable Event log | Steven 20110221 Start : EventLogAutoSave |
| O06-2 | Enable Alarm Histroy | `chkO06AlarmHistroy` | `bEnableAlarmHistroyAutoSave` | bool | `EnableAutoSaveAlarmHistroy` |  | ECBool | Enable Alarm Histroy |  |
| O06-3 | Enable Alarm Statist | `chkO06AlarmStatist` | `bEnableAlarmStatistAutoSave` | bool | `EnableAutoSaveAlarmStatist` |  | ECBool | Enable Alarm Statist |  |
| O06-4 | Enable Production Data | `chkO06Production` | `bEnableProductionAutoSave` | bool | `EnableAutoSaveProductiont` |  | ECBool | Enable Production Data | Steven 20140816 : Production Data |
| O06-6 | Use Net Drive | `chkO06UseNetDrive` | `bAlarmStatistAutoSaveNetDrive` | bool | `bAlarmStatistAutoSaveNetDrive` |  | ECBool | Use Net Drive | Steven 20140306 : 使用網路硬碟 |
| O07 | [FT ]Can'#39't off continue | `cbO07` | `bFTContinueON` | bool | `bFTContinueON` |  | ECBool | [FT ]Can't off continue | kevin 20121008 FT MODE 不能關連續FAIL |
| O07 | [FT ]Can'#39't off continue | `edO07` | `iFTMAXValue` | int | `iFTMAXValue` |  | ECBool | [FT ]Can't off continue | kevin 20121008 FT MODE 連續FAIL 最大數量 |
| O09 | Initial start need ask | `cbO09` | `bInitialStartNeedAsk` | bool | `bInitialStartNeedAsk` |  | ECBool | Initial start need ask | ChungHung 20130529 add manual change start mode 如果是InitialStart 按下Start時要詢問 |
| O10 | Use event log saver program | `cbO10_EventLogSaver` | `bO10UseEventLogSaver` | bool | `bO10UseEventLogSaver` |  | ECBool | Use event log saver program | Steven 20140902 : 使用外掛小程式 |
| O11 | Record Jam Rate By Time :                       minutes. | `cbO11` | `bRecordJamRateByTime` | bool | `Record Jam Rate By Time` |  | ECBool | Record Jam Rate By Time :                       minutes. | 2015.11.11 , Joye , Add Jam Rate Record |
| O11 | Record Jam Rate By Time :                       minutes. | `edtO11` | `iRecordJamRateIntervalTime` | int | `Record Jam Rate Interval Time` |  | ECBool | Record Jam Rate By Time :                       minutes. | 2015.11.11 , Joye , Add Jam Rate Record |
| O12 | Use Head Condition1  Life Time Control . | `cbO12_LifeTimeCount` | `bLifeTimeCount` | bool[] | `bLifeTimeCount 1` |  | ECBool | Use Head Life Time Control . | wei 20160509 Life Time Count |
| O13 | Use Head Condition1  Life Time Control . | `cbO13_LifeTimeCount` | `bLifeTimeCount` | bool[] | `bLifeTimeCount 2` |  | ECBool | Use Head  Life Time Control . | wei 20160509 Life Time Count |
| O14 | Use Head Condition1  Life Time Control . | `cbO14_LifeTimeCount` | `bLifeTimeCount` | bool[] | `bLifeTimeCount 3` |  | ECBool | Use Head  Life Time Control . | wei 20160509 Life Time Count |
| O15-1 | Saving file period : | `cbbO15_1` | `iO15_SaveFilePeriod` | int | `iO15_SaveFilePeriod` |  | ECInteger | Saving file period : | Steven 20170829 (wei) : Event Log檔案存檔設定 |
| O15-2 | File name include machine ID | `chkO15_2` | `bO15_EventLogFileNameWithMachineID` | bool | `bO15_EventLogFileNameWithMachineID` |  | ECBool | File name include machine ID |  |
| O15-3 | Save same folder | `chkO15_3` | `bO15_EventLogSaveSameFolder` | bool | `bO15_EventLogSaveSameFolder` |  | ECBool | Save same folder | KaiChen 20180322 ：矽格-湖口 新增 將 Event Log 放在同個資料夾 |
| O16 | Continuously alarm need password. Count : | `chkO16` | `bO16ConAlarmNeedKeyInPasswordCT` | bool | `bO16ConAlarmNeedKeyInPasswordCT` |  | ECBool | Continuously alarm need password. Count : | Steven 20200513 : 改成可以設定[O16] |
| O16 | Continuously alarm need password. Count : | `edtO16` | `iO16ConAlarmNeedKeyInPasswordCT` | int | `Con Alarm Need KeyIn Password CT` |  | ECBool | Continuously alarm need password. Count : | jou 2014-09-04 Continuous Same Alarm N time Need KeyIn Password |
| O17 | Use level [166] when alarm count | `chkO17` | `bO17EnableLevelUpWhenContiAlarm` | bool | `bO17EnableLevelUpWhenContiAlarm` |  | ECBool | Use level [166] when alarm count | Steven 20210127 : 逸昌要求在單位時間內相同Alarm發生多次,提昇解除alarm權限 |
| O17 | Use level [166] when alarm count | `edtO17_Count` | `iO17LevelUpWhenContiAlarmCount` | int | `iO17LevelUpWhenContiAlarmCount` |  | ECBool | Use level [166] when alarm count | Steven 20210127 : 連續alarm次數 |
| O17 | Use level [166] when alarm count | `edtO17_Time` | `iO17LevelUpWhenContiAlarmTime` | int | `iO17LevelUpWhenContiAlarmTime` |  | ECBool | Use level [166] when alarm count | Steven 20210127 : 連續alarm時間 |
| O18 | Safe door on/off duration detect. (hr) | `cbO18` | `bO18SafeDoorOnOffDurationDetect` | bool | `bO18SafeDoorOnOffDurationDetect` |  | ECBool | Safe door on/off duration detect. (hr) | JerryYang 20210112 : 安全門檢查機制,長時間未開啟要跳出alarm |
| O18 | Safe door on/off duration detect. (hr) | `edtO18` | `iO18SafeDoorOnOffDurationHour` | int | `iO18SafeDoorOnOffDurationHour` |  | ECBool | Safe door on/off duration detect. (hr) |  |
| O19 | Auto Record Report | — | `iO19_MonthPeriod` | int | — | -- | — | Auto Record Report |  |
| O19 | Auto Record Report | — | `iO19_WeekPeriod` | int | — | -- | — | Auto Record Report |  |
| O19-1 |  | `cbO19_1` | `bO19_AutoRecordReportByEveryDay` | bool | `bO19_AutoRecordReportByEveryDay` |  |  |  |  |
| O19-2 |  | `cbO19_2` | `bO19_AutoRecordReportByEveryWeek` | bool | `bO19_AutoRecordReportByEveryWeek` |  |  |  |  |
| O19-4 |  | `cbO19_4` | `bO19_AutoRecordReportByEveryMonth` | bool | `bO19_AutoRecordReportByEveryMonth` |  |  |  |  |
| O19-6 |  | `edtO19_6` | `asO19_SavePath` | AnsiString | `asO19_SavePath` |  |  |  |  |
| O20 | Input/output arm picker life time control. | `cbO20` | `bO20InOutArmPickerLifeTimeCount` | bool | `bO20InOutArmPickerLifeTimeCount` |  | ECBool | Input/output arm picker life time control. |  |
| O20-1 | Clear data when initial start | `cbO20_1` | `bO20_1ClearLifeTimeWhenInitialStart` | bool | `bO20_1ClearLifeTimeWhenInitialStart` |  | ECBool | Clear data when initial start | Steven 20240531 : initial start的時候要清除資料 |
| O21 | FT After tray end clear fail bin count | `cbO21` | `bO21FTAfterTrayEndClearFailBinCount` | bool | `bO21FTAfterTrayEndClearFailBinCount` |  | ECBool | FT After tray end clear fail bin count | Frank 20241114 : Add |
| O22 | Clear tray count when double click. | `chkO22` | `bO22_ClearSortCntByDoubleClick` | bool | `bO22_ClearSortCntByDoubleClick` |  | ECBool | Clear tray count when double click. | Steven 20241206 : 點兩下可以清除數量 |
| O23 | Lot ID input by barcode reader. | `chkO23` | `bO23_InputLotIDByBarcode` | bool | `bO23_InputLotIDByBarcode` |  | ECBool | Lot ID input by barcode reader. | Steven 20241224 : LotID只能用Barcode |
| O24 | Save production by Lot ID | `chkO24` | `bO24_ProductionLogByLot` | bool | `bO24_ProductionLogByLot` |  | ECBool | Save production by Lot ID | Steven 20250519 : Production Log By Lot |

---


## 未綁定 UI 元件的欄位

_無。所有 `O##` 開頭的欄位均已在主表中列出。_
