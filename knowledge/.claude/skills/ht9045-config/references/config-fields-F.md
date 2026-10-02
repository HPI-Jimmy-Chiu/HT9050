# HT9045_CONFIG 欄位速查：群組 [F] Shuttle（Shuttle 梭式機構）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Fxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 57 個欄位，分屬 40 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| F01 | Shake shuttle when jam happen.  Speed(%): | `cbF01` | `bF01ShakeShuttleWhenJam` | bool | `bF01ShakeShuttleWhenJam` | 35300 | ECBool | Shake shuttle when jam happen.  Speed(%): |  |
| F01 | Shake shuttle when jam happen.  Speed(%): | `edF01` | `iF01ShuttleShakeSpeed` | int | `iF01ShuttleShakeSpeed` | 35300 | ECBool | Shake shuttle when jam happen.  Speed(%): |  |
| F03 | Output shuttle skip detect  IC miss | `cbF03` | `bF03OutputShuttleSkipICMiss` | bool | `bF03OutputShuttleSkipICMiss` | 35302 | ECBool | Output shuttle skip detect  IC miss |  |
| F05 | Enable shuttlet clean function | `cbF05` | `bF05EnableShtPurgeFunction` | bool | `bF05EnableShtPurgeFunction` | 35303 | ECBool | Enable shuttlet clean function |  |
| F05 | Enable shuttlet clean function | `edF05` | `iF05ShuttlePurgeCount` | int | `iF05ShuttlePurgeCount` | 35303 | ECBool | Enable shuttlet clean function |  |
| F06 | Enable initial IC check | `cbF06` | `bF06InitialICCheck` | bool | `bF06InitialICCheck` | 35305 | ECBool | Enable initial IC check |  |
| F06 | Enable initial IC check | — | `bF06_Active` | bool | — | 35305 | — | Enable initial IC check | Steven 20140627 : Add for ASE-CL -- F06 打勾 |
| F06 | Enable initial IC check | — | `bF06_Enable` | bool | — | 35305 | — | Enable initial IC check | Steven 20140627 : Add for ASE-CL -- F06 Enable |
| F07 | Out shuttle sensor detect mode | `rgF07` | `iF07OutShuttleSensorMode` | int | `iF07OutShuttleSensorMode` | 35306 | ECInteger | Out shuttle sensor detect mode |  |
| F09 | Check IC which first time load | `cbF09` | `bF09CheckICWhichFirstTimeLoad` | bool | `bF09CheckICWhichFirstTimeLoad` | 35307 | ECBool | Check IC which first time load |  |
| F11 | Out shuttle use front rear sensor detect superfluous IC | `cbF11` | `bF11OutShtUseFrontRearSensor` | bool | `bF11OutShtUseFrontRearSensor` | 35308 | ECBool | Out shuttle use front rear sensor detect superfluous IC |  |
| F11 | Out shuttle use front rear sensor detect superfluous IC | — | `bF11_Active` | bool | — | 35308 | — | Out shuttle use front rear sensor detect superfluous IC | Sam 20240202 : 新增 F11 Lock by file 功能 |
| F11 | Out shuttle use front rear sensor detect superfluous IC | — | `bF11_Enable` | bool | — | 35308 | — | Out shuttle use front rear sensor detect superfluous IC | Sam 20240202 : 新增 F11 Lock by file 功能 |
| F12 | Rotate shuttle need check if the rotation is done. | `cbF12` | `bRotateShNeedCheck` | bool | `bRotateShNeedCheck` | 35309 | ECBool | Rotate shuttle need check if the rotation is done. | Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭 |
| F12 | Rotate shuttle need check if the rotation is done. | `EdF12` | `dRoShCheckDelayTime` | double | `dRoShCheckDelayTime` | 35309 | ECBool | Rotate shuttle need check if the rotation is done. | Steven 20110802 : 轉轉蝦頭要檢查有沒有轉頭的延遲時間 |
| F13 | Rotate shuttle speed | `edF13_Ini` | `iInitSpeed` | int | `RotateInitSpeed` | --- | --- | Rotate shuttle speed | kevin 20110531 旋轉SHUTTLE 鎖最高速度 |
| F13 | Rotate shuttle speed | `edF13_Jog` | `iPJogHighSpeed` | int | `RotateJogHighSpeed` | --- | --- | Rotate shuttle speed | kevin 20110531 旋轉SHUTTLE 鎖最高速度 |
| F13 | Rotate shuttle speed | `edF13_ADC` | `iRotateADC` | int | `Rotate ADC` | --- | --- | Rotate shuttle speed | Steven 20101018 : 轉轉蝦頭的加減速 |
| F14 | Knock shuttle when jam happen.  Interval(Sec.) : | `cbF14` | `bF14KnockShuttle` | bool | `bKnockShuttle` | 35314 | ECBool | Knock shuttle when jam happen.  Interval(Sec.) : | Steven 20120801 : Shuttle敲敲 |
| F14 | Knock shuttle when jam happen.  Interval(Sec.) : | `edF14` | `dF14KnockShuttleInterval` | double | `dKnockShuttleInterval` | 35314 | ECBool | Knock shuttle when jam happen.  Interval(Sec.) : | Steven 20120801 : Shuttle敲敲 |
| F14 | Knock shuttle when jam happen.  Interval(Sec.) : | `edF14_No` | `iF14KnockShuttleNo` | int | `iKnockShuttleNo` | 35314 | ECBool | Knock shuttle when jam happen.  Interval(Sec.) : | wei 20121206 |
| F14-1 |  | `cbF14_1` | `bF14_1KnockShuttleFirst` | bool | `bKnockShuttleFirst` | 35319 | ECBool | Knock shuttle first . | jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊 |
| F14-1 |  | `edF14_1` | `dF14KnockShuttleIntervalFirst` | double | `dF14KnockShuttleIntervalFirst` | 35319 | ECBool | Knock shuttle first . | jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊 |
| F14-1 |  | `edF14_1_No` | `iF14KnockShuttleNoFirst` | int | `iF14KnockShuttleNoFirst` | 35319 | ECBool | Knock shuttle first . | jou 2015-12-09 SCS 要求 Shuttle 每次入料前 敲擊 |
| F15 | Out shuttle lose IC need input password | `cbF15` | `bF15OutShuttleLoseICNeedPWD` | bool | `OutShuttleLoseICNeedPWD` | 35317 | ECBool | Out shuttle lose IC need input password | ChungHung 20120912 Amkor 需求Shuttle lose ic need password |
| F16 | Check input shuttle sensor I/O | `cbF16` | `bF16CheckShuttleSensorBroken` | bool | `CheckShuttleSensorBroken` | 35318 | ECBool | Check input shuttle sensor I/O | 2014-01-06    Dell    for TSMC 確認shuttle 有沒有斷線 |
| F17 | Always shuttle 1 first after one cycle in hot mode | `cbF17` | `bF17Sht1First` | bool | `bF17Sht1First` | 35322 | ECBool | Always shuttle 1 first after one cycle in hot mode | Steven 20160926 : One cycle後要先跑蝦頭一 |
| F18 | In shuttle product detect | `cbF18` | `bF18InshuttleDetect` | bool | `bInshuttleDetect` | 35323 | ECBool | In shuttle product detect | kevin 20141213 20140206 input SHUTTLE 第9顆sensor 進入偵測是否有ic |
| F19 | Out shuttle lose IC need do piggyback check | `cbF19` | `bF19OutShuttleLoseICNeedPiggyback` | bool | `bOutShuttleLoseICNeedPiggyback` | 35324 | ECBool | Out shuttle lose IC need do piggyback check | Steven 20150709 : Out shuttle lose IC要做Piggyback |
| F20 | In shuttle product prominent detect | `cbF20` | `bF20InShuttleProminentDetect` | bool | `bF20InShuttleProminentDetect` | 35325 | ECBool | In shuttle product prominent detect | Alick 20160815 add for 力成 Input Shuttle 檢測IC是否放到外圍，避免壓壞 |
| F21 | IN/OUT Arm Z motor on home sensor ,shuttle move | `cbF21` | `bF21InOutArmZMotorPrivate` | bool | `bF21InOutArmZMotorPrivate` | 35326 | ECBool | IN/OUT Arm Z motor on home sensor ,shuttle move | kevin 20161005 shuttle移動時判斷 In Out Arm Z軸在Home sensor |
| F22 | In shuttle check has device from Index Arm. | `cbF22` | `bF22InShuttleDetectOutNoIC` | bool | `bF22InshuttleDetectOutNoIC` | 35327 | ECBool | In shuttle check has device from Index Arm. | kevin 20161108 shuttle 出來撿測有無IC殘留 |
| F23 |  | `cbF23` | `bF23ShuttleVibration` | bool | `bF23ShuttleVibration` | 35328 | ECBool | Enable shuttle vibration                   (Unit : 0.1 Sec) | JerryYang 20171006 (wei) Shuttle 震動馬達 |
| F23 |  | `edF23` | `iF23ShuttleVibrationTime` | int | `iF23ShuttleVibrationTime` | 35328 | ECBool | Enable shuttle vibration                   (Unit : 0.1 Sec) | JerryYang 20171205 (Steven) shuttle震動馬達功能 |
| F23-1 |  | `edtF23_1` | `iF23ShuttleVibrationCount` | int | `iF23ShuttleVibrationCount` |  |  |  |  |
| F24 | Out shuttle lose IC must open index door and push Z1 | `cbF24` | `bF24OutShuttleLoseIcOpenIndexDoor` | bool | `OutShuttleLoseIcOpenIndexDoor` | 35330 | ECBool | Out shuttle lose IC must open index door and push Z1 | kevin 20180725 (wei) add out shuttle lose IC push Z1 open index door |
| F25 |  | `cbF25` | `bF25VibrateForOutShuttle` | bool | `bF25VibrateForOutShuttle` |  | ECBool | Vibration function for Output shuttle.                    (Unit : 0.1 Sec) |  |
| F25 |  | `edF25` | `iF25VibrateTime` | int | `iF25VibrateTime` |  | ECBool | Vibration function for Output shuttle.                    (Unit : 0.1 Sec) |  |
| F26 | Out shuttle Jam Select Skip or Retry | — | `bF26_Enable` | bool | — |  | — | Out shuttle Jam Select Skip or Retry | Sam 20220527 : for 矽格湖口 -- F26 Enable |
| F26 | Out shuttle Jam Select Skip or Retry | `rgF26` | `iF26OutShuttleJamSelectSkipOrRetry` | int | `iF25OutShuttleJamSelectSkipOrRetry` |  | ECInteger | Out shuttle Jam Select Skip or Retry | KaiChen 20181211 ：Out shuttle Jam Select Skip or Retry |
| F27 | Out shuttle lose IC, out arm need to pick again. | `chkF27` | `bF27OutShtLoseICNeedToPick` | bool | `bF27OutShtLoseICNeedToPick` |  | ECBool | Out shuttle lose IC, out arm need to pick again. | Steven 20220120 : 吳如春要求out shuttle lose IC時, out arm還是要下去吸料 |
| F28 | Index Check Shuttle pos for Sensor. | `cbF28` | `bF28IndexCheckShuttlePos` | bool | `bF26IndexCheckShuttlePos` |  | ECBool | Index Check Shuttle pos for Sensor. | kevin 20220512 add  Index check shuttle pos for Sensor |
| F29 | Always Vibration | `cbF29` | `bF29AlwaysVibrateOnShuttle` | bool | `bF26AlwaysVibrateOnShuttle` |  | ECBool | Always Vibration | Sam 20210602 : 每次都要強制震動                                       //Steven 20230308 : A26 --> A29 |
| F30 |  | `cbF30` | `bF30InShuttleSensorFollow16site` | bool | `bF26InShuttleSensorFollow16site` |  | ECBool | In shuttle floating sensor using new rule(use middle sensor) | JerryYang 20210426 : 2x4, 2x6 follow 2x8 shuttle sensor位置 |
| F31 | Enable Check Shuttle Motor Move Over Times To Jam | `cbF31` | `bF31_CheckShtMoveCnt` | bool | `bF31_CheckShtMoveCnt` |  | ECBool | Enable Check Shuttle Motor Move Over Times To Jam | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-1 | Shuttle 1 Motor Move Count Set | `edF31_1` | `iShtMoveCntSet` | int[] | `F31_1_iShuttleMotorMoveCountSet` |  | ECInteger | Shuttle 1 Motor Move Count Set | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-2 | Shuttle 2 Motor Move Count Set | `edF31_2` | `iShtMoveCntSet` | int[] | `F31_2_iShuttleMotorMoveCountSet` |  | ECInteger | Shuttle 2 Motor Move Count Set | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-3 | Shuttle 1 Motor Move Count Now | `edF31_3` | `iShtMoveCntNow` | int[] | `F31_3_iShuttleMotorMoveCountNow` |  | ECInteger | Shuttle 1 Motor Move Count Now | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-4 | Shuttle 2 Motor Move Count Now | `edF31_4` | `iShtMoveCntNow` | int[] | `F31_4_iShuttleMotorMoveCountNow` |  | ECInteger | Shuttle 2 Motor Move Count Now | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-5 |  | `edF31_5` | `iShtMoveCntHis` | int[] | `F31_5_iShuttleMotorMoveCountHistroy` |  |  |  | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F31-6 | Shuttle 1 Motor Move Count Histroy | `edF31_6` | `iShtMoveCntHis` | int[] | `F31_6_iShuttleMotorMoveCountHistroy` |  | ECInteger | Shuttle 1 Motor Move Count Histroy | Ztex 2023.04.19 Add HT-1032 TriTemp Function |
| F32 | Check in shuttle sensor by pass. Set value : | `cbF32` | `bF32CheckInSHSenBypass` | bool | `bF30CheckInSHSenBypass` |  | ECBool | Check in shuttle sensor by pass. Set value : | KenHsieh 20230829 : add Check In Shuttle Sensor By pass |
| F32 | Check in shuttle sensor by pass. Set value : | `edF32` | `iF32InSHSenBypassValue` | int | `iF30InSHSenBypassValue` |  | ECBool | Check in shuttle sensor by pass. Set value : | KenHsieh 20230829 : add Check In Shuttle Sensor By pass |
| F33 | Shuttle 2DID Mapping checking function. | `cbF33_Check2DHardware` | `bF33_Check2DHardware` | bool | `bF33_Check2DHardware` |  | ECBool | Enable Check shuttle cross Sensor | JerryYang 20250220 : 2DID硬體順序檢查功能 |
| F34 |  | `cbF34` | `bF34OutShtPickErrSetErrBin` | bool | `bF34OutShtPickErrSetErrBin` |  | ECBool | In shuttle offset no use for autoclean |  |
| F35 | Output shuttle lose IC, need trigger reset process. | `cbF35` | `bF35OutShtLoseNeedSetErrBin` | bool | `bF35OutShtLoseNeedSetErrBin` |  |  | Output shuttle lose IC, need trigger reset process. |  |
| F36 |  | `cbF36` | `bF36OutShtLoseICResetSetAllToErr` | bool | `bF36OutShtLoseICResetSetAllToErr` |  |  |  | JerryYang 20250120 : add |

---


## 未綁定 UI 元件的欄位

_無。所有 `F##` 開頭的欄位均已在主表中列出。_
