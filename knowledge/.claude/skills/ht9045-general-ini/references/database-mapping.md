# database.cpp — ReadGeneralIni() INI 對應表

> 原始檔：`HT9045/HT9011UC_Code_V3.33.898.0_20260313_Steven/database.cpp`
> `SYSTEM_MODULAR::ReadGeneralIni()` 約 lines 302–1511

## 目錄

- [INI 檔來源](#ini-檔來源)
- [Version Section](#version-section)
- [System Section](#system-section)
- [TempCtrl Section](#tempctrl-section)
- [TrayZ Section](#trayz-section)
- [TrayCassette Section](#traycassette-section)
- [TrayY Section](#trayy-section)
- [IndexDriver Section](#indexdriver-section)
- [Index Section](#index-section)
- [MotorDriver Section](#motordriver-section)
- [2D_BarCode Section](#2d_barcode-section)
- [OCR Section](#ocr-section)
- [ATC Section](#atc-section)
- [NUMBER_PANEL Section](#number_panel-section)
- [RealTimeCCD Section](#realtimeccd-section)
- [FinePitch Section](#finepitch-section)
- [ROTATE_KIT Section](#rotate_kit-section)
- [OutSortArm Section](#outsortarm-section)
- [AIR_CON Section](#air_con-section)
- [Laser Section](#laser-section)
- [Vibration Section](#vibration-section)
- [Ground_Man Section](#ground_man-section)
- [RFID Section](#rfid-section)
- [Fix_AI_CCD Section](#fix_ai_ccd-section)
- [Tray_Mapping Section](#tray_mapping-section)
- [Auto_Alignment Section](#auto_alignment-section)
- [AutoTeach Section](#autoteach-section)
- [database 獨有 Key](#database-獨有-key)
- [特殊邏輯](#特殊邏輯)

---

## INI 檔來源

- **主要**：`D:\HT9045\system\Gerneral.ini`（透過 `OpenGeneralIniFile()` 取得）
- **GPIB**：`D:\GPIB9045\system\general.ini`（讀取 Model 判斷機型）

---

## Version Section

| Key | 全域變數 | 預設值 | 特殊邏輯 |
|-----|----------|--------|----------|
| Model | MachName | "ModelNG" | 驗證機型（9045/9046/502/1032/7080），無效則 bHandlerModel=false 並 return |
| SubModel | SubMachineType | Type_None | 依機型條件讀取 |
| Ver | sVer | "" | ASE_KaohSiung 客戶有條件回寫 |

## System Section

| Key | 全域變數 | 預設值 | 特殊邏輯 |
|-----|----------|--------|----------|
| CUSTOMER_CODE | CUSTOMER_CODE | 0 | 觸發 CustomerFunctionSelect()，影響大量預設值 |
| AUTO_EMPTY_COLOR | AUTO_EMPTY_COLOR | (條件) | 若 INI 中不存在，彈 Dialog 詢問使用者 |
| SUPPORT_2_EMPTY_EMPTY | SUPPORT_2_EMPTY_EMPTY | 0 | — |
| EP_Install | EP_Install | 0 | — |
| INOUT_ARM_PICKER_USE_MOTOR | InOutArmPickerUseMotor | 1 | — |
| bUseAuto2Empty | bUseAuto2Empty | 0 | — |
| USE_AUTO_RETEST | USE_AUTO_RETEST | eartUninstall | — |
| UNLOADER_AUTO1_ART | UNLOADER_ART[eAuto1] | eartInstall | 條件：USE_AUTO_RETEST==eartInstall |
| UNLOADER_AUTO2_ART | UNLOADER_ART[eAuto2] | eartInstall | 條件：USE_AUTO_RETEST==eartInstall |
| UNLOADER_AUTO3_ART | UNLOADER_ART[eAuto3] | eartInstall | 條件：USE_AUTO_RETEST==eartInstall |
| UNLOADER_AUTO4_ART | UNLOADER_ART[eAuto4] | eartInstall | 條件：AUTO_EMPTY_COLOR>=3 |
| UNLOADER_AUTO5_ART | UNLOADER_ART[eAuto5] | eartInstall | 條件：AUTO_EMPTY_COLOR>=3 |
| UNLOADER_AUTO6_ART | UNLOADER_ART[eAuto6] | eartInstall | 條件：AUTO_EMPTY_COLOR>=4 |
| bAutoTrackCanGoRear | bAutoTrackCanGoRear | 0 | — |
| bAutoZNoUseART | bNoAutoZSelect | 0 | — |
| WEIGHT_CALIBRATION | WEIGHT_CALIBRATION | 0 | — |
| SHUTTLE_SENSOR_TYPE | SHUTTLE_SENSOR_TYPE | 0 | — |
| NUEC_TYPE | NUEC_TYPE | 0 | — |
| ENABLE_OUT_SHUTTLE_SENEOR | ENABLE_OUT_SHUTTLE_SENEOR | true | HT9046/HT1032 條件覆寫為 false |
| ENABLE_OUT_SHUTTLEY_LATCH | ENABLE_OUT_SHUTTLEY_LATCH | false | HT9046_LS/HT1032 條件覆寫為 true |
| Use_AxisY_Sensor_2x3mode | Use_AxisY_Sensor_2x3mode | false | — |
| Bias_Mode_Use_Y_Sensor | Bias_Mode_Use_Y_Sensor | false | — |
| SAFE_DOOR_AMOUNT | SAFE_DOOR_AMOUNT | 2 | — |
| NUMBER_PANEL_TYPE | NUMBER_PANEL_TYPE | 2 | — |
| INDEX_SUCKER_TYPE | INDEX_SUCKER_TYPE | 0 | — |
| INDEX_PRESS_TYPE | INDEX_PRESS_TYPE | 0 | cast to eIndexPressType |
| TRAY_VIBRATION | TRAY_VIBRATION | 0 | — |
| TRAY_ARM_MODE | TRAY_ARM_MODE | 0 | — |
| LOADER_VIBRATION | USE_LOADER_VIBRATION | false | — |
| USE_TRAY_ROBOT | USE_TRAY_ROBOT | 0 | — |
| USE_LOADER_HINGE | USE_LOADER_HINGE | 0 | Range: 0-1 |
| USE_DIE_CLEAN | USE_DIE_CLEAN | 0 | Range: 0-1 |
| USE_CKD_FCM_CleanAir | USE_CKD_FCM_CleanAir | false | — |
| USE_CATCH_TRAY_MODEL | USE_CATCH_TRAY_MODEL | 2 (if ART) / 0 | 條件依 USE_AUTO_RETEST |
| UseCatchTrayBlock | bNewCatchTrayblock | 1 (ASE) / 0 | 客戶相依 |
| USE_16_HEATER | USE_16_HEATER | eht4Heater | 連動 EJ1N_Count (4/8/0) |
| REAL_TIME_CCD | REAL_TIME_CCD | false | — |
| RTC_TemperNumber | RTC_TemperNumber | 1 | — |
| CCD2_TEMPER | CCD2_TEMPER | false | — |
| LB_TEMP | LB_TEMP | false | — |
| LB_TEMP_UpDown | LB_TEMP_UpDown | false | — |
| Index_ESDAir | Index_ESDAir | false | — |
| INSTALL_OCR | INSTALL_OCR | eocrUninstal | — |
| INSTALL_OCR_YMot | INSTALL_OCR_YMot | eocrYMotUninstal | — |
| SAFE_DOOR_LOCK | SAFE_DOOR_LOCK | false | — |
| IN_SHT_LAST_SENSOR | IN_SHT_LAST_SENSOR | 0 | — |
| HOT_PLATE_POSITION | HOT_PLATE_POSITION | 0 | — |
| HOT_PLATE_LIMITATION | HOT_PLATE_LIMITATION | 0 | — |
| USE_LASER_DISTANCE | USE_LASER_DISTANCE | 0 | — |
| USE_DEVICE_FLIPPER | USE_DEVICE_FLIPPER | 0 | — |
| MAGAZINE_BIN_DISP_TYPE | MAGAZINE_BIN_DISP_TYPE | 0 | — |
| MACHINE_HAS_AUTO_ALIGNMENT_CCD | MACHINE_HAS_AUTO_ALIGNMENT_CCD | 0 | — |
| USE_IN_OUT_ARM_Y_PITCH | USE_IN_OUT_ARM_Y_PITCH | iXPitch60 | — |
| USE_OUT_ARM_Y_PITCH | USE_OUT_ARM_Y_PITCH | =USE_IN_OUT_ARM_Y_PITCH | — |
| IN_OUT_ARM_Y_PITCH_MIN | IN_OUT_ARM_Y_PITCH_MIN | 1500 | 複雜 range 驗證 |
| IN_OUT_ARM_Y_PITCH_MAX | IN_OUT_ARM_Y_PITCH_MAX | 7500 | 複雜 range 驗證 |
| USE_IN_OUT_ARM_X_PITCH | USE_IN_OUT_ARM_X_PITCH | iXPitch40mm | — |
| IN_OUT_ARM_X_PITCH_MIN | IN_OUT_ARM_X_PITCH_MIN | 4000+ | 依 X Pitch 類型 |
| IN_OUT_ARM_X_PITCH_MAX | IN_OUT_ARM_X_PITCH_MAX | 12000+ | 依 X Pitch 類型 |
| BASE_X_TO_HP | BASE_X_TO_HP | 6800 | 條件 clamping |
| INSTALL_HEAT_GUN | INSTALL_HEAT_GUN | 0 | — |
| INSTALL_ATC_HEAT_GUN | INSTALL_ATC_HEAT_GUN | 0 | — |
| FIX3_FULL_PLACE | FIX3_FULL_PLACE | 0 | Range: 0-5 |
| USE_MAGNETIC_SCALE | USE_MAGNETIC_SCALE | 0 | — |
| USE_HOTPLATE_TYPE | USE_HOTPLATE_TYPE | 0 | — |
| MOTION_CARD_TYPE | MOTION_CARD_TYPE | 0 | — |
| MOTIONNET_SPEED | MOTIONNET_SPEED | COMMSPEED_20M | — |
| IO_CARD_TYPE | IO_CARD_TYPE | 0 | 觸發 LoadIoData() |
| TTL_CARD_TYPE | TTL_CARD_TYPE | 0 | ==2 時設定 TTLRS232VerCheck |
| TTL_CARD_USE_ADDRESS | TTL_CARD_USE_ADDRESS | 0 | — |
| INSTALL_SOCKET_CLAMP | INSTALL_SOCKET_CLAMP | 0 | — |
| INSTALL_DOUBLE_EP | INSTALL_DOUBLE_EP | 0 | — |
| USE_PRECISER | USE_PRECISER | 0 | — |
| iPreciserInstallArea | iPreciserInstallArea | 0 | — |
| USE_COLOR_TRAY_SENSOR | USE_COLOR_TRAY_SENSOR | 0 | — |
| USE_SOCKET_SENSOR | USE_SOCKET_SENSOR | 999 | — |
| Canbus_Method | CANBUS_METHOD | 0 | — |
| USE_RFID_READER | USE_RFID_READER | 0 | — |
| USE_RFID_SYSTEM | USE_RFID_SYSTEM | 0 | Range: 0-1 |
| USE_MR_SYSTEM | USE_MR_SYSTEM | 0 | Range: 0-2 |
| SHUTTLE_Z_TYPE | SHUTTLE_Z_TYPE | 0 | ASE+9046_LS 條件覆寫 |
| USE_ESD_Monior | ESD_Monitor | 0 | Range: 0-1 |
| USE_NOVX3360 | USE_NOVX3360 | 0 | Range: 0-1 |
| USE_AutoCleanIonFan | USE_AutoCleanIonFan | 0 | Range: 0-1 |
| USE_KASUGA | USE_KASUGA | 0 | Range: 0-1 |
| USE_KASUGA_Fan | USE_KASUGA_Fan | 0 | Range: 0-1 |
| USE_OTD | USE_OTD | 0 | Range: 0-2 |
| USE_PULSE_TYPE | USE_PULSE_TYPE | 0 | Range: 0-1 |
| ION_PULSE_COUNT | ION_PULSE_COUNT | 3000 | Range: 3000-100000 |
| HTIonBarFunction | iUseHTIonBarFunction | 0 | Range: 0-3 |
| CHAMBER_USE_PULSE_TYPE | CHAMBER_USE_PULSE_TYPE | 0 | Range: 0-1 |
| I24V_PULSE_COUNT | i24V_PULSE_COUNT | 500 | Range: 500-1000 |
| SocketBasedAdd4Temp | iSocketBaseTempCount | 0 | HT9046_LS 條件覆寫 |
| USE_46_SUCKER_DB | USE_46_SUCKER_DB | 0 | HT9046/1032 強制 0 + 回寫 |
| USE_46_SENSOR_DB | USE_46_SENSOR_DB | 0 | HT9046/1032 強制 0 + 回寫 |
| INDEX_MOTION_CARD | INDEX_MOTION_CARD | 0 | — |
| USE_FINE_PITCH | USE_FINE_PITCH | 0 | — |
| USE_OUT_SHT_MOT | USE_OUT_SHT_MOT | 0 | — |
| ControlPanelMode | iControlPanelMode | 0 | — |
| VacuUnitType | VCCU_UNIT_TYPE | 0 | — |
| AOI | USE_AOI_Inspection | 0 | Range: 0-1 |
| Scanner_AOI | USE_Scanner_AOI_Inspection | 0 | Range: 0-1 |
| Top_Scanner_AOI | USE_Top_Scanner_AOI_Inspection | 0 | Range: 0-1 |
| Fix_AI_CCD | USE_Fix_AI_CCD | 0 | Range: 0-1 |
| USE_TRAY_MAPPING | USE_TRAY_MAPPING | etmUninstall | Range: etmUninstall-etmDeviceRemain |
| USE_COLORSENSOR_MUN | USE_COLORSENSOR_MUN | eCSMUN_Uninstall | — |
| USE_AUTO_ALIGNMENT | USE_AUTO_ALIGNMENT | 0 | — |
| SocketSenAmpQty | SOCKET_AMP_QTY | 4-8 | 條件依 Socket/Color/Customer |
| RotateSenAmpQty | ROTATE_AMP_QTY | 4 (if RotateKit) / 0 | Range: 0-4 |
| ColorSenAmpQty | COLOR_AMP_QTY | 6 | Range: 0-8 |
| SocketSenAmpQty2nd | SOCKET_AMP_QTY_2nd | 0 | Range: 0-16 |
| VibrationCardQty | VibrationMotorCount | 2 | Range: 0-2 |
| CanBusNudn1Qty | NUDN1_QTY | 2 | Range: 0-4 |
| Nudn1Macid11Qty | NUDN1_MACID11_AMP_QTY | 16 | Range: 0-16 |
| Nudn1Macid12Qty | NUDN1_MACID12_AMP_QTY | 4 | Range: 0-16 |
| Nudn1Macid13Qty | NUDN1_MACID13_AMP_QTY | 0 | Range: 0-16 |
| Nudn1Macid14Qty | NUDN1_MACID14_AMP_QTY | 0 | Range: 0-16 |
| USE_OHT_SYSTEM | USE_OHT_SYSTEM | 0 | Range: 0-1 |
| USE_Multile_Empty | USE_Multile_Empty | 0 | Range: 0-1 |
| USE_KEYENCE_LOADER | USE_KEYENCE_LOADER | 0 | Range: 0-1 |
| USE_KEYENCE_EMPTY | USE_KEYENCE_EMPTY | 0 | Range: 0-2 |
| USE_MultileEmptyTrayID_Keyence | USE_MultileEmptyTrayID_Keyence | 0 | Range: 0-1 |
| TRAY_MAPPING_GRAB | TRAY_MAPPING_GRAB | 0 | — |
| HighTemperatureSet175 | iTemp175 | 0 | 溫度限制條件邏輯 |
| HighTemperatureSet155 | iTemp155 | 0 | 溫度限制條件邏輯 |
| HighTemperatureSet150 | iTemp150 | 0 | 溫度限制條件邏輯 |
| HighTempLimit | iTempLimitation | tTemp130 | 條件依 iTemp 旗標 |
| DewPoint_Hardware_Install | DewPoint_Hardware_Install | 0 | — |
| HotGunFlowEnable | HotGunFlowEnable | 0 | — |
| HotGunFlow_LineNo | HotGunFlow_LineNo | 0 | — |
| HotGunFlow_DevNo | HotGunFlow_DevNo | 0 | — |
| HotGunFlow_Gun1_ChannelNo | HotGunFlow_Gun1_ChannelNo | 0 | — |
| HotGunFlow_Gun2_ChannelNo | HotGunFlow_Gun2_ChannelNo | 0 | — |
| AUTO3_IS_MAGAZINE | AUTO3_IS_MAGAZINE | 0 | Range: 0-1 |
| BASE_HEATER_COUNT | BASE_HEATER | 0 | — |
| SHUTTLE_FLOODGATE | SHUTTLE_FLOODGATE | 0 | — |
| Tri_Temp_Machine | Tri_Temp_Machine | 0 | ==1 時清除所有 AMBIENT_TEMP_CHECK |
| AirStream_Select | AirStream_Select | 0 | — |
| TriTemperature_TotalChannel | TriTemperature_TotalChannel | 3 | — |
| Tri_Temperature_MaxDegree | Tri_Temperature_MaxDegree | 175 | — |
| Tri_Temperature_MinDegree | Tri_Temperature_MinDegree | -55 | — |
| SetHeaterTemp_MaxOutSht | SetHeaterTemp_MaxOutSht | 60 | — |
| SetHeaterTemp_MaxIndex | SetHeaterTemp_MaxIndex | 60 | — |
| SetHeaterTemp_MaxBase | SetHeaterTemp_MaxBase | 60 | — |
| Total_Compressor | Total_Compressor | 0 | — |
| IndexDoorHeater | INDEXDOORHEATER | 0 | — |
| SafePlcIO | Enable_PLCSafety_IO | 0 | — |
| DOUBLE_BELT_MODE | DOUBLE_BELT_MODE | 0 | — |
| USE_COVER_TRAYID | USE_COVER_TRAYID | tCIDNotUse | — |
| In_Shuttle_Auto_Latch | In_Shuttle_Auto_Latch | 0 | Range: 0-1 |
| USE_LD_Rot_Arm | USE_LD_Rot_Arm | 0 | — |
| AGVModal | USE_E84_Sensor | 0 | — |
| USE_LdUldCassetteMode | USE_LdUldCassetteMode | 0 | — |
| OFFLINE_ALARM | OFFLINE_ALARM | 1 | — |
| CHECK_EP_SETTING | CHECK_EP_SETTING | 1 | — |
| USE_InPlacement | USE_InPlacement | eartUninstall | — |
| T_MODE_SPEED | T_MODE_SPEED | 0.9 | Range: 0.5-1.0 |
| FixTrayDataCleanTime | dFixTrayDataCleanTime | 1.5 | Range: 0.5-3.0 |
| bHasEnteredPEModel | bHasEnteredPEModel | false | — |
| bHT9045S_USE2x4 | bHT9045S_USE2x4 | 0 | Range: 0-1 |
| bBarCodeRules | bEnable_KLT_Function | 0 | Range: 0-1 |
| OTDRecord | bOTDRecord | false | 條件回寫 |
| USE_BARCODE_AS_KEYBOARD | USE_BARCODE_AS_KEYBOARD | 0 | CC_KYEC_LEE 條件覆寫 |
| USE_ARM_PROTECTION | USE_ARM_PROTECTION | true | ASE/AMD 客戶預設 false |
| iDBQueryDays | iDBQueryDays | 3 | — |
| iMagazineCheckZPos | iMagazineCheckZPos | 400 | Range: 100-1000 |
| JCET_FOR_EVAN | JCET_FOR_EVAN | (條件) | ifdef FOR_EVAN 強制 1 |
| iAutoFormSize | iAutoFormSize | 0 | — |
| AOA_InArm_Loader_X/Y | iAOA_InArm_Loader_X/Y | 0 | — |
| AOA_InArm_Shuttle1/2_X/Y | iAOA_InArm_Shuttle1/2_X/Y | 0 | — |
| AOA_InArm_Hotplate1/2_X/Y | iAOA_InArm_Hotplate1/2_X/Y | 0 | — |
| AOA_OutArm_Auto1-6_X/Y | iAOA_OutArm_Auto1-6_X/Y | 0 | Auto4-6 條件讀取 |
| AOA_OutArm_Fix1-6_X/Y | iAOA_OutArm_Fix1-6_X/Y | 0 | Fix4-6 條件讀取 |
| AOA_OutArm_Shuttle1/2_X/Y | iAOA_OutArm_Shuttle1/2_X/Y | 0 | — |
| iXpitchMin/Max 系列 | iXpitchMin, iXpitchMax... | (計算值) | 依 Pitch 類型計算 |

## TempCtrl Section

| Key | 全域變數 | 預設值 | 特殊邏輯 |
|-----|----------|--------|----------|
| HEATER_CTRL_TYPE | TC401HeaterControl | KT4H | — |
| COM_PORT | sTempComPort | "COM2" | auto-prepend "COM" |
| COM_PORT_OMRON | sTempOmronComPort | "COM7" | auto-prepend "COM" |
| COM_PORT_DYNAMIC | sTempDynamicComPort | "COM6" | auto-prepend "COM" |
| SHUTTLE_COOLING | SHUTTLE_COOLING | 0.0 | Range: 0.0-10.0 |
| VORTEX_COOLING | VORTEX_COOLING | 0.5 | Range: 0.0-10.0 |
| SOCKET_OFFSET | SOCKET_OFFSET | 10.0 | Range: 0.0-30.0 |
| Socket | bUseSocketTemp | 0 | 第九軸加熱 |
| TEMPCTRL_NEED_UNDER_20A | TEMPCTRL_NEED_UNDER_20A | false | HotPlate 分段控制 |
| UseHotGunCheck | bUseHotGunCheck | false | Hot Gun 流量偵測 |
| UseHotGunFlowCheck | bUseHotGunFlowCheck | false | Hot Gun Flow |
| TEMPCTRL_HOTPLATE_TOGTHER | TEMPCTRL_HOTPLATE_TOGTHER | (依機型) | 9045=false, 9046/502/1032=讀取 |
| bTEMPCTRL_Shuttle_TOGTHER | bTEMPCTRL_Shuttle_TOGTHER | true | 條件回寫 |
| AMBIENT_TEMP_CHECK01–71 | AMBIENT_TEMP_CHECK[0..tcTotalCount-1] | false(i<2)/true(i>=2) | Loop 讀取；Tri_Temp==1 時清除 |

## TrayZ / TrayCassette / TrayY Section

（與 HandlerSys 對應相同 Key，讀取至全域變數陣列 `LOAD_Z_USE_MOTOR[]`、`LOADUNLOAD_USE_CASSETTE[]`、`LoaderUnload_StepMotor` 等）

## IndexDriver Section

| Key | 全域變數 | 預設值 |
|-----|----------|--------|
| COM_PORT | sTorqueComPort | "COM1" (auto-prepend "COM") |
| USE_IO_CHANGE_TOQUE | USE_IO_CHANGE_TOQUE | 0 |
| USE_ReadIndex_TOQUE | USE_ReadIndex_TOQUE | 0 |
| USE_INDEX_ARM_AXES | USE_INDEX_ARM_AXES | 0 |
| CLEAN_AIR | CLEAN_AIR | false |

## Index Section

（與 HandlerSys 對應相同 Key）

## MotorDriver Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| Type | MOTOR_DRIVER_TYPE | Panasonic_DRIVER | **database 獨有** — 馬達廠牌選擇 |

## 2D_BarCode Section

（與 HandlerSys 對應之基本 Key 相同，另有以下 database 獨有 Key）

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| Use_CCDShuttle_1A-2B_IP | asCCDBarCodeIP[0-3] | "172.16.8.210"-"213" | **database 獨有** — CCD IP（條件讀取） |
| Use_CCDShuttle_1A-2B_Port | asCCDBarCodePort[0-3] | "5001" | **database 獨有** — CCD Port（條件讀取） |
| BaudRate | BarcodeBaudRate | 9600 | **database 獨有** |
| InBaudRate | InBarcodeBaudRate | 9600 | **database 獨有** |
| ByteSize | BarcodeByteSize | 8 | **database 獨有** |
| StopBit | BarcodeStopBit | 1 | **database 獨有** |
| Parity | BarcodeParity | "None" | **database 獨有** |

## ATC Section

（與 HandlerSys 對應相同 Key，另有以下 database 獨有 Key）

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| USE_ATC_SELFTEST | bUseATC_SelfTestFunction | 0 | **database 獨有** — ATC 自我測試 |

## NUMBER_PANEL Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| NUMBER_PANEL_DELAY | dNumberPanelDelay | 1.0 | **database 獨有** — 顯示器輪巡時間 |
| COM_PORT | sNumberPanelComPort | "COM4" | auto-prepend "COM" |

## FinePitch Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| COM_PORT | sFinePitchComPort | "COM3" | **database 獨有** |
| COM_PORT_Adjustment | sFinePitchAdjustmentComPort | "COM3" | **database 獨有** |

## ROTATE_KIT Section

（與 HandlerSys 對應相同 Key，另有以下 database 獨有 Key）

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| SpecialSequence | iSpecialSequence | 0 | **database 獨有** — Range: 0-1 |

## Fix_AI_CCD Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| Fix_BGA_AI_CCD_IP | asFix2BGAAICCDIP[0] | "172.16.8.210" | **database 獨有** |
| Fix_BGA_AI_CCD_IP2 | asFix2BGAAICCDIP[1] | "172.16.8.210" | **database 獨有** |
| Fix_BGA_AI_CCD_Port | asFix2BGAAICCDPort[0] | "8000" | **database 獨有** |
| Fix_BGA_AI_CCD_Port2 | asFix2BGAAICCDPort[1] | "8001" | **database 獨有** |
| Vision_Light_PORT | asVisionLightPort | "COM8" | **database 獨有** |

## Tray_Mapping Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| Use_Tray_ID-Map_IP (多個) | asCCDTrayIP[0-11] | "172.16.8.200"-"154" | **database 獨有** — 含 CoverTray |
| Use_Tray_ID-Map_Port (多個) | asCCDTrayPort[0-11] | "5101"-"23" | **database 獨有** |

## Auto_Alignment Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| Use_AutoAlign_InTop-OutBottom_IP | asCCDAlignIP[0-3] | "172.16.110.201" | **database 獨有** |
| Use_AutoAlign_InTop-OutBottom_Port | asCCDAlignPort[0-3] | "5110"-"5113" | **database 獨有** |

## AutoTeach Section

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| iInArmAutoYTeachOffset | iInArmAutoYTeachOffset | 0 | **database 獨有** |
| iOutArmAutoYTeachOffset | iOutArmAutoYTeachOffset | 0 | **database 獨有** |
| iInArmTeachZ | iInArmTeachZ | -1500 | **database 獨有** |
| ioutArmTeachZ | ioutArmTeachZ | -1500 | **database 獨有** |

## RFID Section

（與 HandlerSys 基本 Key 相同，另有以下 database 獨有 Key）

| Key | 全域變數 | 預設值 | 說明 |
|-----|----------|--------|------|
| RFID1_COM_PORT | asRFIDComPort[0] | "COM15" | **database 獨有** — MR 系統用 |
| RFID2_COM_PORT | asRFIDComPort[1] | "COM16" | **database 獨有** |
| BaudRate | RFIDBaudRate / iRFIDBaudRate | 9600 | **database 獨有** |
| ByteSize | RFIDByteSize / iRFIDByteSize | 8 | **database 獨有** |
| StopBit | RFIDStopBit / iRFIDStopBit | 1 | **database 獨有** |
| Parity | RFIDParity / sRFIDParity | "None" | **database 獨有** |

---

## database 獨有 Key

以下 Key **僅在 database.cpp 中讀取**，HandlerSys 與 main 均不處理：

| Section | Key | 說明 |
|---------|-----|------|
| MotorDriver | Type | 馬達廠牌選擇 |
| TempCtrl | SHUTTLE_COOLING / VORTEX_COOLING / SOCKET_OFFSET | 溫度 Offset 值 |
| TempCtrl | Socket | 第九軸加熱 |
| TempCtrl | TEMPCTRL_NEED_UNDER_20A | HotPlate 分段 |
| TempCtrl | UseHotGunCheck / UseHotGunFlowCheck | Hot Gun 檢查 |
| TempCtrl | TEMPCTRL_HOTPLATE_TOGTHER / bTEMPCTRL_Shuttle_TOGTHER | 分段加熱控制 |
| TempCtrl | AMBIENT_TEMP_CHECK01–71 | 71 個常溫檢查位置 |
| NUMBER_PANEL | NUMBER_PANEL_DELAY | 輪巡時間 |
| FinePitch | COM_PORT / COM_PORT_Adjustment | Fine Pitch 通訊 |
| 2D_BarCode | CCD IP/Port, BaudRate, ByteSize, StopBit, Parity | 通訊參數 |
| RFID | BaudRate, ByteSize, StopBit, Parity, RFID1/2_COM_PORT | 通訊參數 |
| Fix_AI_CCD | 全部 Key | AI CCD IP/Port/Light |
| Tray_Mapping | 全部 Key | Tray 映射 CCD IP/Port |
| Auto_Alignment | 全部 Key | 自動對位 CCD IP/Port |
| AutoTeach | 全部 Key | 自動教導偏移 |
| ATC | USE_ATC_SELFTEST | ATC 自我測試 |
| ROTATE_KIT | SpecialSequence | 特殊序列 |
| IndexDriver | USE_ReadIndex_TOQUE | Index 力矩讀取 |
| System | T_MODE_SPEED | T 模式速度 |
| System | FixTrayDataCleanTime | Fix Tray 清除時間 |
| System | bHT9045S_USE2x4 | 2x4 模式 |
| System | bBarCodeRules (KLT) | KLT 功能 |
| System | OTDRecord | OTD 紀錄 |
| System | USE_BARCODE_AS_KEYBOARD | 條碼鍵盤 |
| System | USE_ARM_PROTECTION | 臂保護 |
| System | iDBQueryDays | 資料庫查詢天數 |
| System | iMagazineCheckZPos | Magazine Z 位置 |
| System | JCET_FOR_EVAN | JCET 特殊 |
| System | iAutoFormSize | Auto 表單大小 |
| System | OFFLINE_ALARM | 離線警報 |
| System | CHECK_EP_SETTING | EP 設定檢查 |
| System | USE_InPlacement | In Placement |
| System | USE_ATC_RS232_Check | ATC RS232 檢查 |
| System | bHasEnteredPEModel | PE 模式追蹤 |

---

## 特殊邏輯

1. **CUSTOMER_CODE 串聯效應**：讀取後呼叫 `CustomerFunctionSelect()`，覆寫多個預設值（如 ASE_KaohSiung 的 CatchTrayBlock、ARM_PROTECTION 等）
2. **機型串聯**：Version/Model 決定 MachineTypeChoice，影響 TEMPCTRL、SHUTTLE_Z_TYPE、46_SUCKER/SENSOR 等
3. **AMBIENT_TEMP_CHECK Loop**：以 tcTotalCount 為長度迴圈讀取 01–71（最多 71 個），Tri_Temp_Machine==1 時全部清零
4. **AUTO_EMPTY_COLOR 缺失處理**：INI 中找不到時彈出 Dialog 詢問使用者，並回寫 INI
5. **COM Port auto-prepend**：部分 COM Port 欄位若讀取值不以 "COM" 開頭，自動補上前綴
6. **條件回寫**：USE_46_SUCKER_DB、USE_46_SENSOR_DB 在 HT9046/1032 機型強制歸零並 WriteIniDataGeneral
