# HT9045_CONFIG 欄位速查：群組 [L] Temperature（溫度控制）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Lxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 84 個欄位，分屬 72 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| L03 | Socket Air Cooling contact count trun on | `cbL03` | `bL03SocketAirCoolingCT` | bool | `bL03SocketAirCoolingCT` | 35621 | ECBool | Socket Air Cooling contact count trun on | jou 2016-04-28 Socket Air Cooling contact count trun on |
| L03 | Socket Air Cooling contact count trun on | `edL03` | `iL03SocketAirCoolingCT` | int | `iL03SocketAirCoolingCT` | 35621 | ECBool | Socket Air Cooling contact count trun on | jou 2016-04-28 Socket Air Cooling contact count trun on |
| L04 | Temperature range(2..10) | `edL04` | `iL04TemptureRange` | int | `iL04TemptureRange` | 35600 | ECInteger | Temperature range(2..10) |  |
| L05 | Chamber temperature range(2..30) | `edL05` | `iL05ChamberTemptureRange` | int | `iL05ChamberTemptureRange` | 35601 | ECInteger | Chamber temperature range(2..30) |  |
| L06 | Ambient temperature range(0..10) | `edtL06` | `iAmbTemperatureRange` | int | `iAmbTemperatureRange` | 35602 | ECInteger | Ambient temperature range(0..10) | jou 2013-04-11 Ambient Temperature Range |
| L07 | Use single limit | `cbL07` | `bL07UseSingleTenmpertureLimit` | bool | `bL07UseSingleTenmpertureLimit` | 35603 | ECBool | Use single limit |  |
| L08 | Socket temperature range(1..30) | `edL08_Over` | `iSocketTemptureRangeOver` | int | `iSocketTemptureRange` | -- | -- | Socket temperature range(1..30) | Steven 20140308 : DUT溫度限制改成上下限分開 |
| L08 | Socket temperature range(1..30) | `edL08_Under` | `iSocketTemptureRangeUnder` | int | `iSocketTemptureRangeUnder` | -- | -- | Socket temperature range(1..30) | Steven 20140308 : DUT溫度限制改成上下限分開 |
| L09 | Hot Temp shuttle no add offset pos | `cbL09` | `bL09HotTempShuttleNoAddPos` | bool | `bL23HotTempShuttlenoAddPos` | -- | -- | Tempture Position Shift | kevin 20200718 : ASEKH 高溫shuttle不補位置  //JerryYang 20230204 : L23 -> L09 |
| L09-1 | Hight Temp(165): | `edtL09_1` | `iL09_1HightTemp_Sht_Shift` | int | `iL09_1HightTemp_Sht_Shift` |  |  | Hight Temp(165): |  |
| L09-2 | Low Temp(-45): | `edtL09_2` | `iL09_2LowTemp_Sht_Shift` | int | `iL09_2LowTemp_Sht_Shift` |  |  | Low Temp(-45): | Ztex 2024.10.01 Add AStream Error Compress Onecycle |
| L10 | Temperature record Interval | `rgL10` | `iL10TempRecordInterval` | int | `iL10TempRecordInterval` | 35610 | ECInteger | Temperature record Interval |  |
| L10-1 | Index Test log temperature | `cbL10` | `bL10IndexTestlogTemp` | bool | `bL10IndexTestlogTemp` | 35623 | ECBool | Index Test log temperature | kevin 20190323 : index 測試時才記錄溫度 |
| L11-1 | ATC temperature range(1..30) | `cbL11_1` | `bL11_1ATCTemperatureOverAlarm` | bool | `bATCTemperatureOverAlarm` | 35611 | ECBool | ATC temperature range(1..30) | 2012.05.11 , Joye , ATC Temperature Over Check |
| L11-1 | ATC temperature range(1..30) | `edL11_1` | `iATCTemperatureRange` | int | `iATCTemperatureRange` | 35611 | ECBool | ATC temperature range(1..30) | 2012.05.11 , Joye , ATC Temperature Over Check |
| L11-1-2 |  | `edL11_1_2` | `dATCTemperatureCheckTime` | double | `dATCTemperatureCheckTime` |  |  |  | Steven 20121222 : ATC Temperature Over Check Time |
| L11-2 | Enable Chiller Auto Close Protected. | `cbL11_2` | `bL11_2ATCChillerProtectedFunction` | bool | `bATCChillerProtectedFunction` | 35614 | ECBool | Enable Chiller Auto Close Protected. | 2012.05.07 , Joye , Chiller |
| L11-2 | Enable Chiller Auto Close Protected. | `edL11_2` | `iATCChillerCheckTime` | int | `iATCChillerCheckTime` | 35614 | ECBool | Enable Chiller Auto Close Protected. | 2012.05.07 , Joye , Chiller |
| L11-4 | ATC max temperature limit : | `edL11_4` | `iATCTemperatureOverLimit` | int | `iATCTemperatureOverLimit` | 35624 | ECInteger | ATC max temperature limit : | Steven 20140916 : [L11-4] ATC的最高上限溫度 |
| L11-6 |  | `cbL11_6` | `bL11_6ATCUseTemperatureOutsideAlarm` | bool | `bATCUseTemperatureOutsideAlarm` | 35625 | ECBool | ATC Temperature outside             Continuous            (s)Alarm | Ifor 20150911 : [L11-6] ATC突波於設定範圍內且持續發生超過設定時間後發出Alarm |
| L11-6 |  | `edL11_6_Continuous` | `iATCTemperatureContinuous` | int | `iATCTemperatureContinuous` | 35625 | ECBool | ATC Temperature outside             Continuous            (s)Alarm | Ifor 20150911 : [L11-6] ATC 溫度持續超出設定值時間 |
| L11-6 |  | `edL11_6_Outside` | `iATCTemperatureOutside` | int | `iATCTemperatureOutside` | 35625 | ECBool | ATC Temperature outside             Continuous            (s)Alarm | Ifor 20150911 : [L11-6] ATC 溫度超出設定值 |
| L11-7 | ATC max peak alarm: | `cbL11_7` | `bL11_7ATCUseMaxSurgeAlarm` | bool | `bATCUseMaxSurgeAlarm` | 35628 | ECBool | ATC max peak alarm: | Ifor 20150911 : [L11-7] ATC突波超出最大設定值發出AlarmedL11_6_Outside |
| L11-7 | ATC max peak alarm: | `edL11_7_MaxSurge` | `iATCMaxSurgeAlarm` | int | `iATCMaxSurgeAlarm` | 35628 | ECBool | ATC max peak alarm: | Ifor 20150911 : [L11-7] ATC 突波警報最大容許範圍 |
| L11-8 | Use temperature difference over setting alarm | `cbL11_8` | `bL11_8ATCUseTemperatureCompare` | bool | `bATCUseTemperatureCompare` | 35630 | ECBool | Use temperature difference over setting alarm | Ifor 20150911 : [L11-8] ATC第1組sensor跟第2組sensor溫度差值超過設定值發出警報edL11_6_Continuous |
| L12 | Temperature error no close heater power | `cbL12` | `bL12TempErrNoCloseHeater` | bool | `TempErrNoCloseHeater` | 35617 | ECBool | Temperature error no close heater power | ChungHung 20120913 SCK  要求Temp Error 不要關閉加熱 |
| L13 | Hot plate and shuttle use same temperature offset | `cbL13` | `bL13HotPlateAndShuttleUseOneTempOffset` | bool | `bHotPlateAndShuttleUseOneTempOffset` | 35618 | ECBool | Hot plate and shuttle use same temperature offset | Steven 20131023 : 加熱盤與蝦頭使用同一個溫度補償的檔案 |
| L15 | Chamber mode too low need wait initial wait time | `cbL15` | `bL15EnableChamberModeEvenBlowNeedWaitTime` | bool | `bEnableChamberModeEvenBlowNeedWaitTime` | 35620 | ECBool | Chamber mode too low need wait initial wait time | ChungHung 20140519 add Chamber Mode Even Blow need Wait Initial Wait time in Temp_Set |
| L17 | Turn on all head heater when close site. | `cbL17` | `bL17HeadHeaterOnWhenCloseSite` | bool | `bHeadHeaterOnWhenCloseSite` | 35631 | ECBool | Turn on all head heater when close site. | Steven 20150803 : 關Site的地方也要開啟加熱 (For ATK) |
| L18 | No full site add temperature offset | `cbL18` | `bL18NofullsiteaddTemperatureoffset` | bool | `NofullsiteaddTemperatureoffset` | 35632 | ECBool | No full site add temperature offset | wei 20160107 No FullSite Add Offset |
| L19 | Keep heating when chamber door open without chamber heat | `cbL19` | `bL19OpenHeatDoorgiveupchamberhot` | bool | `bL19OpenHeatDoorgiveupchamberhot` | 35633 | ECBool | Keep heating when chamber door open without chamber heat | kevin 20170520 (wei) 開chambo門只有不加熱chambo |
| L20 | Abiemt guard band check | `cbL20` | `bL20AbientGuardBand` | bool | `bL20AbientGuardBand` | 35634 | ECBool | Abiemt guard band check | kevin 20180115 (Steven) add Amient Guard Band |
| L21 | Power OFF open chamber door, break all temperature power. | `cbL21` | `bL21PowerOffTemperature` | bool | `bL21PowerOffTemperature` | 35635 | ECBool | Power OFF open chamber door, break all temperature power. | kevin 20181112 (Steven) : power off 開 Chamber door 斷所有加熱電 |
| L22 | Enable 3 Sigma For Temperature Monitoring | `cbL22` | `bL22Enable3SigmaTempMonitor` | bool | `bL22Enable3SigmaTempMonitor` |  | ECBool | Enable 3 Sigma For Temperature Monitoring | kevin 20200521 : ASEKH 3SIGMA  溫度統計 |
| L24 | Heater stable wait time                       sec | `cbL24` | `bL24HeaterStableTime` | bool | `bL24HeaterStableTime` |  | ECBool | Heater stable wait time                       sec | JerryYang 20210122 : ASE-CL新增待溫功能 |
| L24 | Heater stable wait time                       sec | `edL24` | `iL24HeaterStableTime` | int | `iL24HeaterStableTime` |  | ECBool | Heater stable wait time                       sec | JerryYang 20210122 : ASE-CL新增待溫功能 |
| L25 | Change the format of Send Working file name to ATC | `cbL25` | `bL25_1ATCFileNameWithTemp` | bool | `bL25_1ATCFileNameWithTemp` |  | ECBool | Change the format of Send Working file name to ATC |  |
| L28 | Tempearture offset function use ready temp range | `cbL28` | `bL28TempOfsUseReadyTempRange` | bool | `bL28TempOfsUseReadyTempRange` |  | ECBool | Tempearture offset function use ready temp range | Sam 20231214 : Temp offset use ready temp range |
| L29 | Ambient mode does not display temperature | `cbL29` | `bL29AmbientNotShowTemp` | bool | `bL29AmbientNotShowTemp` |  | ECBool | Ambient mode does not display temperature | Sam 20221101 : 常溫模式不顯示溫度 |
| L30 | Use by configuration | `cbL30` | `bL30Use1CableLayoutKitByConfig` | bool | `bL30Use1CableLayoutKitByConfig` | -- | -- | Use 1 Cable Layout Kit By Configuration | Sam 20210715 : Use 1CableLayoutKit By Config |
| L30-1 | Use 1 Cable Layout Kit | `cbL30_1` | `bL30Use1CableLayoutKit` | bool | `bL30Use1CableLayoutKit` |  | ECBool | Use 1 Cable Layout Kit | Sam 20210715 : Use 1CableLayoutKit By Config |
| L32-1 | Enable Manual Defrost Function | `cbL32_1` | `bL32_1ManuDefrost` | bool | `bL32_1ManualDefrost` |  | ECBool | Enable Manual Defrost Function |  |
| L32-2 | Enable Auto Defrost Function | `cbL32_2` | `bL32_2AutoDefrostFunction` | bool | `bL32_2AutoDefrostFunction` |  | ECBool | Enable Auto Defrost Function |  |
| L32-3 |  | — | `bL32_3OneCycleDefrost` | bool | — |  | — |  |  |
| L32-4 | Set Defrost Temperature                   Degree | `edL32_4` | `iL32_4SetDefrostTemp` | int | `iL32_4SetDefrostTempature` |  | ECInteger | Set Defrost Temperature                   Degree | 除霜的門檻溫度 |
| L32-5 |  | `edL32_5` | `iL32_5SetDefrostTime` | int | `iL32_5SetDefrostTime` |  | ECInteger | Set Defrost Time                                   Minutes | 除霜的時間 |
| L32-6 | Set Below Temperature                       Degree. | `edL32_6` | `iL32_6LowTempRunAlarmDegree` | int | `iL32_6LowTempRunAlarmDegree` |  | ECInteger | Set Below Temperature                       Degree. |  |
| L32-7-1 |  | `edL32_7_1` | `iL32_7LowTempRunAlarmHour` | int | `iL32_7LowTempRunAlarmHour` |  |  |  |  |
| L32-7-2 |  | `edL32_7_2` | `iL32_7LowTempRunAlarmMin` | int | `iL32_7LowTempRunAlarmMin` |  |  |  |  |
| L32-8 | Set Air Stream Temp                             Degree. | `edL32_8` | `iL32_8SetAirStreamTemp` | int | `iL32_8SetAirStreamTemp` |  | ECInteger | Set Air Stream Temp                             Degree. |  |
| L33-1 | Enable Check Function( Only alarm) | `cbL33_1` | `bL33_1CheckDoorOpenForTriTemp` | bool | `bL33_1CheckDoorOpenForTriTemp` |  | ECBool | Enable Check Function( Only alarm) |  |
| L33-2 | Enable Automatic Heating and Defrosting | `cbL33_2` | `bL33_2DoorOpenRunDefrost` | bool | `bL33_2DoorOpenRunDefrost` |  | ECBool | Enable Automatic Heating and Defrosting |  |
| L33-3 |  | `edL33_3` | `iL33_3BDoorOpenTimeForLowTemp` | int | `iL33_3BDoorOpenTimeForLowTemp` |  | ECInteger | Set Dig Door Check Time(Below 25 Degree)                 Sec |  |
| L33-4 |  | `edL33_4` | `iL33_4SDoorOpenTimeForLowTemp` | int | `iL33_4SDoorOpenTimeForLowTemp` |  | ECInteger | Set Hatchway Check Time (Below25 Degree)               Sec(Small Door) |  |
| L33-5 |  | `edL33_5` | `iL33_5DoorOpenTempForLowTemp` | int | `iL33_5DoorOpenTempForLowTemp` |  | ECInteger | Set Check Door Open Over Time :Cold Temperature                    Degree |  |
| L33-6 |  | `edL33_6` | `iL33_6DoorOpenTempForHotTemp` | int | `iL33_6DoorOpenTempForHotTemp` |  | ECInteger | Set Check Door Open Over Time :Hot Temperature                       Degree |  |
| L34-1 | Enable this Function | `cbL34_1` | `bL34_1DelayOfFixDoorOpen` | bool | `bL34_1DelayOfFixDoorOpen` |  | ECBool | Enable this Function |  |
| L34-2 | Set Time                     Sec | `edL34_2` | `iL34_2DelaySecOfFixDoorOpen` | int | `iL34_2DelaySecOfFixDoorOpen` |  | ECInteger | Set Time                     Sec |  |
| L34-3 | Set DewPoint                     Degree | `edL34_3` | `dL34_3DewPointOfFixDoorOpen` | double | `dL34_3DewPointOfFixDoorOpen` |  | ECDouble | Set DewPoint                     Degree |  |
| L34-4 | Set Open Auto3 Track Flood Gate                     Sec | `edL34_4` | `iL34_4OpenAuto3TrackGateSec` | int | `iL34_4OpenAuto3TrackGateSec` |  | ECInteger | Set Open Auto3 Track Flood Gate                     Sec |  |
| L34-5 | Open Fix tray area safe door cylinders automatically | `cbL34_5` | `bL34_5FixTrayDoorCynAutoOpen` | bool | `bL34_5FixTrayDoorCynAutoOpen` |  | ECBool | Open Fix tray area safe door cylinders automatically |  |
| L35-1 | Enable Function | `cbL35_1` | `bL35_1OverSetTempOpenFan` | bool | `bL35_1OverSetTempOpenFan` |  | ECBool | Enable Function |  |
| L35-2 | Set Temperature                             Degree | `edL35_2` | `iL35_2OpenFanTemp` | int | `iL35_2OpenFanTemp` |  | ECInteger | Set Temperature                             Degree |  |
| L35-3 |  | — | `bL35_3NotUseVaccumCheckKit` | bool | — |  | — |  |  |
| L36-1 | Tri Temp ATC Rang | `edtL36_1` | `iL36_1Tri_Temp_Rang_ATC` | int | `iL36_1Tri_Temp_Rang_ATC` |  | ECInteger | Tri Temp ATC Rang |  |
| L36-2 | Tri Temp Heater Rang | `edtL36_2` | `iL36_2Tri_Temp_Rang_Heater` | int | `iL36_2Tri_Temp_Rang_Heater` |  | ECInteger | Tri Temp Heater Rang |  |
| L37 |  | `cbL37` | `bL37UnDockTurnOffAir` | bool | `bL37UnDockTurnOffAir` |  | ECBool | Docking/OTD Area Sensor  Off  Must Stop Air Machine Function |  |
| L38-1 |  | — | `bL38_1AirCoolerToCoolDown_SetEnable` | bool | — |  | — |  | Use air cooler to cool down |
| L38-2 |  | — | `iL38_2AirCoolerToCoolDown_SetTemperature` | int | — |  | — |  |  |
| L39-1 |  | `cbL39_1` | `bL39_1AutoRunWhenTempOk` | bool | `bL39_1AutoRunWhenTempOk` |  | ECBool | Enable operation after waiting for the Temperature In Range |  |
| L39-2 |  | `cbL39_2` | `bL39_2WaitTempstabilize` | bool | `bL39_2WaitTempstabilize` |  | ECBool | Wait for the Temperature Stabilization Time                  Sec |  |
| L39-2 |  | `edtL39_2` | `iL39_2WaitTempstabilize` | int | `iL39_2WaitTempstabilize` |  | ECBool | Wait for the Temperature Stabilization Time                  Sec |  |
| L40 | Immediate Temperature Exceed Range Show Alarm | `edL40` | `iL40ImmediateTempExceedsAlarm` | int | `iL40ImmediateTempExceedsAlarm` |  | ECInteger | Immediate Temperature Exceed Range Show Alarm |  |
| L40-2 |  | — | `bL40_2UnDockTurnOffAirDelay` | int | — |  | — |  |  |
| L40-3 |  | — | `iL40_3AirStreamRang` | int | — |  | — |  |  |
| L41 |  | `edL41` | `iL41TemperatureAlarmSecond` | int | `iL41TemperatureAlarmSecond` |  | ECInteger | ATC temperature exceed the scope                 seconds show alarm. |  |
| L42 | Use Out Shuttle Desoak Time                  Sec. | `cbL42` | `bL42_UseOutShuttleDesoakTime` | bool | `bL42_UseOutShuttleDesoakTime` |  | ECBool | Use Out Shuttle Desoak Time                  Sec. |  |
| L42 | Use Out Shuttle Desoak Time                  Sec. | `edtL42` | `iL42_UseOutShuttleDesoakTime` | int | `iL42_UseOutShuttleDesoakTime` |  | ECBool | Use Out Shuttle Desoak Time                  Sec. |  |
| L43 | Enable Power Follow Function | `cbL43` | `bL43EnableATCPowerFollow` | bool | `bL43EnableATCPowerFollow` |  | ECBool | Enable Power Follow Function | KenHsieh 20240216 : add ATC Power Follow Function |
| L44 | Set Cold Air Switch Temperature | `cbL44` | `bL44_SetColdAirSwitchTemp` | bool | `bL44_SetColdAirSwitchTemp` |  | ECBool | Enable Air Stream Abnormal The Compressor Need Onecycle |  |
| L44 | Set Cold Air Switch Temperature | `edtL44` | `iL44_SetColdAirSwitchTemp` | int | `iL44_SetColdAirSwitchTemp` |  | ECBool | Enable Air Stream Abnormal The Compressor Need Onecycle |  |
| L45 | Set Dew Point Offset | `cbL45` | `bL45_SetDewPointOffset` | bool | `bL45_SetDewPointOffset` |  | ECBool | Set Cold Air Switch Temperature | Ztex 2025.04.01 Add Set Dew Point Offset |
| L45 | Set Dew Point Offset | `edtL45` | `iL45_SetDewPointOffset` | int | `iL45_SetDewPointOffset` |  | ECBool | Set Cold Air Switch Temperature | Ztex 2025.04.01 Add Set Dew Point Offset |
| L46 | Enable Air Stream Abnormal The Compressor Need Onecycle | `cbL46` | `bL46_AStreamErrorCompressOnecycle` | bool | `bL43_AStreamErrorCompressOnecycle` |  | ECBool | Enable Index Suck IC Turn Off Air Stream |  |

---


## 未綁定 UI 元件的欄位

_無。所有 `L##` 開頭的欄位均已在主表中列出。_
