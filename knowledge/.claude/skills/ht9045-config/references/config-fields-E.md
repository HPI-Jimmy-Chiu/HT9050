# HT9045_CONFIG 欄位速查：群組 [E] In/Out Arm（進出手臂設定）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Exx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 66 個欄位，分屬 60 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| E30 | In arm use different scale (Range 0.95'#65374'1.05) | — | `bE30InArmUseDifferentScale` | bool | — | 35200 | — | In arm use different scale (Range 0.95～1.05) |  |
| E30-1 | In arm use different scale (Range 0.95'#65374'1.05) Hot | — | `bE30_1InArmUseDifferentScale_Hot` | bool | — |  | — | In arm use different scale (Range 0.95~1.05) Hot | Ztex 2024.11.18 Add In\Out\Sht Different Scale By Temperature --> |
| E30-2 | In arm use different scale (Range 0.95'#65374'1.05) Cold | — | `bE30_2InArmUseDifferentScale_Cold` | bool | — |  | — | In arm use different scale (Range 0.95~1.05) Cold |  |
| E31 | Out arm use different scale (Range 0.95'#65374'1.05) | — | `bE31OutArmUseDifferentScale` | bool | — | 35207 | — | Out arm use different scale (Range 0.95～1.05) |  |
| E31-1 | Out arm use different scale (Range 0.95'#65374'1.05) Hot | — | `bE31_1OutArmUseDifferentScale_Hot` | bool | — |  | — | Out arm use different scale (Range 0.95~1.05) Hot |  |
| E31-2 | Out arm use different scale (Range 0.95'#65374'1.05) Cold | — | `bE31_2OutArmUseDifferentScale_Cold` | bool | — |  | — | Out arm use different scale (Range 0.95~1.05) Cold |  |
| E32 | Shuttle use different scale (Range 0.95'#65374'1.05) | — | `bE32ShuttleUseDifferentScale` | bool | — | 35220 | — | Shuttle use different scale (Range 0.95～1.05) |  |
| E32-1 | Shuttle use different scale (Range 0.95'#65374'1.05) Hot | — | `bE32_1ShuttleUseDifferentScale_Hot` | bool | — |  | — | Shuttle use different scale (Range 0.95~1.05) Hot |  |
| E32-2 | Shuttle use different scale (Range 0.95'#65374'1.05) Cold | — | `bE32_2ShuttleUseDifferentScale_Cold` | bool | — |  | — | Shuttle use different scale (Range 0.95~1.05) Cold |  |
| E33 | In && out arm Z using same offset | `cbE33` | `bE33InOutArmZOffsetSameOne` | bool | `bE33InOutArmZOffsetSameOne` | 35229 | ECBool | In && out arm Z using same offset |  |
| E34 | In && out arm pitch && pick/release using same offset | `cbE34` | `bE34InOutArmPitchZOffsetSameOne` | bool | `bE34InOutArmPitchZOffsetSameOne` | 35230 | ECBool | In && out arm pitch && pick/release using same offset |  |
| E35 |  | `cbE35` | `bInOutArmPlaceSkipSuckDetect` | bool | `bInOutArmPlaceSkipSuckDetect` | 35231 | ECBool | In && Out arm disable check device drop when picker goes down. | jou 2011-05-26 |
| E36 |  | `edE36` | `iInArm60mmOffset` | int | `iInArm60mmOffset` | 35232 | ECInteger | In arm Y pitch 60mm offset ( Range:100~-100 , 0.01mm/unit ) | jou 2011-08-05 In Arm Y Pitch 60mm Offset |
| E37 |  | `edE37` | `iOutArm60mmOffset` | int | `iOutArm60mmOffset` | 35233 | ECInteger | Out arm Y pitch 60mm offset ( Range:100~-100 , 0.01mm/unit ) | jou 2011-09-26 Out Arm Y Pitch 60mm Offset |
| E38 | Check hot plate while initial start. | `cbE38` | `bE38CheckHotPlateWhileInitialStart` | bool | `CheckHotPlateWhileInitialStart` | 35234 | ECBool | Check hot plate while initial start. | ChungHung 20120206 Hotplate check |
| E39 | Check hot plate after clean out and before tray feed. | `cbE39` | `bE39CheckHotPlateAfterCleanOutAndBeforeTrayFeed` | bool | `CheckHotPlateAfterCleanOutAndBeforeTrayFeed` | 35235 | ECBool | Check hot plate after clean out and before tray feed. | ChungHung 20120206 Hotplate check |
| E39-1 | Put the devices to error bin | `cbE39_1` | `bE39_1PutTheDevicesToErrorBin` | bool | `PutTheDevicesToErrorBin` | 35236 | ECBool | Put the devices to error bin | ChungHung 20120206 Hotplate check |
| E40 | Clear all hot IC then pick loader IC | `cbE40` | `bE40ClearAllHotICThenPickLoadIC` | bool | `bClearAllHotICThenPickLoadIC` | 35237 | ECBool | Clear all hot IC then pick loader IC | ChungHung 20120514 Clear HotIC then Pick Load IC |
| E41 | Tray pitch > 35mm, in out arm speed must small than 80%. | `cbE41` | `bE41_35TrayPitchInOutSpeedSmall80Percent` | bool | `b35TrayPitchInOutSpeedSmall80Percent` | 35238 | ECBool | Tray pitch > 35mm, in out arm speed must small than 80%. | Steven 20120727 : Tray Pitch 大於35mm的話, In Out Arm需要小於80% |
| E42 | In arm need servo off when out shuttle alarm | `cbE42` | `bE42OutShuttleAlarmInArmServoOff` | bool | `bOutShuttleAlarmInArmServoOff` | 35239 | ECBool | In arm need servo off when out shuttle alarm | ChungHung 20120723 Output Shuttle Alarm InArm Servo off |
| E43 | Auto clean use hot plate 1 | `cbE43` | `bE43AutoCleanUseHotplate` | bool | `bAutoCleanUseHotplate` |  | ECBool | Auto clean use hot plate 1 | ChungHung 20131120 AutoClean use Hotplate1 |
| E43-1 | Auto clean count save to DefineAutoClean folder | `chkE43_1` | `bE43_1_AutoCleanCountSaveFolder` | bool | `bE43_1_AutoCleanCountSaveFolder` |  |  | Auto clean count save to DefineAutoClean folder | Steven 20250527 : Save auto clean count to DefineAutoClean folder |
| E44 | Shuttle need servo off when shuttle lose devices | `cbE44` | `bE44EnableLoseDeviceOutShuttleServoOff` | bool | `bEnableLoseDeviceOutShuttleServoOff` |  | ECBool | Shuttle need servo off when shuttle lose devices | ChungHung 20140522 add OutShuttle lose devices can servo off |
| E45 | All setup file use one offset data | `cbE45` | `bE45_AllSetupFileUseOneFile` | bool | `bE45_AllSetupFileUseOneFile` | 35240 | ECBool | All setup file use one offset data | Steven 20140827 : 所有工作檔共用同一個Offset檔案 |
| E46 | Loader use 2 offset for each row | `cbE46` | `bE46_LoaderUse2Offset` | bool | `bE46_LoaderUse2Offset` | 35241 | ECBool | Loader use 2 offset for each row | Steven 20140827 : Loader可以分別調前後排的Offset |
| E47 | Shuttle use 4 offset for each row and col | `cbE47` | `bE47_ShuttleUse4Offset` | bool | `bE47_ShuttleUse4Offset` | 35242 | ECBool | Shuttle use 4 offset for each row and col | Steven 20140827 : Shuttle可以分別調前後排的Offset |
| E48 | Auto clean shuttle use 4 offset for each row and col | `cbE48` | `bE48_ShuttleUse4Offset_Autoclean` | bool | `bE48_ShuttleUse4Offset_Autoclean` | 35243 | ECBool | Auto clean shuttle use 4 offset for each row and col | 20140923 wei :  Auto clean Shuttle可以分別調前後排的Offset |
| E49 | Loader pick up error only RETRY and CLEAN OUT | `cbE49` | `bE49_LoaderOnlyRetryAndCleanOut` | bool | `bE49_LoaderOnlyRetryAndCleanOut` | 35244 | ECBool | Loader pick up error only RETRY and CLEAN OUT | Steven 20141105 : Loader吸取異常只能Retry與CleanOut |
| E50 | Output arm pickup error can choose | `rgE50` | `iE50_OutArmPickUpErrorOption` | int | `iE50_OutArmPickUpErrorOption` | 35245 | ECInteger | Output arm pickup error can choose | JerryYang 20210813 : 改成選項  //Steven 20141121 : OutArm吸取異常只能Retry |
| E51 | In arm Z ADC speed | `cbE51` | `bE51_EnableInArmZADC` | bool | `bE51_EnableInArmZADC` | 35246 | ECBool | In arm Z ADC speed | Steven 20141212 : 使用固定的ADC |
| E51 | In arm Z ADC speed | `edE51` | `iE51_EnableInArmZADC` | int | `iE51_EnableInArmZADC` | 35246 | ECBool | In arm Z ADC speed |  |
| E52 | Out arm Z ADC speed | `cdE52` | `bE52_EnableOutArmZADC` | bool | `bE52_EnableOutArmZADC` | 35248 | ECBool | Out arm Z ADC speed |  |
| E52 | Out arm Z ADC speed | `edE52` | `iE52_EnableOutArmZADC` | int | `iE52_EnableOutArmZADC` | 35248 | ECBool | Out arm Z ADC speed |  |
| E53 | When low yield do auto clean and close site. | `cbE53` | `bE53LowYieldAutoClean` | bool | `bE53LowYieldAutoClean` | 35250 | ECBool | When low yield do auto clean and close site. | kevin 20160802 : CosFunction.bLowYieldAutoClean --> IniConfig.bE53LowYieldAutoClean |
| E54 | Check close site can not have IC when pick from hot plate | `cbE54` | `bE54CheckCloseSiteNoIC` | bool | `bE54CheckCloseSiteNoIC` | 35251 | ECBool | Check close site can not have IC when pick from hot plate | Steven 20160922 : 因為OneCycle永遠先跑蝦頭1, 檢查加熱盤錯誤功能與ByArmCloseSite衝突 |
| E55 | Use Fix3 Full Tray Function | `cbE55` | `bE55UseFix3FullTray` | bool | `bE55UseFix3FullTray` | 35252 | ECBool | Use Fix3 Full Tray Function | Ifor 20161121 : add Use Fix3 Full Tray Function |
| E56 | When loader have pick up error, to retry at same position. | `cbE56` | `bE56LoaderRetryAtSamePosition` | bool | `bE56LoaderRetryAtSamePosition` | 35253 | ECBool | When loader have pick up error, to retry at same position. | Steven 20170828 (wei) : Loader吸取異常時,要在同一個位置作Retry |
| E57 | Hot Plate can use another vacuum delay time | `cbE57` | `bE57HPCanUseAnotherVacuumDelay` | bool | `bE57HPCanUseAnotherVacuumDelay` | 35254 | ECBool | Hot Plate can use another vacuum delay time | Steven 20180125 (Jou) : 加熱盤的真空等待時間 |
| E58 | In Out Arm Y pitch home check. | `cbE58` | `bE57YPitchHome` | bool | `bE57YPitchHome` | 35255 | ECBool | In Out Arm Y pitch home check. | kevin 20180822 (Steven) : in out arm Y pitch 放完IC 歸 Y PITCH HOME |
| E59 | Offset file group by [###] | `cbE59` | `bE59GroupOffsetFile` | bool | `bE59GroupOffsetFile` | 35256 | ECBool | Offset file group by [###] | Steven 20190327 : Offset file使用中括號做群組 |
| E60 | In arm pick from loader drop error auto skip | `cbE60` | `bE60PickLoaderDropAutoSkip` | bool | `bE60PickLoaderDropAutoSkip` |  | ECBool | In arm pick from loader drop error auto skip |  |
| E61 | In arm standby postion on loader | `cbE61` | `bE61InArmStandbyPosOnLoader` | bool | `bE61InArmStandbyPosOnLoader` |  | ECBool | In arm standby postion on loader |  |
| E62 | Search last row when auto skip count over limit. | `cbE62` | `bE62TryPickLastRow` | bool | `bE62TryPickLastRow` |  | ECBool | Search last row when auto skip count over limit. | JerryYang 20200422 Auto skip次數到達後, 自動再去最後一排吸吸看 |
| E63 |  | `cbE63` | `bE63RetryPickLoader` | bool | `bE63RetryPickLoader` |  | ECBool | In arm retry to pick up the loader device before alarm takeout tray message. |  |
| E64 |  | `cbE64` | `bE64_50TrayPitchInOutSpeedSmall50Percent` | bool | `bE64_50TrayPitchInOutSpeedSmall50Percent` |  | ECBool | Tray pitch > 50mm or Tray  X-Division=1 , in out arm speed must small than 50%. | Ifor 20201224 add: Tray Pitch 大於50mm 或 Tray  X-Division=1, In Out Arm需要小於50% |
| E65 | Out arm destroy error, clear the data on auto tray. | `chkE65` | `bE65_ClearTrayDataWhenOutArmDestoryErr` | bool | `bE65_ClearTrayDataWhenOutArmDestoryErr` |  | ECBool | Out arm destroy error, clear the data on auto tray. | Steven 20210316 : 掉料的時候, 清除Unloader tray上的資料 |
| E66 | Log Hot Plate Action | `chkE66` | `bE66_LogHotPlateAction` | bool | `bE66_LogHotPlateAction` |  | ECBool | Log Hot Plate Action | Steven 20211104 : 紀錄加熱盤的動作 |
| E67 | Load pick up error,move wait pos. | `chkE67` | `bE67_LoadPickerrorMoveWaitpos` | bool | `bE67_LoadPickerrorMoveWaitpos` |  | ECBool | Load pick up error,move wait pos. | kevin 20220723  : Load Pick up error Move wait pos |
| E68 |  | `cbE68` | `bE68RecheckInOutArmICFallDown` | bool | `bE68RecheckInOutArmICFallDown` |  | ECBool | In/Out Arm IC drop status check several times and then alarm | Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警 |
| E68 |  | `edE68` | `iE68RecheckInOutArmICFallDown` | int | `iE68RecheckInOutArmICFallDown` |  | ECBool | In/Out Arm IC drop status check several times and then alarm | Sam 20221006 : In/Out Arm IC 掉落狀態多檢查幾次再報警 |
| E69 | Pickup Error Placement | `chkE69` | `bE69_PickupErrorPlacement` | bool | `bE69_PickupErrorPlacement` |  | ECBool | Pickup Error Placement | JimmyChiu 20220908 add Pickup Error Placement |
| E70 | In/Out Arm Use Tray Thick Adjust Z Height | `chkE70` | `bE70_UseTrayThickAdjustZHeight` | bool | `bE70_UseTrayThickAdjustZHeight` |  | ECBool | In/Out Arm Use Tray Thick Adjust Z Height | Ifor 20221220 add:新增選項開啟或關閉使用Tray 厚度 自動補償Z軸高度 |
| E71 |  | `chkE71` | `bE71_10TrayPitchLockTrayAssign` | bool | `bE71_10TrayPitchLockTrayAssign` |  | ECBool | Tray Pitch < 10mm Lock Loader to Empty and Color to Auto Tray | Ifor 20221220 add:新增Tray Pitch 小於10mm 強制鎖定Loader To Empty Color To Auto |
| E72 | Inarm pick IC from tray need wait shuttle. | `chkE72` | `bE72_InarmPickICNeedWaitSH` | bool | `bE72_InarmPickICNeedWaitSH` |  |  | Inarm pick IC from tray need wait shuttle. | KenHsieh 20230614 : Inarm pick IC from tray需待SH到位且為可取放料狀態 |
| E73 |  | `chkE73` | `bE73_InOutZStepMotorLossCheck` | bool | `bE73_InOutZStepMotorLossCheck` |  | ECBool | Inarm Z Motor Step Loss Check.                                  times/min. | JerryYang 20240111 : add |
| E73 |  | `edE73` | `iE73StepMotorCheckCnt` | int | `iE73StepMotorCheckCnt` |  | ECBool | Inarm Z Motor Step Loss Check.                                  times/min. |  |
| E74 | Inspect In/Out arm position. | `chkE74` | `bE74_InspectArmPosition` | bool | `bE74_InspectArmPosition` |  | ECBool | Inspect In/Out arm position. | Jimmychiu 20240408 : debug for inarm position |
| E77 | Output arm C motion. | — | `bE77_OutamrCMotion` | bool | — |  | — | Output arm C motion. |  |
| E78 |  | — | `bE78OneByOneWhenPickErrAtLoader` | bool | — |  | — | InArmDrop Index, check IC | Jimmychiu 20250924 : Suck one by one when a pickup error occurs at the loader. |
| E85 | Enable | `cbE85_Auto1` | `bE85_FillTray_Auto1` | bool | `bE85_FillTray_Auto1` |  | ECBool | Fill The Tray | Jimmychiu 20240726 : Fill The Tray After Out Arm Place |
| E85 | Enable | `cbE85_Auto3` | `bE85_FillTray_Auto3` | bool | `bE85_FillTray_Auto3` |  | ECBool | Fill The Tray | Jimmychiu 20240726 : Fill The Tray After Out Arm Place |
| E85 | Enable | `cbE85_Enable` | `bE85_FillTray_Enable` | bool | `bE85_FillTray_Enable` |  | ECBool | Fill The Tray | Jimmychiu 20240726 : Fill The Tray After Out Arm Place |
| E86 | Input arm pick up error on loader only can skip/tray end. | `cbE86` | `bE86_InArmPickErrOnLoaderOnlyCanSKIP` | bool | `bE72_InArmPickErrOnLoaderOnlyCanSKIP` |  | ECBool | InArm Suck one by one when a pickup error occurs at the loader. | JerryYang 20250120 : add |
| E87 | Pick up Error At Loader Need Open Door. | `cbE87` | `bE87PickupErrorAtLoaderNeedOpenDoor` | bool | `bE73PickupErrorAtLoaderNeedOpenDoor` |  |  |  |  |
| E88 |  | `cbE88` | `bE88_InArmHeightFollow7000` | bool | `bE75_InArmHeightFollow7000` |  |  |  |  |
| E89 | Hot plate pitch use scale. | `cbE89` | `bE89_InArmHotPlatePitchUseScale` | bool | `bE76_InArmHotPlatePitchUseScale` |  |  |  |  |

---


## 未綁定 UI 元件的欄位

_無。所有 `E##` 開頭的欄位均已在主表中列出。_
