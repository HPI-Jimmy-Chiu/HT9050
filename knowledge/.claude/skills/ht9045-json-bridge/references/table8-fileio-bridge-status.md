# 表⑧ 讀檔／填表／寫檔三段式與 JSON 中介層（文字版）

> **由 `HT9011UC_Cpp_V3.33.906.0/scratchpad/gen_fileio_bridge_status.py` 產生，不要手改。**
> 與 `web/page/ScreenShots.html` 表⑧ 同一次執行寫出。產生時間 2026-09-23 16:36。
> BCB 對照樹 `HT9011UC_Code_V3.33.912.0_20260908_Jimmy`；移植樹 `HT9011UC_Cpp_V3.33.906.0`。

## 摘要

| 項目 | 數字 |
|---|---|
| 讀寫檔單位 | 57（form 方法 ＋ 自由函式） |
| BCB 讀檔鍵／寫檔鍵 | 1159／1162 |
| 移植樹 ①ReadFile／②DoIniDataToForm／③Save*File | 34／9／11 |
| 移植樹讀檔鍵／寫檔鍵 | 532／404 |
| 讀檔欄位已出 JSON | **470／553（85.0%）** |

⚠ 使用者 20260923 裁決：② `DoIniDataToForm()` 在新架構**不翻譯**（顯示層是 HTML，由 `ToJson` 取代）；③ 不逐字翻譯（它讀控制項），改從結構寫檔。所以 ②／③ 兩欄的「移植 ✔」只代表移植樹裡**有那段程式**，不代表新架構會用它。

## 逐單位

| 讀寫檔單位 | 單元 | 資料檔 | 機制 | ① BCB 讀檔 | ① 移植 | ② BCB 填表 | ② 移植 | ③ BCB 寫檔 | ③ 移植 | JSON 中介層 | SaveAllFile |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `TfOffSet` | `cOffSet.cpp` | `Position Offset Hot.Data、Position Offset.Data` | ReadIniData | :1998（79 鍵） | ✘ | :2316 | ✘ | :1470（48 鍵） | ✘ | ✘ 未接 0/45 欄 | ✔ |
| `TfHotPlate` | `cHotPlate.cpp` | `HotPlate.Data` | ReadIniData | :154（10 鍵） | ✔ 10 鍵 | :310 | ✔ | :480（11 鍵） | ✘ | ✘ 未接 0/9 欄 | ✔ |
| `(free) ReadLastSetIni` | `cprod.cpp` | `HotPlate.Data、config.ini` | ReadIniData | :2977（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | ✘ 未接 0/1 欄 | — |
| `TfYieldMonitoring` | `uYieldMonitoring.cpp` | `Tester.Data、config.ini` | ReadIniData | :900（250 鍵） | ✘ | :531 | ✘ | :2033（217 鍵） | ✘ | ◐ 部分 179/180 欄 | ✔ |
| `TfSpeed` | `cSpeed.cpp` | `ArmCondition.Data` | ReadIniData | :340（125 鍵） | ✔ 125 鍵 | :1008 | ✔ | :2250（110 鍵） | ✘ | ◐ 部分 9/26 欄 | ✔ |
| `TfSetup` | `cSetUp.cpp` | `Contact.Data、HandlerCondition.Data、Temperature.Data、configByRecipe.ini` | ReadIniData | :2199（99 鍵） | ✔ 96 鍵 | :3044 | ✘ | :3655（133 鍵） | ✘ | ◐ 部分 71/76 欄 | ✔ |
| `TfBarCode` | `BarCode/BarCode.cpp` | `—` | ReadIniData | :673（87 鍵） | ✘ | :1055 | ✘ | — | ✘ | ◐ 部分 77/78 欄 | — |
| `TfTrayAssignment` | `cTrayAssignment.cpp` | `Tray.Data` | ReadIniData | :163（49 鍵） | ✔ 49 鍵 | :548 | ✔ | :1409（86 鍵） | ✘ | ◐ 部分 26/30 欄 | ✔ |
| `TfContact` | `cContact.cpp` | `Contact.Data、Position Offset.Data` | 兩套 | :365（84 鍵） | ✘ | :728 | ✘ | :14313（104 鍵） | ✘ | ✔ 全部 38/38 欄 | ✔ |
| `(free) ReadTestMode` | `cprod.cpp` | `TestMode.Data` | ReadIniData | :3464（27 鍵） | ✔ 27 鍵 | — | ✘ | — | ✘ | ✔ 全部 5/5 欄 | — |
| `TfAGV` | `Automation/AGV.cpp` | `—` | ReadIniData | :1227（26 鍵） | ✘ | :1264 | ✘ | — | ✘ | ✔ 全部 3/3 欄 | — |
| `TfAutoAlignment` | `AutoAlignment/AutoAlignment.cpp` | `HandlerCondition.Data` | ReadIniData | :1373（26 鍵） | ✘ | :1433 | ✘ | :426（11 鍵） | ✘ | ✔ 全部 20/20 欄 | — |
| `TfTrayMapping` | `cTrayMapping.cpp` | `HandlerCondition.Data` | ReadIniData | :513（23 鍵） | ✘ | :407 | ✘ | — | ✘ | ✔ 全部 23/23 欄 | — |
| `TfQAMode` | `QAMode.cpp` | `Tester.Data` | ReadIniData | :179（14 鍵） | ✔ 14 鍵 | :38 | ✔ | — | ✘ | ✔ 全部 2/2 欄 | — |
| `TfFixAICCD` | `FixAICCD.cpp` | `—` | ReadIniData | :95（10 鍵） | ✘ | :77 | ✘ | — | ✘ | ✔ 全部 10/10 欄 | — |
| `TfRFID` | `MR/RFID.cpp` | `—` | ReadIniData | :196（10 鍵） | ✘ | :215 | ✘ | — | ✘ | ✔ 全部 5/5 欄 | — |
| `TfMagazine` | `Magazine.cpp` | `HandlerCondition.Data、config.ini` | ReadIniData | :3613（4 鍵） | ✘ | :3654 | ✘ | — | ✘ | ✔ 全部 2/2 欄 | — |
| `(free) ReadESDDataFile` | `csystem.cpp` | `—` | ReadIniData | :23411（28 鍵） | ✔ 28 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_RMS` | `cprod.cpp` | `config.ini` | ReadIniData | :2085（25 鍵） | ✔ 25 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Index` | `cprod.cpp` | `config.ini` | ReadIniData | :2557（24 鍵） | ✔ 24 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Count` | `cprod.cpp` | `HandlerCondition.Data、config.ini` | ReadIniData | :2462（21 鍵） | ✔ 20 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_FTP` | `cprod.cpp` | `config.ini` | ReadIniData | :2261（16 鍵） | ✔ 15 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `TfBinSel` | `cBinSel.cpp` | `Binasgn.Data、BinasgnOff.Data、BinasgnOff_ART.Data、Binasgn_ART.Data、Binasgn_MRT.Data、Binasgn_MRT_RT.Data` | ReadIniData | :1121（16 鍵） | ✔ 16 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Specific` | `cprod.cpp` | `config.ini` | ReadIniData | :2766（15 鍵） | ✔ 15 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `TfContactForce` | `ContactForce.cpp` | `—` | ReadIniData | :966（13 鍵） | ✘ | — | ✘ | :1183（13 鍵） | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Visible` | `cprod.cpp` | `config.ini` | ReadIniData | :2721（11 鍵） | ✔ 11 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Contact_Force` | `cprod.cpp` | `config.ini` | ReadIniData | :2688（9 鍵） | ✔ 9 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `TfTrayForm` | `cTrayForm.cpp` | `Tray.Data` | 兩套 | :364（9 鍵） | ✘ | :318 | ✘ | :643（9 鍵） | ✘ | — 讀檔器不在此 class | ✔ |
| `(free) ReadTasterInfo` | `cprod.cpp` | `config.ini` | ReadIniData | :3244（7 鍵） | ✔ 7 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadWriteTrayID` | `csystem.cpp` | `—` | ReadIniData | :24783（7 鍵） | ✔ 7 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_QA_Mode` | `cprod.cpp` | `config.ini` | ReadIniData | :2649（5 鍵） | ✔ 4 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Socket` | `cprod.cpp` | `config.ini` | ReadIniData | :2709（4 鍵） | ✔ 4 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadRmsPath` | `cprod.cpp` | `config.ini` | ReadIniData | :3272（4 鍵） | ✔ 4 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `TfLaserSensor` | `OmronLaser/LaserSensor.cpp` | `HandlerCondition.Data` | 兩套 | :1218（4 鍵） | ✔ 4 鍵 | :1248 | ✔ | — | ✘ | — 讀檔器不在此 class | — |
| `TfTeach` | `uteach.cpp` | `—` | 兩套 | :4785（4 鍵） | ✔ 4 鍵 | — | ✘ | :4939（2 鍵） | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_EventLog` | `cprod.cpp` | `config.ini` | ReadIniData | :2379（2 鍵） | ✔ 2 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Tray` | `cprod.cpp` | `config.ini` | ReadIniData | :2542（2 鍵） | ✔ 2 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadRmsInfo` | `cprod.cpp` | `config.ini` | ReadIniData | :3165（2 鍵） | ✔ 2 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadWriteBinCountMode` | `csystem.cpp` | `—` | ReadIniData | :24655（2 鍵） | ✔ 2 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Auto_Clean` | `cprod.cpp` | `config.ini` | ReadIniData | :2737（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_SingleTempLimit` | `cprod.cpp` | `config.ini` | ReadIniData | :2669（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ProcessLastSetIni_Tester` | `cprod.cpp` | `config.ini` | ReadIniData | :2529（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadEventLogAutoSaveInfo` | `cprod.cpp` | `config.ini` | ReadIniData | :3188（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) ReadWriteAutoCleanCount` | `AutoClean/AutoClean.cpp` | `—` | ReadIniData | :1269（1 鍵） | ✔ 1 鍵 | — | ✘ | — | ✘ | — 讀檔器不在此 class | — |
| `(free) SaveEventLogAutoSaveInfo` | `cprod.cpp` | `config.ini` | ReadIniData | — | ✘ | — | ✘ | :3177（2 鍵） | ✔ 2 鍵 | — 讀檔器不在此 class | — |
| `(free) SaveRmsInfo` | `cprod.cpp` | `config.ini` | ReadIniData | — | ✘ | — | ✘ | :3150（2 鍵） | ✔ 2 鍵 | — 讀檔器不在此 class | — |
| `(free) SaveTasterInfo` | `cprod.cpp` | `config.ini` | ReadIniData | — | ✘ | — | ✘ | :3229（7 鍵） | ✔ 7 鍵 | — 讀檔器不在此 class | — |
| `(free) SaveTempMode` | `cprod.cpp` | `Temperature.Data` | ReadIniData | — | ✘ | — | ✘ | :3645（1 鍵） | ✔ 1 鍵 | — 讀檔器不在此 class | — |
| `(free) SaveTempModeByDLL` | `cprod.cpp` | `Temperature.Data` | ReadIniData | — | ✘ | — | ✘ | :3657（1 鍵） | ✔ 1 鍵 | — 讀檔器不在此 class | — |
| `(free) SaveTestMode` | `cprod.cpp` | `TestMode.Data` | ReadIniData | — | ✘ | — | ✘ | :3314（27 鍵） | ✔ 27 鍵 | — 讀檔器不在此 class | — |
| `(free) WriteESDDataFile` | `csystem.cpp` | `—` | ReadIniData | — | ✘ | — | ✘ | :23482（24 鍵） | ✔ 24 鍵 | — 讀檔器不在此 class | — |
| `TFTestIF` | `cTesterIF.cpp` | `Tester.Data` | ReadIniData | — | ✘ | :997 | ✔ | :337（92 鍵） | ✔ 92 鍵 | — 讀檔器不在此 class | ✔ |
| `TZteach` | `AutoTeach/InOutArmZteach.cpp` | `Position Offset Hot.Data、Position Offset.Data` | ReadIniData | — | ✘ | — | ✘ | :4674（11 鍵） | ✘ | — 讀檔器不在此 class | — |
| `TfCCLink` | `CCLink/MyCCLinkSensor.cpp` | `CCLink.Data` | ReadIniData | — | ✘ | — | ✘ | :648（2 鍵） | ✘ | — 讀檔器不在此 class | — |
| `TfLd_ULd` | `cLd_ULd.cpp` | `UdUld.Data` | HTEditList | — | ✔ | :156 | ✔ | — | ✔ | — 讀檔器不在此 class | ✔ |
| `TfTemp_Set` | `uTemp_Set.cpp` | `Temperature.Data、Tester.Data` | ReadIniData | — | ✘ | :3182 | ✔ | :4606（249 鍵） | ✔ 248 鍵 | — 讀檔器不在此 class | ✔ |
| `TfVacuumUnit` | `VacuumUnit/VacuumUnit.cpp` | `HandlerCondition.Data` | HTEditList | — | ✔ | :363 | ✔ | — | ✔ | — 讀檔器不在此 class | — |

## 已出 JSON 的欄位（全部）

- `TestIF_File.bAdaptiveLowYield` ← `TfYieldMonitoring`
- `TestIF_File.bAdaptiveLowYield_RT` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm4ContinueType_Enable` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm4EnableIntervalYield` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm5_BySiteAlarmYieldEnable` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm5_BySiteCmpYieldEnable` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm5_BySiteLowYieldEnable` ← `TfYieldMonitoring`
- `TestIF_File.bAlarm5_BySitePreCmpYieldEnable` ← `TfYieldMonitoring`
- `TestIF_File.bAllSiteFail` ← `TfYieldMonitoring`
- `TestIF_File.bAllSiteFail_RT` ← `TfYieldMonitoring`
- `TestIF_File.bByBinFailureEnable` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousContact` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousContact_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousLoader` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousLoader_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousPass` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousPassBySocket` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousPassBySocket_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContinuousPass_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailByHead` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailByHead_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailBySocket` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailBySocket_RT` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailIgnore` ← `TfYieldMonitoring`
- `TestIF_File.bContsFailIgnore_RT` ← `TfYieldMonitoring`
- `TestIF_File.bCountSpcBinContinuously_FT` ← `TfYieldMonitoring`
- `TestIF_File.bCountSpcBinContinuously_RT` ← `TfYieldMonitoring`
- `TestIF_File.bCreateManualEOCAP` ← `TfYieldMonitoring`
- `TestIF_File.bEnableOpenShortART` ← `TfYieldMonitoring`
- `TestIF_File.bEnablePassYieldART` ← `TfYieldMonitoring`
- `TestIF_File.bEnableRecoverART` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmIntervalLowYieldBySite` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmIntervalLowYieldBySite_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmIntervalLowYieldByTotal` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmIntervalLowYieldByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYield` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYieldByTotal` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYieldByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYieldSpecial` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYield_AutoClean` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmLowYield_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmSiteYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmSiteYieldCmp_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmSiteYieldDifferent` ← `TfYieldMonitoring`
- `TestIF_File.bFailAlarmSiteYieldDifferent_RT` ← `TfYieldMonitoring`
- `TestIF_File.bFailCountEnable` ← `TfYieldMonitoring`
- `TestIF_File.bFailRateMode` ← `TfYieldMonitoring`
- `TestIF_File.bFailRateMode_RT` ← `TfYieldMonitoring`
- `TestIF_File.bHeadToHeadYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.bLoadCellMeasure` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAlarmByBin` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOff` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOffAlarm` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOffArmContiFail` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOffByArmSite` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOffByContiFail` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldAutoSiteOffByPicker` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldByPicker` ← `TfYieldMonitoring`
- `TestIF_File.bLowYieldByPicker_RT` ← `TfYieldMonitoring`
- `TestIF_File.bSiteToSiteYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.bSiteYieldOverAlert` ← `TfYieldMonitoring`
- `TestIF_File.bSlidingWindowYield` ← `TfYieldMonitoring`
- `TestIF_File.bSpecBinByArmPerSiteCompareEnable` ← `TfYieldMonitoring`
- `TestIF_File.bSpecBinBySiteCompareEnable` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteAlarmYield` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteAlarmYieldRej` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteCmpYield` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteCmpYieldRej` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteLowYield` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySiteLowYieldRej` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySitePreCmpYield` ← `TfYieldMonitoring`
- `TestIF_File.dAlarm5_BySitePreCmpYieldRej` ← `TfYieldMonitoring`
- `TestIF_File.dByBinFailurePercent` ← `TfYieldMonitoring`
- `TestIF_File.dFailAlarmSiteYield` ← `TfYieldMonitoring`
- `TestIF_File.dFailAlarmSiteYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.dFailAlarmSiteYieldCmp_RT` ← `TfYieldMonitoring`
- `TestIF_File.dFailAlarmSiteYield_RT` ← `TfYieldMonitoring`
- `TestIF_File.dIntervalLowYieldLimitBySite` ← `TfYieldMonitoring`
- `TestIF_File.dIntervalLowYieldLimitBySite_RT` ← `TfYieldMonitoring`
- `TestIF_File.dIntervalLowYieldLimitByTotal` ← `TfYieldMonitoring`
- `TestIF_File.dIntervalLowYieldLimitByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldByPicker` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldByPicker_RT` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldLimit` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldLimitByTotal` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldLimitByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldLimitSpecial` ← `TfYieldMonitoring`
- `TestIF_File.dLowYieldLimit_RT` ← `TfYieldMonitoring`
- `TestIF_File.dSpecBinByArmPerSiteComparePercent` ← `TfYieldMonitoring`
- `TestIF_File.dSpecBinBySiteComparePercent` ← `TfYieldMonitoring`
- `TestIF_File.fOpenShortYieldART` ← `TfYieldMonitoring`
- `TestIF_File.fPassYieldART` ← `TfYieldMonitoring`
- `TestIF_File.fRecoverYieldART` ← `TfYieldMonitoring`
- `TestIF_File.iACAlarmType` ← `TfYieldMonitoring`
- `TestIF_File.iACCounts` ← `TfYieldMonitoring`
- `TestIF_File.iACGroupMethod` ← `TfYieldMonitoring`
- `TestIF_File.iACPeriod` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveContsLowerAlarmMin` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveContsLowerAlarmMin_RT` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveContsLowerAlarmNor` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveContsLowerAlarmNor_RT` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveYieldMax` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveYieldMax_RT` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveYieldMin` ← `TfYieldMonitoring`
- `TestIF_File.iAdaptiveYieldMin_RT` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm4ContinueType_ContinueCount` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm4ContinueType_IntervalCount` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm4IntervalYieldIntervalCount` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm4IntervalYieldYield` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm5_BySiteIntervalContactCnt` ← `TfYieldMonitoring`
- `TestIF_File.iAlarm5_OSBin` ← `TfYieldMonitoring`
- `TestIF_File.iAlarmWhenSiteOnCountLess` ← `TfYieldMonitoring`
- `TestIF_File.iAllSiteFailCount` ← `TfYieldMonitoring`
- `TestIF_File.iAllSiteFailCountRT` ← `TfYieldMonitoring`
- `TestIF_File.iByBinFailureIgnore` ← `TfYieldMonitoring`
- `TestIF_File.iCloseSiteBin` ← `TfYieldMonitoring`
- `TestIF_File.iCloseSiteOnHPDontTest` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousContactCount` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousContactCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousLoaderCount` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousLoaderCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBin` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBinCount` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBinCountBySocket` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBinCountBySocket_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBinCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContinuousPassBin_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailHeadAlarmCT` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailHeadAlarmCT_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailIgnore` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailIgnore_RT` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailSocketAlarmCT` ← `TfYieldMonitoring`
- `TestIF_File.iContsFailSocketAlarmCT_RT` ← `TfYieldMonitoring`
- `TestIF_File.iCountAlarmAction` ← `TfYieldMonitoring`
- `TestIF_File.iCountAlarmAction_RT` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYield` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldCmpCount` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldCmpCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldCmp_RT` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldDifferentCount` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYieldDifferentCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iFailAlarmSiteYield_RT` ← `TfYieldMonitoring`
- `TestIF_File.iFailCountIgnore` ← `TfYieldMonitoring`
- `TestIF_File.iFailCountLimit` ← `TfYieldMonitoring`
- `TestIF_File.iHeadToHeadYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.iHeadToHeadYieldCmpCount` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldCountBySite` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldCountBySite_RT` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldCountByTotal` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldCountByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldLimitBySite` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldLimitBySite_RT` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldLimitByTotal` ← `TfYieldMonitoring`
- `TestIF_File.iIntervalLowYieldLimitByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.iLoadCellCount` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldAutoSiteOffAlarm` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCount` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountByPicker` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountByPicker_RT` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountByTotal` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountSpecial1` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCountSpecial2` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCount_AutoClean` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldCount_RT` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimit` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimitByTotal` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimitByTotal_RT` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimitSpecial` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimit_AutoClean` ← `TfYieldMonitoring`
- `TestIF_File.iLowYieldLimit_RT` ← `TfYieldMonitoring`
- `TestIF_File.iSiteToSiteYieldCmp` ← `TfYieldMonitoring`
- `TestIF_File.iSiteToSiteYieldCmpCount` ← `TfYieldMonitoring`
- `TestIF_File.iSiteYieldOverAlert` ← `TfYieldMonitoring`
- `TestIF_File.iSiteYieldOverAlertCount` ← `TfYieldMonitoring`
- `TestIF_File.iSlidingWindowSize` ← `TfYieldMonitoring`
- `TestIF_File.iSpecBinByArmPerSiteCompareIgnore` ← `TfYieldMonitoring`
- `TestIF_File.iSpecBinBySiteCompareIgnore` ← `TfYieldMonitoring`
- `TestIF_File.bIndexCycleTimeMonitor` ← `TfSpeed`
- `TestIF_File.bIndexPickICWhenOutShtNoIC` ← `TfSpeed`
- `TestIF_File.dICTTolerance` ← `TfSpeed`
- `TestIF_File.dIndexCycletimeMonitor` ← `TfSpeed`
- `TestIF_File.dMonitorOutlier` ← `TfSpeed`
- `TestIF_File.iICTAction` ← `TfSpeed`
- `TestIF_File.iInArmToShtReleaseMode` ← `TfSpeed`
- `TestIF_File.iMonitorWindow` ← `TfSpeed`
- `TestIF_File.iShakeShuttleWhenPlaceIC` ← `TfSpeed`
- `TestIF_File.UseRotateForHT7000HPKit` ← `TfSetup`
- `TestIF_File.b12SiteUse10Heater` ← `TfSetup`
- `TestIF_File.b16Direct12Shuttle` ← `TfSetup`
- `TestIF_File.b16Direct8Shuttle` ← `TfSetup`
- `TestIF_File.b1CableLayoutKit` ← `TfSetup`
- `TestIF_File.b1x2Use1x4SiteKit` ← `TfSetup`
- `TestIF_File.b2CableLayoutKit` ← `TfSetup`
- `TestIF_File.b2x2Use16SiteKit` ← `TfSetup`
- `TestIF_File.b2x6Use2x8SitSLK` ← `TfSetup`
- `TestIF_File.b6CableLayoutKit` ← `TfSetup`
- `TestIF_File.bArm1PickPlaceArm2Test` ← `TfSetup`
- `TestIF_File.bArm1PickPlaceArm2Test_RunAutoClean` ← `TfSetup`
- `TestIF_File.bArm1UseHeat` ← `TfSetup`
- `TestIF_File.bAutoSiteMappingOneCycle` ← `TfSetup`
- `TestIF_File.bAutoSiteMappingOpenSite` ← `TfSetup`
- `TestIF_File.bCheckArm2Vacuum` ← `TfSetup`
- `TestIF_File.bEnSocketSensor` ← `TfSetup`
- `TestIF_File.bEnablePreciserHotPlate` ← `TfSetup`
- `TestIF_File.bEnableRTPreciser` ← `TfSetup`
- `TestIF_File.bEnableUsePreciser` ← `TfSetup`
- `TestIF_File.bEnableUseXCenterPitch` ← `TfSetup`
- `TestIF_File.bF18InshuttleDetect` ← `TfSetup`
- `TestIF_File.bHontechLayoutKit2x2` ← `TfSetup`
- `TestIF_File.bInArmUseBackRowSuck` ← `TfSetup`
- `TestIF_File.bIndEPSLK` ← `TfSetup`
- `TestIF_File.bNS7000CS` ← `TfSetup`
- `TestIF_File.bNS7000kit` ← `TfSetup`
- `TestIF_File.bNS8000CS` ← `TfSetup`
- `TestIF_File.bNSKitPress` ← `TfSetup`
- `TestIF_File.bOcrFunction` ← `TfSetup`
- `TestIF_File.bOctal_12Kit` ← `TfSetup`
- `TestIF_File.bOctal_16Kit` ← `TfSetup`
- `TestIF_File.bOctal_80Kit` ← `TfSetup`
- `TestIF_File.bOutArmUseBackRowSuck` ← `TfSetup`
- `TestIF_File.bQualSite2X2Shift` ← `TfSetup`
- `TestIF_File.bRTC20CheckFunction` ← `TfSetup`
- `TestIF_File.bRTC20GiveWayCheck` ← `TfSetup`
- `TestIF_File.bRTC20OverFlowCheck` ← `TfSetup`
- `TestIF_File.bRTCICResidueCheck` ← `TfSetup`
- `TestIF_File.bRotateShuttle` ← `TfSetup`
- `TestIF_File.bSingleHeater` ← `TfSetup`
- `TestIF_File.bSingleInArmUseOtherSuck` ← `TfSetup`
- `TestIF_File.bSingleUseOtherSuck` ← `TfSetup`
- `TestIF_File.bSocketDisibleinitialcheck` ← `TfSetup`
- `TestIF_File.bSocketSensorCheckFloating` ← `TfSetup`
- `TestIF_File.bSquare_OctalKit` ← `TfSetup`
- `TestIF_File.bUse1x3SiteKit` ← `TfSetup`
- `TestIF_File.bUse32Heater` ← `TfSetup`
- `TestIF_File.bUseRTCStepAsideMode` ← `TfSetup`
- `TestIF_File.bUseSLKClamp` ← `TfSetup`
- `TestIF_File.bUseSocketFloat` ← `TfSetup`
- `TestIF_File.dPreciserXPitch` ← `TfSetup`
- `TestIF_File.dPreciserYPitch` ← `TfSetup`
- `TestIF_File.dShiftXPitch` ← `TfSetup`
- `TestIF_File.dSiteXCenterPitch` ← `TfSetup`
- `TestIF_File.dSiteXPitch` ← `TfSetup`
- `TestIF_File.dSiteYOffset` ← `TfSetup`
- `TestIF_File.dSiteYPitch` ← `TfSetup`
- `TestIF_File.i1x4SiteYOffset` ← `TfSetup`
- `TestIF_File.iARM_Y_PITCH` ← `TfSetup`
- `TestIF_File.iSensorCheckType` ← `TfSetup`
- `TestIF_File.iSeparabilityTest` ← `TfSetup`
- `TestIF_File.iShuttleMode` ← `TfSetup`
- `TestIF_File.iShuttle_Sel` ← `TfSetup`
- `TestIF_File.iSiteMap` ← `TfSetup`
- `TestIF_File.iSocketCount` ← `TfSetup`
- `TestIF_File.iUseSuckMode` ← `TfSetup`
- `TestIF_File.iYPitchOffsetMode` ← `TfSetup`
- `TestIF_File.sOcrText` ← `TfSetup`
- `TestIF_File.sRtcFileName` ← `TfSetup`
- `TestIF_File.sTestMode` ← `TfSetup`
- `TestIF_File.as2DInsertString` ← `TfBarCode`
- `TestIF_File.asMes2DID_URL` ← `TfBarCode`
- `TestIF_File.b2DIDAllowList` ← `TfBarCode`
- `TestIF_File.b2DIDListErrorBin` ← `TfBarCode`
- `TestIF_File.b2DIDNotExist2Error` ← `TfBarCode`
- `TestIF_File.b2DIDStringFormat` ← `TfBarCode`
- `TestIF_File.b2DIDYield` ← `TfBarCode`
- `TestIF_File.b2DTriggerMode` ← `TfBarCode`
- `TestIF_File.b2DUseAnyChar` ← `TfBarCode`
- `TestIF_File.b2DUsePinInspection` ← `TfBarCode`
- `TestIF_File.b2DUseSubJob` ← `TfBarCode`
- `TestIF_File.b2DUseUndefinedCMD` ← `TfBarCode`
- `TestIF_File.bBarCodeInspReport` ← `TfBarCode`
- `TestIF_File.bBarCodeMultiRecipe` ← `TfBarCode`
- `TestIF_File.bCheckCodeByLot` ← `TfBarCode`
- `TestIF_File.bCheckCodeByServer2DID` ← `TfBarCode`
- `TestIF_File.bCheckCodeByShuttle` ← `TfBarCode`
- `TestIF_File.bCheckLotHaveCode` ← `TfBarCode`
- `TestIF_File.bCheckSum` ← `TfBarCode`
- `TestIF_File.bChkMakeWhite2DIDList` ← `TfBarCode`
- `TestIF_File.bEnableBarCode` ← `TfBarCode`
- `TestIF_File.bEnableBottom2D` ← `TfBarCode`
- `TestIF_File.bEnableConsecutiveFailure` ← `TfBarCode`
- `TestIF_File.bEnableMulti2D` ← `TfBarCode`
- `TestIF_File.bEnableShtFloatChk` ← `TfBarCode`
- `TestIF_File.bLotIDVerify` ← `TfBarCode`
- `TestIF_File.bNoCodeDeviceAutoSkip` ← `TfBarCode`
- `TestIF_File.bRetryOffsetMove` ← `TfBarCode`
- `TestIF_File.bRetryShiftOffsetMove` ← `TfBarCode`
- `TestIF_File.bSFCUse2Photo` ← `TfBarCode`
- `TestIF_File.bSaveFailImage` ← `TfBarCode`
- `TestIF_File.bSearch2DIDByLot` ← `TfBarCode`
- `TestIF_File.bSetCloseSite2DIDtoEmpty` ← `TfBarCode`
- `TestIF_File.bSortingBy2DIDList` ← `TfBarCode`
- `TestIF_File.bUseBarcodeAutoAdjustLight` ← `TfBarCode`
- `TestIF_File.bUseHandShakeCommunication` ← `TfBarCode`
- `TestIF_File.d2DIDYield` ← `TfBarCode`
- `TestIF_File.dBottom2DOffsetX` ← `TfBarCode`
- `TestIF_File.dBottom2DOffsetY` ← `TfBarCode`
- `TestIF_File.dMulti2DXPitch` ← `TfBarCode`
- `TestIF_File.dRetryOffsetMove` ← `TfBarCode`
- `TestIF_File.dRetryShiftOffsetMove` ← `TfBarCode`
- `TestIF_File.i2D1stLineLength` ← `TfBarCode`
- `TestIF_File.i2D2ndLineLength` ← `TfBarCode`
- `TestIF_File.i2DHandShakeTimeOut` ← `TfBarCode`
- `TestIF_File.i2DIDStrEnd` ← `TfBarCode`
- `TestIF_File.i2DIDStrStart` ← `TfBarCode`
- `TestIF_File.i2DReadMultiLine` ← `TfBarCode`
- `TestIF_File.i2DYieldIgnoreCnt` ← `TfBarCode`
- `TestIF_File.iActionOf2DNotInList` ← `TfBarCode`
- `TestIF_File.iAutoAdjustLightTimeOut` ← `TfBarCode`
- `TestIF_File.iBarCodeDelay` ← `TfBarCode`
- `TestIF_File.iBarCodeMaxLength` ← `TfBarCode`
- `TestIF_File.iBarCodeMinLength` ← `TfBarCode`
- `TestIF_File.iBarCodePos1Delay` ← `TfBarCode`
- `TestIF_File.iBarCodePosDelay` ← `TfBarCode`
- `TestIF_File.iCheckSumLength` ← `TfBarCode`
- `TestIF_File.iConsecutiveFailure` ← `TfBarCode`
- `TestIF_File.iEnableAllSite2DIDErr` ← `TfBarCode`
- `TestIF_File.iLotIDVerifyE` ← `TfBarCode`
- `TestIF_File.iLotIDVerifyS` ← `TfBarCode`
- `TestIF_File.iMulti2DMap` ← `TfBarCode`
- `TestIF_File.iMulti2DType` ← `TfBarCode`
- `TestIF_File.iNoCodeDeviceToErr` ← `TfBarCode`
- `TestIF_File.iSFCAutoRetry` ← `TfBarCode`
- `TestIF_File.iSFCExposureTimeOut` ← `TfBarCode`
- `TestIF_File.iSFCGetResultTimeOut` ← `TfBarCode`
- `TestIF_File.iSFCStartDelay` ← `TfBarCode`
- `TestIF_File.iSFCUse2PhotoOffset` ← `TfBarCode`
- `TestIF_File.iSelectUseCCDSh1` ← `TfBarCode`
- `TestIF_File.iSelectUseCCDSh2` ← `TfBarCode`
- `TestIF_File.iShtDuplicateRetryCnt` ← `TfBarCode`
- `TestIF_File.s2DFileName` ← `TfBarCode`
- `TestIF_File.sLotIDSubstr` ← `TfBarCode`
- `TestIF_File.sLotIDVerify` ← `TfBarCode`
- `TestIF_File.str2DTriggerOFFCMD` ← `TfBarCode`
- `TestIF_File.str2DTriggerONCMD` ← `TfBarCode`
- `DeviceForm_File.ContactMode` ← `TfContact`
- `DeviceForm_File.DieForcePerPinG` ← `TfContact`
- `DeviceForm_File.DieForcePerPinN` ← `TfContact`
- `DeviceForm_File.DoubleForce` ← `TfContact`
- `DeviceForm_File.DropSpeed` ← `TfContact`
- `DeviceForm_File.DropWait` ← `TfContact`
- `DeviceForm_File.DummyMode` ← `TfContact`
- `DeviceForm_File.ForcePerPinG` ← `TfContact`
- `DeviceForm_File.ForcePerPinN` ← `TfContact`
- `DeviceForm_File.IndexArmPick` ← `TfContact`
- `DeviceForm_File.IndexContactBackUp` ← `TfContact`
- `DeviceForm_File.IndexContactShuttlePickUp` ← `TfContact`
- `DeviceForm_File.IndexDrop` ← `TfContact`
- `DeviceForm_File.IndexPlace` ← `TfContact`
- `DeviceForm_File.IndexUp` ← `TfContact`
- `DeviceForm_File.UpSpeed` ← `TfContact`
- `DeviceForm_File.UpWait` ← `TfContact`
- `DeviceForm_File.VacuumMode` ← `TfContact`
- `DeviceForm_File.XDimension` ← `TfContact`
- `DeviceForm_File.YDimension` ← `TfContact`
- `DeviceForm_File.bTesterSidePush` ← `TfContact`
- `DeviceForm_File.dDieForceKitDiameter` ← `TfContact`
- `DeviceForm_File.dKitDiameter` ← `TfContact`
- `DeviceForm_File.dLoadCellZ1Down` ← `TfContact`
- `DeviceForm_File.dLoadCellZ2Down` ← `TfContact`
- `DeviceForm_File.dSitePushWaitTime` ← `TfContact`
- `DeviceForm_File.dZ1Torue` ← `TfContact`
- `DeviceForm_File.dZ2Torue` ← `TfContact`
- `DeviceForm_File.iAutoHeightSHTReleaseOfs` ← `TfContact`
- `DeviceForm_File.iHeadDeviceCT` ← `TfContact`
- `DeviceForm_File.iKitDiameterMode` ← `TfContact`
- `DeviceForm_File.iPinCT` ← `TfContact`
- `DeviceForm_File.iPinOfDie` ← `TfContact`
- `DeviceForm_File.iPurgeBdforePickShuttleOffSet` ← `TfContact`
- `DeviceForm_File.iPurgeBeforePickShuttleInterval` ← `TfContact`
- `DeviceForm_File.iPurgeBeforePickShuttleTime` ← `TfContact`
- `DeviceForm_File.iSidePushMode` ← `TfContact`
- `DeviceForm_File.iSocketInitialICCheckPosition` ← `TfContact`
- `TrayForm.AutoCoverInitial` ← `TfTrayAssignment`
- `TrayForm.AutoCoverRetest` ← `TfTrayAssignment`
- `TrayForm.AutoFromEmptyColor` ← `TfTrayAssignment`
- `TrayForm.LoaderToEmptyColor` ← `TfTrayAssignment`
- `TrayForm.LodareType` ← `TfTrayAssignment`
- `TrayForm.asNoRTBinFix` ← `TfTrayAssignment`
- `TrayForm.bAutoFeed` ← `TfTrayAssignment`
- `TrayForm.bChkLoadDirection` ← `TfTrayAssignment`
- `TrayForm.bColorTray` ← `TfTrayAssignment`
- `TrayForm.bEnableAMR` ← `TfTrayAssignment`
- `TrayForm.bEnableAMRLoader` ← `TfTrayAssignment`
- `TrayForm.bFailAutoTrayManual_FT` ← `TfTrayAssignment`
- `TrayForm.bFailAutoTrayManual_RT` ← `TfTrayAssignment`
- `TrayForm.bIDTrayOrder` ← `TfTrayAssignment`
- `TrayForm.bMoveAfterTrayGoOut` ← `TfTrayAssignment`
- `TrayForm.bSpecTrayCnt` ← `TfTrayAssignment`
- `TrayForm.bTraySortCntFunc` ← `TfTrayAssignment`
- `TrayForm.bTrayUpDownSet` ← `TfTrayAssignment`
- `TrayForm.bVTestNoRTBin` ← `TfTrayAssignment`
- `TrayForm.iFixTrayMode` ← `TfTrayAssignment`
- `TrayForm.iFullTrayCount` ← `TfTrayAssignment`
- `TrayForm.iInputTrayCount` ← `TfTrayAssignment`
- `TrayForm.iManualRemoveLoader` ← `TfTrayAssignment`
- `TrayForm.iReaderPos` ← `TfTrayAssignment`
- `TrayForm.iTrayOrder` ← `TfTrayAssignment`
- `TrayForm.iTraySortCntFunc` ← `TfTrayAssignment`
- `TestMode.iDutOnOff` ← `(free) ReadTestMode`
- `TestMode.iDutOnOffEE` ← `(free) ReadTestMode`
- `TestMode.iRunMode` ← `(free) ReadTestMode`
- `TestMode.iTemperatureMode` ← `(free) ReadTestMode`
- `TestMode.iTestConnection` ← `(free) ReadTestMode`
- `TestIF_File.bEnableE84` ← `TfAGV`
- `TestIF_File.iE84TimeOut_K12` ← `TfAGV`
- `TestIF_File.iLoaderUnloaderTrayCount` ← `TfAGV`
- `TestIF_File.AutoAlignmentFileName` ← `TfAutoAlignment`
- `TestIF_File.bEnableAutoAlignment` ← `TfAutoAlignment`
- `TestIF_File.iAOA_DecodeTimeOut` ← `TfAutoAlignment`
- `TestIF_File.iAlignmentPlatePointX` ← `TfAutoAlignment`
- `TestIF_File.iAlignmentPlatePointY` ← `TfAutoAlignment`
- `TestIF_File.iAlignmentPointX` ← `TfAutoAlignment`
- `TestIF_File.iAlignmentPointY` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentCK_InArmZPickUpOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentCK_InArmZRealaseOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentCK_OutArmZPickUpOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentCK_OutArmZRealaseOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentShuttleHotplateEvent` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentShuttleHotplateEvent_Z` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTrayEvent` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTrayEvent_Z` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTray_InArmZPickUpOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTray_InArmZRealaseOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTray_OutArmZPickUpOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignmentTray_OutArmZRealaseOffset` ← `TfAutoAlignment`
- `TestIF_File.iAutoAlignment_DeviceThick` ← `TfAutoAlignment`
- `TestIF_File.bCheckTrayIDBylot` ← `TfTrayMapping`
- `TestIF_File.bDisableMapSuck` ← `TfTrayMapping`
- `TestIF_File.bEnableDeviceRemain` ← `TfTrayMapping`
- `TestIF_File.bEnableOCRTrayIDDown` ← `TfTrayMapping`
- `TestIF_File.bEnableSuckMapCheck` ← `TfTrayMapping`
- `TestIF_File.bEnableTrayDeviceCnt` ← `TfTrayMapping`
- `TestIF_File.bEnableTrayID` ← `TfTrayMapping`
- `TestIF_File.bEnableTrayID2` ← `TfTrayMapping`
- `TestIF_File.bEnableTrayIDDownFTP` ← `TfTrayMapping`
- `TestIF_File.bEnableTrayMap` ← `TfTrayMapping`
- `TestIF_File.iTrayAutoRetry` ← `TfTrayMapping`
- `TestIF_File.iTrayCodeMaxLength` ← `TfTrayMapping`
- `TestIF_File.iTrayCodeMinLength` ← `TfTrayMapping`
- `TestIF_File.iTrayDeciveCntStart` ← `TfTrayMapping`
- `TestIF_File.iTrayExposureTimeOut` ← `TfTrayMapping`
- `TestIF_File.iTrayGetResultTimeOut` ← `TfTrayMapping`
- `TestIF_File.iTrayID2Shift` ← `TfTrayMapping`
- `TestIF_File.iTrayIDReadShift` ← `TfTrayMapping`
- `TestIF_File.iTrayIDShift` ← `TfTrayMapping`
- `TestIF_File.iTrayMapCatch` ← `TfTrayMapping`
- `TestIF_File.iTrayMapShift` ← `TfTrayMapping`
- `TestIF_File.iTrayMapStart` ← `TfTrayMapping`
- `TestIF_File.iTrayStartDelay` ← `TfTrayMapping`
- `TestIF_File.iQAModeBin` ← `TfQAMode`
- `TestIF_File.iQAModeRunType` ← `TfQAMode`
- `TestIF_File.bEnableFix2BGAAICCD` ← `TfFixAICCD`
- `TestIF_File.bEnableLearningMode` ← `TfFixAICCD`
- `TestIF_File.dInspectResultThres` ← `TfFixAICCD`
- `TestIF_File.iBGALightScrPos` ← `TfFixAICCD`
- `TestIF_File.iFix2BGAAICCDAutoRetry` ← `TfFixAICCD`
- `TestIF_File.iFix2BGAAICCDExposureTimeOut` ← `TfFixAICCD`
- `TestIF_File.iFix2BGAAICCDGetResultTimeOut` ← `TfFixAICCD`
- `TestIF_File.iFix2BGAAICCDOutArmCycleInsp` ← `TfFixAICCD`
- `TestIF_File.iFix2BGAAICCDStartDelay` ← `TfFixAICCD`
- `TestIF_File.iResultShowType` ← `TfFixAICCD`
- `TestIF_File.bEnableRFID` ← `TfRFID`
- `TestIF_File.bEnableView` ← `TfRFID`
- `TestIF_File.iE84TimeOut` ← `TfRFID`
- `TestIF_File.iRFIDDelay` ← `TfRFID`
- `TestIF_File.iRFIDRetryCount` ← `TfRFID`
- `TestIF_File.iMagDisplayOrder` ← `TfMagazine`
- `TestIF_File.iMagFixTrayType` ← `TfMagazine`

## 欄位說明

- **機制**：`ReadIniData`＝`ReadIniData`／`WriteIniData`；`HTEditList`＝`ReadEditTextFromFile`／`SaveEditTextToFile`；`兩套`＝同一表單兩種都用。細節見 `file-io-mechanisms.md`。
- **① BCB 讀檔**：golden 該函式的行號與 `ReadIniData` 呼叫數。`—` 表示這個 class 沒有 `ReadFile()`（讀檔器可能另有其名，如 `TFTestIF::ReadTestIFFile`，或走 `HTEditList`）。
- **JSON 中介層**：`Struct.field` 粒度，兩條路擇一即算：(a) 手寫 staging —— 比對 `WebBridgeTags.cpp`／`wb_serve.cpp` **剝掉註解後**的程式碼；(b) 表驅動（20260923 S2 起）—— `JsonBridge/Bindings.cpp` 的實例 × `JsonBridge/gen/sjson_<TYPE>.gen.cpp` 的 `kFields_` 表。⚠ 這欄記的是**介面有沒有**，不是**值真不真**（`deviceForm.file` 66 欄有介面但 `sourcePorted=false`，值全 0）；真不真看 `GET /api/struct/<binding>` 或 `references/porting-gaps.md`。用結構名比對會嚴重高估（`IniConfig` 整體有被 stage）。
- **SaveAllFile**：是否在 golden `csystem.cpp` `SaveAllFile()` 的權威寫檔名單裡。
- **(free)**：不掛在任何表單上的自由函式（`ReadTestMode`／`SaveTestMode` 等）。
