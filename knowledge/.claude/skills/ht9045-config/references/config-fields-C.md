# HT9045_CONFIG 欄位速查：群組 [C] Hardware（硬體選配）

> 來源：
> - `cConfiguration.dfm`（元件名稱與 Caption [Cxx]）
> - `cConfiguration.cpp`（`elConfig->Add` 元件 ↔ 變數綁定）
> - `Config.h`（變數型別與註解）
> - 版本：V3.33.903.0_20260417

> 欄位命名規則：`b` = bool、`i` = int、`d` = double、`as`/`s` = AnsiString

---

## 已綁定 UI 元件的欄位

共 50 個欄位，分屬 34 個區段。

| 區段 | Caption | UI 元件 | 變數名 | 型別 | INI Key | ECID | EC Type | Function Description | 程式註解 |
|------|---------|---------|--------|------|---------|------|---------|----------------------|----------|
| C01 | Fan Direction | `cbC01` | `bC01_FanDirection` | bool | `bFanDirection` |  | ECBool | Fan Direction | wei 20160215大風扇方向 |
| C02 | Enable CCD | — | `bC02InstallCCD` | bool | — | 35051 | — | Enable CCD |  |
| C03 | Use catch tray hardware | — | `bC03UseCatchTray` | bool | — | 35052 | — | Use catch tray hardware |  |
| C04 | Enable test temp  IC | `cbC04` | `bC04EnableTestTempIC` | bool | `bC04EnableTestTempIC` |  | ECBool | Enable test temp  IC |  |
| C05 | Use power saving mode | `cbC05_ATC` | `bC05_PowerSaveATC` | bool | `UsepowerSaveATC` | -- | -- | Use power saving mode | Ifor 20240401 : Power saving for ATC System |
| C05 | Use power saving mode | `cbC05_Motor` | `bC05_PowerSaveMotor` | bool | `UsepowerSaveMotor` | -- | -- | Use power saving mode | kevin 20110328 啟動馬達省電模式 |
| C05 | Use power saving mode | `cbC05_Temp` | `bC05_PowerSaveTemp` | bool | `UsepowerSaveTemp` | -- | -- | Use power saving mode | kevin 20110328 啟動溫度省電模式 |
| C05 | Use power saving mode | `cbC05_Vacuum` | `bC05_PowerSaveVacuum` | bool | `UsepowerSaveVacuum` | -- | -- | Use power saving mode | Steven 20221215 : Power saving for vacuum pump |
| C05 | Use power saving mode | `edC05_Vacuum` | `iC05HaltTime_Vacuum` | int | `HaltTime_Vacuum` | -- | -- | Use power saving mode | Steven 20221215 : Power saving for vacuum pump |
| C05 | Use power saving mode | `edC05_ATC` | `iHaltTime_ATC` | int | `HaltTime_ATC` | -- | -- | Use power saving mode | Ifor 20240401 : Power saving for ATC System |
| C05 | Use power saving mode | `edC05_Motor` | `iHaltTime_Motor` | int | `HaltTime_Motor` | -- | -- | Use power saving mode | kevin 20110328設定省電模式時間 |
| C05 | Use power saving mode | `edC05_Temp` | `iHaltTime_Temp` | int | `HaltTime_Temp` | -- | -- | Use power saving mode |  |
| C05 | Use power saving mode | `rgC05` | `iPowersaveMode` | int | `PowersaveMode` | -- | -- | Use power saving mode |  |
| C06-01 |  | `cbC06_01` | `bC06_ByPassIonFan` | bool[] | `Fan01` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-02 |  | `cbC06_02` | `bC06_ByPassIonFan` | bool[] | `Fan02` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-03 |  | `cbC06_03` | `bC06_ByPassIonFan` | bool[] | `Fan03` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-04 |  | `cbC06_04` | `bC06_ByPassIonFan` | bool[] | `Fan04` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-05 |  | `cbC06_05` | `bC06_ByPassIonFan` | bool[] | `Fan05` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-06 |  | `cbC06_06` | `bC06_ByPassIonFan` | bool[] | `Fan06` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-07 |  | `cbC06_07` | `bC06_ByPassIonFan` | bool[] | `Fan07` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-08 |  | `cbC06_08` | `bC06_ByPassIonFan` | bool[] | `Fan08` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-09 |  | `cbC06_09` | `bC06_ByPassIonFan` | bool[] | `Fan09` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-10 |  | `cbC06_10` | `bC06_ByPassIonFan` | bool[] | `Fan10` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-11 |  | `cbC06_11` | `bC06_ByPassIonFan` | bool[] | `Fan11` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C06-12 |  | `cbC06_12` | `bC06_ByPassIonFan` | bool[] | `Fan12` |  |  |  | Steven 20111013 : 可以不檢查離子風扇 |
| C08 | Use socket sensor | `cbC08` | `bC08_SocketSensor` | bool | `bUseSocketSensor` | 35072 | ECBool | Use socket sensor | kevin 20130504 使用socket sensor detect 功能 piggy back |
| C08-1 | Every Start Check Function is ON | `cbC08_1` | `bC08_1_CheckSocketSensorDetectON` | bool | `bCheckSocketSensorDetectON` |  | ECBool | Every Start Check Function is ON | Isaac 20201130 : Socket Sensor按Start後要偵測是否啟動功能 |
| C09 | Active car recorder after jam happened | `cbC09` | `bC09_CarRecord` | bool | `bCarRecord` | 35073 | ECBool | Active car recorder after jam happened | wei 2013-12-09 |
| C09 | Active car recorder after jam happened | `edC09` | `iCarRecordDelayTime` | int | `iCarRecordDelayTime` | 35073 | ECBool | Active car recorder after jam happened | wei 2013-12-09 |
| C10 | Enable ESD connect error report function | `cbC10` | `bEnable_ESD_COMERR_Report` | bool | `bEnable_ESD_COMERR_Report` | 35075 | ECBool | Enable ESD connect error report function | Ifor 20150724 :Enable ESD COM ERR Report Function |
| C11 | Use Monitor Video(For TCP/IP) | `cbC11` | `bC11UseMonitorView` | bool | `bC11UseMonitorView` |  | ECBool | Use Monitor Video(For TCP/IP) | JerryYang 20160621 錄影監視功能 |
| C12 | Use PE Mode | `cbC12` | `bC12UsePEMode` | bool | `bC12UsePEMode` |  | ECBool | Use PE Mode | Ifor 20170125 : add PE Mode |
| C13 | Need to restart GroundMan  when initial start | `cbC13` | `bC13NeedToRestartGroundWhenInitialStart` | bool | `bC13NeedToRestartGroundWhenInitialStart` |  | ECBool | Need to restart GroundMan  when initial start | Sam 20220107 : 矽格北興 Initail Start 要重啟 GroundMan |
| C14 | Save communication log of bin display | `chkC14` | `bC14SaveBinDisplayLog` | bool | `bC14SaveBinDisplayLog` |  | ECBool | Save communication log of bin display | Steven 20220309 : BinDisplay Log |
| C16 | Use barcoder reader change setup file | `cbC16` | `bC16UseBarCoderChangeSetupFile` | bool | `bC16UseBarCoderChangeSetupFile` |  | ECBool | Use barcoder reader change setup file | Sam 20230320 : 使用 BarCodeReader 來輸入切換 SetupFile。 |
| C17 | Use Loader Color Sensor | `cbC17` | `bC17UseLoaderColorSensor` | bool | `bC20UseLoaderColorSensor` |  | ECBool | Use Loader Color Sensor | Jimmychiu 20230630 : add color sensor MU-N in Loader |
| C17 | Use Loader Color Sensor | `edtC17` | `iC17DelayTimes` | int | `iC20DelayTimes` |  | ECBool | Use Loader Color Sensor | Jimmychiu 20230630 : add color sensor MU-N in Loader |
| C17-1 | Skip Alarm | `cbC17_1` | `bC17_1_SkipAlarm` | bool | `bC20_1_SkipAlarm` |  |  | Skip Alarm | Jimmychiu 20230630 : add color sensor MU-N in Loader |
| C20-1 |  | — | `bC20_1EnableEnergySavingDryAir` | bool | — |  | — |  |  |
| C20-2 |  | — | `iC20_2LowTempOffEnergySaving` | int | — |  | — |  | Add check Low Temperature Not Use EnergySaving |
| C20-3 |  | — | `iC20_3TempOverUseEnergySaving` | int | — |  | — |  | Add Ower  Temperature Open  EnergySaving |
| C20-4 |  | — | `bC20_4IndexUseForstSensor` | bool | — |  | — |  | add check DewPointMeter |
| C21 | Vibration motor speed | `edtC21_Auto1` | `iC21MotSp_AUTO1` | int[] | `C16VibrateMotSp_AUTO1` | -- | -- | Vibration motor speed |  |
| C21 | Vibration motor speed | `edtC21_Auto2` | `iC21MotSp_AUTO2` | int[] | `C16VibrateMotSp_AUTO2` | -- | -- | Vibration motor speed |  |
| C21 | Vibration motor speed | `edtC21_Auto3` | `iC21MotSp_AUTO3` | int[] | `C16VibrateMotSp_AUTO3` | -- | -- | Vibration motor speed |  |
| C21 | Vibration motor speed | `edtC21_HP1` | `iC21MotSp_HP1` | int[] | `C16VibrateMotSp_HP1` | -- | -- | Vibration motor speed | JerryYang 20230814 : add震動馬達通訊調速版本 |
| C21 | Vibration motor speed | `edtC21_HP2` | `iC21MotSp_HP2` | int[] | `C16VibrateMotSp_HP2` | -- | -- | Vibration motor speed |  |
| C21 | Vibration motor speed | `edtC21_SHT1` | `iC21MotSp_SHT1` | int[] | `C16VibrateMotSp_SHT1` | -- | -- | Vibration motor speed |  |
| C21 | Vibration motor speed | `edtC21_SHT2` | `iC21MotSp_SHT2` | int[] | `C16VibrateMotSp_SHT2` | -- | -- | Vibration motor speed |  |
| C24 | Initial start check  cylinder | `cbC24` | `bC24InitialStartCheckCylinder` | bool | `bC24InitialStartCheckCylinder` |  |  |  | JerryYang 20250120 : add |

---


## 未綁定 UI 元件的欄位

_無。所有 `C##` 開頭的欄位均已在主表中列出。_
