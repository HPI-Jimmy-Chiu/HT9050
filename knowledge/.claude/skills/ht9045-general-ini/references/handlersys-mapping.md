# HandlerSys.cpp — SaveSystemSet() / LoaderSystemSet() INI 對應表

> 原始檔：`HT9045/HT9011UC_Code_V3.33.898.0_20260313_Steven/HandlerSys.cpp`
> `SaveSystemSet()` 約 lines 445–945，`LoaderSystemSet()` 約 lines 90–440

## 目錄

- [System Section](#system-section)
- [TrayZ Section](#trayz-section)
- [TrayCassette Section](#traycassette-section)
- [TrayY Section](#trayy-section)
- [Version Section](#version-section)
- [Index Section](#index-section)
- [IndexDriver Section](#indexdriver-section)
- [TempCtrl Section](#tempctrl-section)
- [2D_BarCode Section](#2d_barcode-section)
- [OCR Section](#ocr-section)
- [ATC Section](#atc-section)
- [EM Aware Section](#em-aware-section)
- [NUMBER_PANEL Section](#number_panel-section)
- [RealTimeCCD Section](#realtimeccd-section)
- [OutSortArm Section](#outsortarm-section)
- [ROTATE_KIT Section](#rotate_kit-section)
- [AIR_CON Section](#air_con-section)
- [Laser Section](#laser-section)
- [Vibration Section](#vibration-section)
- [Ground_Man Section](#ground_man-section)
- [RFID Section](#rfid-section)
- [MachineDefine Section](#machinedefine-section)
- [外部 INI 寫入](#外部-ini-寫入)
- [特殊邏輯](#特殊邏輯)

---

## System Section

| INI Key | UI 元件 | 資料型別 | 說明 |
|---------|---------|----------|------|
| AUTO_EMPTY_COLOR | MachineTrack→ItemIndex | RadioGroup | 機台配置代碼（7 Track = Empty Unloader） |
| SUPPORT_2_EMPTY_EMPTY | (條件寫入) | Int | MachineTrack==2 時寫 1，否則 0 |
| EP_Install | ElectronPressure→ItemIndex | RadioGroup | 電子壓力裝置 |
| INOUT_ARM_PICKER_USE_MOTOR | ArmZAtoZH→ItemIndex | RadioGroup | 進出臂 Z 軸馬達 |
| bUseAuto2Empty | Auto2SelectCy→ItemIndex | RadioGroup | Auto2 分離氣缸裝置 |
| USE_AUTO_RETEST | rgInstallAutoRestest→ItemIndex | RadioGroup | 自動重測功能 |
| UNLOADER_AUTO1_ART | rgAuto1ART→ItemIndex | RadioGroup | Auto1 ART 配置 |
| UNLOADER_AUTO2_ART | rgAuto2ART→ItemIndex | RadioGroup | Auto2 ART 配置 |
| UNLOADER_AUTO3_ART | rgAuto3ART→ItemIndex | RadioGroup | Auto3 ART 配置 |
| UNLOADER_AUTO4_ART | rgAuto4ART→ItemIndex | RadioGroup | Auto4 ART 配置 |
| UNLOADER_AUTO5_ART | rgAuto5ART→ItemIndex | RadioGroup | Auto5 ART 配置 |
| UNLOADER_AUTO6_ART | rgAuto6ART→ItemIndex | RadioGroup | Auto6 ART 配置 |
| bAutoTrackCanGoRear | rgAutoTrackCanGoRear→ItemIndex | RadioGroup | Auto123 前進後退 |
| bAutoZNoUseART | rgInstallARTtwocylinder→ItemIndex | RadioGroup | ART 一段氣缸 |
| CUSTOMER_CODE | edtCustomerCode→Text→int | Edit | 客戶代碼 |
| NUMBER_PANEL_TYPE | rgNumberPanelType→ItemIndex | RadioGroup | 數字面板類型 |
| WEIGHT_CALIBRATION | rgWeightCali→ItemIndex | RadioGroup | 磅秤校正設定 |
| ION_FAN_TYPE | rgIonFanType→ItemIndex | RadioGroup | 風扇類型 |
| SHUTTLE_SENSOR_TYPE | rgShuttleSensor→ItemIndex | RadioGroup | Shuttle 感測器型態 |
| NUEC_TYPE | rgNUECType→ItemIndex | RadioGroup | EtherCAT Shuttle 感應器 |
| ENABLE_OUT_SHUTTLE_SENEOR | cbEnableOutShtSensor→Checked | CheckBox | 旁路 Out Shuttle 感應器 |
| ENABLE_OUT_SHUTTLEY_LATCH | chkOutShtYSensorByLatch→Checked | CheckBox | Out Shuttle Y 感應器 LATCH |
| Use_AxisY_Sensor_2x3mode | chk2x3modeUseAxisYSensor→Checked | CheckBox | 2x3 mode 用 Y 感應器 |
| Bias_Mode_Use_Y_Sensor | chkBiasModeUseYSensor→Checked | CheckBox | 1x2 Bias mode 用 Y 感應器 |
| SAFE_DOOR_AMOUNT | rgSafeDoor→ItemIndex | RadioGroup | 安全門數量 |
| INDEX_SUCKER_TYPE | rgIndexSuckerType→ItemIndex | RadioGroup | Index 吸氣型態（負壓） |
| INDEX_PRESS_TYPE | rgIndexPressType→ItemIndex | RadioGroup | Index 夾緊型態（85KG/240KG） |
| TRAY_VIBRATION | rgTrayVibration→ItemIndex | RadioGroup | Tray 震動馬達 |
| USE_2nd_LOADER | rg2ndLoader→ItemIndex | RadioGroup | 第二個 Loader（HT-9046AU） |
| TRAY_ARM_MODE | rgTrayArmMode→ItemIndex | RadioGroup | Tray 臂模式 |
| LOADER_VIBRATION | rgLoaderVibration→ItemIndex | RadioGroup | Loader 震動馬達 |
| USE_TRAY_ROBOT | rgTrayRobot→ItemIndex | RadioGroup | Tray 機械臂（HT-9046LM） |
| USE_LOADER_HINGE | rgLoaderHinge→ItemIndex | RadioGroup | Loader 鉸鏈（TSMC） |
| USE_DIE_CLEAN | rgDieClean→ItemIndex | RadioGroup | 晶片清洗 |
| USE_CKD_FCM_CleanAir | rgCKDFCM→ItemIndex | RadioGroup | CKD FCM 清潔空氣 |
| USE_CATCH_TRAY_MODEL | rgCatchTrayModel→ItemIndex | RadioGroup | 夾 tray 模式 |
| UseCatchTrayBlock | rgCatchTrayBlock→ItemIndex | RadioGroup | 夾 tray 遮版削短 |
| USE_16_HEATER | rgHeater→ItemIndex | RadioGroup | 16 個加熱器 |
| REAL_TIME_CCD | rgRealTimeCCD→ItemIndex | RadioGroup | 即時 CCD |
| RTC_TemperNumber | rgRealTimeCCDTempNum→ItemIndex | RadioGroup | RTC 感溫數量（RTC≠0 時） |
| CCD2_TEMPER | rgCCDTemp→ItemIndex | RadioGroup | CCD2 溫度 |
| LB_TEMP | rgLBTemp→ItemIndex | RadioGroup | LB 溫度 |
| LB_TEMP_UpDown | rgLBTemp2→ItemIndex | RadioGroup | LB 溫度上下 |
| Index_ESDAir | rgESDTemp→ItemIndex | RadioGroup | Index ESD 空氣 |
| INSTALL_OCR | rgOCR→ItemIndex | RadioGroup | 安裝 OCR |
| INSTALL_OCR_YMot | rgOCRYStepMot→ItemIndex | RadioGroup | OCR Y 軸步進馬達 |
| IN_SHT_LAST_SENSOR | rgInShtLastSensor→ItemIndex | RadioGroup | In Shuttle 最後感應器 |
| SAFE_DOOR_LOCK | rgSafeDoorLock→ItemIndex | RadioGroup | 安全門鎖定 |
| HOT_PLATE_POSITION | rgHotPlatePos→ItemIndex | RadioGroup | 加熱盤位置 |
| HOT_PLATE_LIMITATION | rgHotPlateLimit→ItemIndex | RadioGroup | 加熱盤極限位置 |
| USE_LASER_DISTANCE | rgLaserDistance→ItemIndex | RadioGroup | 雷射測距 |
| USE_DEVICE_FLIPPER | rgDeviceFlipper→ItemIndex | RadioGroup | 翻轉裝置 |
| MAGAZINE_BIN_DISP_TYPE | rgMagBinDispType→ItemIndex | RadioGroup | 雜誌料箱顯示類型 |
| MACHINE_HAS_AUTO_ALIGNMENT_CCD | rgCCDAutoAlignmentMode→ItemIndex | RadioGroup | 自動對位 CCD |
| USE_IN_OUT_ARM_Y_PITCH | rgInOutArmYPitch→ItemIndex | RadioGroup | 進出臂 Y Pitch 模式 |
| USE_OUT_ARM_Y_PITCH | rgOutArmYPitch→ItemIndex | RadioGroup | 出臂 Y Pitch 模式 |
| IN_OUT_ARM_Y_PITCH_MIN | edtMinYPitch→Text | Edit | Y Pitch 最小值 |
| IN_OUT_ARM_Y_PITCH_MAX | edtMaxYPitch→Text | Edit | Y Pitch 最大值 |
| USE_IN_OUT_ARM_X_PITCH | rgInOutArmXPitch→ItemIndex | RadioGroup | 進出臂 X Pitch 模式 |
| IN_OUT_ARM_X_PITCH_MIN | edtMinXPitch→Text | Edit | X Pitch 最小值 |
| IN_OUT_ARM_X_PITCH_MAX | edtMaxXPitch→Text | Edit | X Pitch 最大值 |
| BASE_X_TO_HP | edtHPLimit→Text | Edit | 基準軸 X 到加熱盤距離 |
| INSTALL_HEAT_GUN | rgHeatGun→ItemIndex | RadioGroup | 熱風槍機構 |
| INSTALL_ATC_HEAT_GUN | rgATCHeatGun→ItemIndex | RadioGroup | ATC3.5 熱風槍 |
| FIX3_FULL_PLACE | rgFix3FullPlace→ItemIndex | RadioGroup | Fix3 滿盤功能 |
| USE_MAGNETIC_SCALE | rgMagneticScale→ItemIndex | RadioGroup | 磁性尺 |
| USE_PICKER_COUNT | rgPickerCount→ItemIndex | RadioGroup | 吸嘴數量 |
| USE_COLOR_TRAY_SENSOR | rgColorSensor→ItemIndex | RadioGroup | 顏色 Tray 感應器 |
| USE_SOCKET_SENSOR | rgSocketSen→ItemIndex | RadioGroup | Socket 感應器 |
| Canbus_Method | rgCanBusMethod→ItemIndex | RadioGroup | CanBus 軟體配置 |
| USE_PRECISER | rgPreciser→ItemIndex | RadioGroup | InArm Preciser 站台 |
| iPreciserInstallArea | rgPreciserPos→ItemIndex | RadioGroup | Preciser 安裝位置 |
| USE_HOTPLATE_TYPE | rgHotplateType→ItemIndex | RadioGroup | Hotplate 類型 |
| MOTION_CARD_TYPE | rgMotionCard→ItemIndex | RadioGroup | Motion 卡模式 |
| MOTIONNET_SPEED | rgMNetSpeed→ItemIndex | RadioGroup | Motion Net 速度 |
| IO_CARD_TYPE | rgIOCard→ItemIndex | RadioGroup | I/O 卡模式 |
| TTL_CARD_TYPE | rgTTLCard→ItemIndex | RadioGroup | TTL 卡模式 |
| TTL_CARD_USE_ADDRESS | rgTTLUseAddress / (雙卡時強制 1) | RadioGroup | TTL 卡站別 |
| SHUTTLE_Z_TYPE | rgShuttleZType→ItemIndex | RadioGroup | Shuttle Z 感應器類型 |
| FIX3_INSTALL | rgInstallFix3→ItemIndex | RadioGroup | Fix3 安裝 |
| CROSS_SENSOR_INSTALL | rgShuttleCrossSensor→ItemIndex | RadioGroup | Shuttle cross 感應器 |
| AUTO_SENSOR_INSTALL | rgAutoShuttleSensor→ItemIndex | RadioGroup | Auto Shuttle 感應器 |
| ShuttleVibration | rgShuttleVibration→ItemIndex | RadioGroup | Shuttle 震動馬達 |
| USE_TRAY_MAPPING | rgTrayMapping→ItemIndex | RadioGroup | Tray 映射 |
| Fix_AI_CCD | rgFixAICCD→ItemIndex | RadioGroup | 固定 AI CCD |
| USE_AUTO_ALIGNMENT | rgAutoAlignment→ItemIndex | RadioGroup | 自動對位 |
| USE_COLORSENSOR_MUN | rgCOLORSENSOR_MUN→ItemIndex | RadioGroup | Color 感應器 MU-N |
| USE_ESD_Monior | rg3M_EM_AWARE_Monitor→ItemIndex | RadioGroup | ESD 監控 |
| USE_OTD | rgOTDInstall→ItemIndex | RadioGroup | OTD |
| USE_NOVX3360 | rgNovx3360→ItemIndex | RadioGroup | Simco ION 風扇 |
| USE_AutoCleanIonFan | rgAutoCleanIonFan→ItemIndex | RadioGroup | IO 觸發 IonFan 清針 |
| USE_PULSE_TYPE | rgUsePulseType→ItemIndex | RadioGroup | Simco 脈衝類型 |
| ION_PULSE_COUNT | edIONPulseCount→Text | Edit | ION 脈衝計數 |
| USE_KASUGA | rgKasuga→ItemIndex | RadioGroup | Kasuga ION 風扇 |
| USE_KASUGA_Fan | rgKasuga_Fan→ItemIndex | RadioGroup | Kasuga 風扇通訊 |
| KASUGA_Fan_PORT | cbKASUGA_Fan→Text | ComboBox | KASUGA 風扇通訊埠 |
| HTIonBarFunction | rgHTIonBar→ItemIndex | RadioGroup | Unloader IonBar 功能 |
| SocketBasedAdd4Temp | rgUse4DUT→ItemIndex | RadioGroup | Socket 基礎加 4 溫（HT9046AH） |
| USE_46_SUCKER_DB | rgUseSucker_9046_DB→ItemIndex | RadioGroup | HT9045 使用 46 配氣 |
| USE_46_SENSOR_DB | rgUseSensor_9046_DB→ItemIndex | RadioGroup | HT9045 使用 46 配電 |
| INDEX_MOTION_CARD | rgIndexMotionCard→ItemIndex | RadioGroup | Index 使用 Galil |
| ControlPanelMode | rg_ControlPanelMode→ItemIndex | RadioGroup | 控制面板模式 |
| VacuUnitType | rgVacuUnitType→ItemIndex | RadioGroup | VacuumUnit 通訊模組 |
| USE_FINE_PITCH | rgFinePitch→ItemIndex | RadioGroup | Fine Pitch |
| USE_OUT_SHT_MOT | rgUseOutSht→ItemIndex | RadioGroup | Out Shuttle 獨立馬達 |
| AOI | rgAOI→ItemIndex | RadioGroup | AOI（SPIL） |
| EP_MAXKPA | edMaxKpa→Text | Edit | 最大電子壓力 KPA |
| EP_MAXA | edMaxMpaFB→Text | Edit | 最大電子壓力 MPA |
| EP_MINMPA | edMinMpa→Text | Edit | 最小電子壓力 MPA |
| EP_MINA_FeedBack | edtMinMpaFB→Text | Edit | 最小電子壓力反饋 MPA |
| USE_MR_SYSTEM | rgMRSystem→ItemIndex | RadioGroup | MR 系統 |
| I24V_PULSE_COUNT | ed24VMonitorPulseCount→Text | Edit | 24V 監控脈衝計數 |
| USE_RFID_SYSTEM | rgRFIDSystem→ItemIndex | RadioGroup | RFID 系統 |
| USE_RFID_READER | rgRFIDReader→ItemIndex | RadioGroup | RFID 讀取器（SJSEMI） |
| CHAMBER_USE_PULSE_TYPE | rgChamberUsePulseType→ItemIndex | RadioGroup | Chamber 脈衝類型 |
| SocketSenAmpQty | cbSocketSenAmpCnt→ItemIndex | ComboBox | Socket 感應器放大數量 |
| RotateSenAmpQty | cbRotateSenAmpCnt→ItemIndex | ComboBox | 旋轉感應器放大數量 |
| ColorSenAmpQty | cbColorSenAmpCnt→ItemIndex | ComboBox | 顏色感應器放大數量 |
| SocketSenAmpQty2nd | cbSocketSenAmpCnt2nd→ItemIndex | ComboBox | 第二 Socket 感應器放大 |
| VibrationCardQty | cbVibrationCardQty→ItemIndex | ComboBox | 震動卡數量 |
| CanBusNudn1Qty | coCanBusNudn1→ItemIndex | ComboBox | CanBus NUDN1 數量 |
| Nudn1Macid11Qty | coNudn1Macid11→ItemIndex | ComboBox | NUDN1 MAC ID11 數量 |
| Nudn1Macid12Qty | coNudn1Macid12→ItemIndex | ComboBox | NUDN1 MAC ID12 數量 |
| Nudn1Macid13Qty | coNudn1Macid13→ItemIndex | ComboBox | NUDN1 MAC ID13 數量 |
| Nudn1Macid14Qty | coNudn1Macid14→ItemIndex | ComboBox | NUDN1 MAC ID14 數量 |
| Scanner_AOI | rgScanner_AOI→ItemIndex | RadioGroup | Scanner AOI |
| Top_Scanner_AOI | rgTopScanner_AOI→ItemIndex | RadioGroup | TFAMD Top AOI |
| HighTemperatureSet150 | (條件寫入) | Int | 150 度溫度設定 |
| HighTemperatureSet155 | (條件寫入) | Int | 155 度溫度設定 |
| HighTemperatureSet175 | (條件寫入) | Int | 175 度溫度設定 |
| HighTempLimit | rgHighTempLimit→ItemIndex | RadioGroup | 高溫限制 |
| DewPoint_Hardware_Install | rgDewpointHW→ItemIndex | RadioGroup | 露點計硬體 |
| INSTALL_SOCKET_CLAMP | rgSocketClamp→ItemIndex | RadioGroup | Socket 夾持汽缸 |
| INSTALL_DOUBLE_EP | rgDoubleEPControl→ItemIndex | RadioGroup | 雙電子壓力控制 |
| HotGunFlowEnable | rgHotGunFlow→ItemIndex | RadioGroup | 熱槍流量啟用 |
| HotGunFlow_LineNo | edHotGunFlow_LineNo→Text | Edit | 熱槍流量行號 |
| HotGunFlow_DevNo | edHotGunFlow_DevNo→Text | Edit | 熱槍流量裝置號 |
| HotGunFlow_Gun1_ChannelNo | edHotGunFlow_Gun1_ChannelNo→Text | Edit | 熱槍 1 通道號 |
| HotGunFlow_Gun2_ChannelNo | edHotGunFlow_Gun2_ChannelNo→Text | Edit | 熱槍 2 通道號 |
| Individual_EP_COUNT | (條件 "16" or "4") | String | 獨立電子壓力數量 |
| AUTO3_IS_MAGAZINE | rgAuto3Magazine→ItemIndex | RadioGroup | Auto3 是否為 Magazine |
| USE_OHT_SYSTEM | rgOHTSystem→ItemIndex | RadioGroup | OHT 系統 |
| USE_Multile_Empty | rgMultileEmpty→ItemIndex | RadioGroup | 多個 Empty |
| USE_KEYENCE_LOADER | rgLoaderKeyence→ItemIndex | RadioGroup | Keyence Loader |
| USE_KEYENCE_EMPTY | rgEmptyKeyence→ItemIndex | RadioGroup | Keyence Empty |
| USE_MultileEmptyTrayID_Keyence | rgMultileEmptyTrayIDKeyence→ItemIndex | RadioGroup | 多個 Empty Tray ID |
| TRAY_MAPPING_GRAB | rgTrayMappingGrabImage→ItemIndex | RadioGroup | Tray 映射投擲 IC 功能 |
| BASE_HEATER_COUNT | rgBaseHeaterCount→ItemIndex | RadioGroup | 基礎加熱器數（HT-1032） |
| SHUTTLE_FLOODGATE | rgShuttleFloodgate→ItemIndex | RadioGroup | Shuttle 閘門（HT-1032） |
| Tri_Temp_Machine | rgTriTempMachine→ItemIndex | RadioGroup | 三溫機器（HT-1032） |
| AirStream_Select | rgAirStreamSelect→ItemIndex | RadioGroup | 氣流選擇（HT-1032） |
| Tri_Temperature_MaxDegree | edtTriTemperature_MaxDegree→Text | Edit | 三溫最高溫度 |
| Tri_Temperature_MinDegree | edtTriTemperature_MinDegree→Text | Edit | 三溫最低溫度 |
| TriTemperature_TotalChannel | edtTriTempTotalCh→Text | Edit | 三溫總通道數 |
| SetHeaterTemp_MaxOutSht | edtOutShtMaxTemp→Text | Edit | Out Shuttle 最高溫 |
| SetHeaterTemp_MaxIndex | edtIndexMaxTemp→Text | Edit | Index 最高溫 |
| SetHeaterTemp_MaxBase | edtBaseMaxTemp→Text | Edit | 基座最高溫 |
| Total_Compressor | edt_Total_Compressor→Text | Edit | 壓縮機總數 |
| IndexDoorHeater | rg_IndexDoorHeater→ItemIndex | RadioGroup | Index 門加熱 |
| SafePlcIO | rgSafePlcIO→ItemIndex | RadioGroup | 安全 PLC IO |
| DOUBLE_BELT_MODE | rgDoubleBeltMode→ItemIndex | RadioGroup | 雙皮帶模式 |
| USE_COVER_TRAYID | rgCoverTrayID→ItemIndex | RadioGroup | 使用蓋 Tray ID |
| In_Shuttle_Auto_Latch | rgInShtAutoLatch→ItemIndex | RadioGroup | InSht 感應器自動 Latch |
| USE_LD_Rot_Arm | rgLDCarRotAtm→ItemIndex | RadioGroup | Loader 旋轉臂 |
| AGVModal | rgE84Sensor→ItemIndex | RadioGroup | E84 感應器 |
| USE_LdUldCassetteMode | rgLdUldCassetteMode→ItemIndex | RadioGroup | Boat Carrier |

## TrayZ Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| LOAD_Z_USE_MOTOR | chkLoader→Checked | Tray Z 馬達（Loader） |
| EMPTY_Z_USE_MOTOR | chkEmpty→Checked | Tray Z 馬達（Empty） |
| COLOR_Z_USE_MOTOR | chkColor→Checked | Tray Z 馬達（Color） |
| AUTO1_Z_USE_MOTOR | chkAuto1→Checked | Tray Z 馬達（Auto1） |
| AUTO2_Z_USE_MOTOR | chkAuto2→Checked | Tray Z 馬達（Auto2） |
| AUTO3_Z_USE_MOTOR | chkAuto3→Checked | Tray Z 馬達（Auto3） |
| AUTO4_Z_USE_MOTOR | chkAuto4→Checked | Tray Z 馬達（Auto4） |
| AUTO5_Z_USE_MOTOR | chkAuto5→Checked | Tray Z 馬達（Auto5） |
| AUTO6_Z_USE_MOTOR | chkAuto6→Checked | Tray Z 馬達（Auto6） |

## TrayCassette Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| LOAD_USE_Cassette | cbLoaderCassette→Checked | Cassette（Loader） |
| EMPTY_USE_Cassette | cbEmptyCassette→Checked | Cassette（Empty） |
| COLOR_USE_Cassette | cbColorCassette→Checked | Cassette（Color） |
| AUTO1_USE_Cassette | cbAuto1Cassette→Checked | Cassette（Auto1） |
| AUTO2_USE_Cassette | cbAuto2Cassette→Checked | Cassette（Auto2） |
| AUTO3_USE_Cassette | cbAuto3Cassette→Checked | Cassette（Auto3） |
| AUTO4_USE_Cassette | cbAuto4Cassette→Checked | Cassette（Auto4） |
| AUTO5_USE_Cassette | cbAuto5Cassette→Checked | Cassette（Auto5） |
| AUTO6_USE_Cassette | cbAuto6Cassette→Checked | Cassette（Auto6） |

## TrayY Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| LoaderUnload_StepMotor | rgLdUldUseStepMotor→ItemIndex | Loader 入 Tray 改步進馬達 |
| COM PORT | cbbTrayStepMotor→Text | 步進馬達通訊埠 |

## Version Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| Serial No | edtSeriaNo→Text | 序列號 |
| SubModel | rgModel→ItemIndex (0=None) | 子型號 |
| Ver | asHandlerVersion | Handler 版本 |

## Index Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| EnableUserDefMaxContactHeight | chkUserDefMaxContactHeight→Checked | 用戶定義最大接觸高度 |
| UserDefMaxContactHeight | edtUserDefMaxContactHeight→Text | 最大接觸高度值 |
| EnableUser_Define_IndexZ_SafePos | chkUser_Define_IndexZ_SafePos→Checked | 用戶定義 Index Z 安全位置 |
| UserDefineIndexZSafePos | edtUserDefineIndexZSafePos→Text | Index Z 安全位置值 |

## IndexDriver Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| COM_PORT | cbComIndex→Text | Index 控制器通訊埠 |
| USE_IO_CHANGE_TOQUE | rgIOChangeToque→ItemIndex | Index I/O 改變力矩 |
| INDEX_DRIVER_TYPE | rgIndexMotorType→ItemIndex | Index 馬達類型 |
| USE_HP_COM_CARD | chkUseHPComCard→Checked | 使用鴻勁通訊卡 |
| CLEAN_AIR | rgCleanAir→ItemIndex | 清潔空氣 |
| USE_INDEX_ARM_AXES | rgIndexMotorAxis→ItemIndex | Index 臂軸線 |

## TempCtrl Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| COM_PORT | cbComTemp→Text | 溫控通訊埠 |
| COM_PORT_OMRON | cbComTempOmron→Text | OMRON 溫控通訊埠 |
| COM_PORT_DYNAMIC | cbComDyTemp→Text | 動態溫控通訊埠 |
| HEATER_CTRL_TYPE | rgHeaterType→ItemIndex | HEATER 控制器類型 |

## 2D_BarCode Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| BOTTOM_2DID | rgBottom2DID→ItemIndex | 底部 2D 條碼 ID |
| BAR_CODE_INSTALL | ebctUseCCDMode / rg2DBarcode | 2D 條碼安裝（CCD 模式或其他） |
| SHT_FLOATING_CHK | rgShuttleFloating→ItemIndex | IC 置偏檢查 |
| BAR_CODE_USECOUNT | edCognexSystemCCD→Text | COGNEX 系統 CCD 數量 |
| BOTTOM_2DID_CCD | rgBottom2DID_CCD→ItemIndex | 底部 2D 8CCD |
| BarCode1_COM_PORT | cb2DReader1→Text | 2D 讀取器 1 埠 |
| BarCode2_COM_PORT | cb2DReader2→Text | 2D 讀取器 2 埠 |
| BarCode3_COM_PORT | cb2DReader3→Text | 2D 讀取器 3 埠 |
| BarCode4_COM_PORT | cb2DReader4→Text | 2D 讀取器 4 埠 |

## OCR Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| OCR_COM_PORT | cbOCR→Text | OCR 通訊埠 |
| OCRwithTester_COM_PORT | cbOCRwithTester→Text | OCR with Tester 通訊埠 |

## ATC Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| USE_ATC_MODE | rgATC→ItemIndex | ATC 模式 |
| ATC1_COM_PORT | cbbATC1→Text | ATC1 通訊埠 |
| ATC2_COM_PORT | cbbATC2→Text | ATC2 通訊埠 |
| ATC3_COM_PORT | cbbATC3→Text | ATC3 通訊埠 |
| ATC4_COM_PORT | cbbATC4→Text | ATC4 通訊埠 |
| ATC_SYSTEM_IP | edATCSystemIP→Text | ATC 系統 IP |
| ATC_SYSTEM_PORT | edATCSystemPort→Int (NewATC 強制 1234) | ATC 系統埠 |
| ATC_SYSTEM_USEHEAT | edATCSystemUseHeat→Text | ATC 系統 Heat 計數 |

## EM Aware Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| EM_AWARE_PORT1 | cbEMAwarePort1→Text | ESD 監控埠 1 |
| EM_AWARE_PORT2 | cbEMAwarePort2→Text | ESD 監控埠 2 |
| EM_AWARE_PORT3 | cbEMAwarePort3→Text | ESD 監控埠 3 |
| EM_AWARE_PORT4 | cbEMAwarePort4→Text | ESD 監控埠 4 |
| EM_AWARE_USE_4_COM | cbESDUse4COM→Checked | ESD 使用 4 個 COM 埠 |
| NOVX_3360_PORT | cbNovx3360→Text | Simco ION 風扇埠 |

## NUMBER_PANEL Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| COM_PORT | cbComBinDisp→Text | 數字面板通訊埠 |
| (NUMBER_PANEL2) COM_PORT | cbbComBinDisp2→Text | 第二數字面板通訊埠 |

## RealTimeCCD Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| Port | cbComRTC→Text | 即時 CCD 通訊埠 |

## OutSortArm Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| USE_OUT_SORT_ARM | rgOutSortArm→ItemIndex | 出臂分級臂（HT-9046AU） |
| USE_OUT_SORT_X_PITCH_MIN | edtOutSortXPitchMin→Text | X Pitch 最小 |
| USE_OUT_SORT_X_PITCH_MAX | edtOutSortXPitchMax→Text | X Pitch 最大 |

## ROTATE_KIT Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| USE_ROTATE_KIT | rgRotateKit→ItemIndex | 旋轉 Kit |
| RotateKit_Type | rgRotateKit_Type→ItemIndex | 旋轉 Kit 類型（馬達版） |
| iRotate_In_Index | rgRotateKitIn→ItemIndex (Type==1 時強制 -1) | 旋轉進站 |
| iRotate_Out_Index | rgRotateKitOut→ItemIndex (Type==1 時強制 -1) | 旋轉出站 |

## AIR_CON Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| USE_AIR_CONDITIONER | rgAirConditioner→ItemIndex | 冷氣機 |
| AIR_CON_PORT | cbAirCon→Text | 冷氣機埠 |

## Laser Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| COM_Laser_1 | cbComLaser1→Text | 雷射 1 通訊埠 |
| COM_Laser_2 | cbComLaser2→Text | 雷射 2 通訊埠 |
| COM_Laser_InArm | cbComLaserInArm→Text | 進臂雷射通訊埠 |

## Vibration Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| VibrationCommunication | rgVibrationCommuncation→ItemIndex | 震動馬達通訊調速 |

## Ground_Man Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| USE_GROUND_MAN | rgGroundMan→ItemIndex | 通訊式 GroundMan |
| Ground_Man_COM_PORT | cbbGroundMan→Text | GroundMan 通訊埠 |
| Ground_Man_ScanPoint | rgGroundMan_ScanPoint→ItemIndex | GroundMan 掃描點 |
| Ground_Man_AlarmOhm | edGroundMan_AlarmOhm→Text | GroundMan 報警歐姆 |

## RFID Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| RFIDReader_PORT | cbbRFIDReader→Text | RFID 讀取器埠 |

## MachineDefine Section

| INI Key | UI 元件 | 說明 |
|---------|---------|------|
| Tri_Temp_Machine | rgTriTempMachine→ItemIndex | 三溫機器（同時寫入 System 與 MachineDefine） |

---

## 外部 INI 寫入

| 外部 INI 路徑 | Section | Key | UI 元件 |
|---------------|---------|-----|---------|
| `D:\GPIB9045\system\general.ini` | Version | Model | (機型代碼字串) |
| `D:\RS232Standard\System\Setup.ini` | COMPort | CommName | cbComTester→Text |
| `D:\RS232Standard\System\Setup.ini` | COMPort_TTL | CommName | cbComTTLRS232→Text |
| `D:\RS232Standard\System\Setup.ini` | COMPort_TTL_2 | CommName | cbComTTLRS232_2→Text |

---

## 特殊邏輯

1. **溫度限制向前相容**：`HighTempLimit` 寫入前，先清除 `HighTemperatureSet150/155/175`，再依 ItemIndex 設定對應旗標
2. **TTL 雙卡強制**：當 TTL_CARD_TYPE == 2（雙卡）時，`TTL_CARD_USE_ADDRESS` 強制寫 1
3. **Bottom 2D → CCD 模式**：`BOTTOM_2DID == 1` 時強制 `BAR_CODE_INSTALL = ebctUseCCDMode`
4. **ATC Port 強制**：NewATC 模式下 `ATC_SYSTEM_PORT` 強制寫 1234
5. **旋轉 Kit 馬達版**：`RotateKit_Type == 1` 時 `iRotate_In_Index` 和 `iRotate_Out_Index` 強制 -1
6. **Tri_Temp_Machine 雙寫**：同時寫入 `[System]` 和 `[MachineDefine]` 兩個 Section
7. **GPIB Model 同步**：`Version/Model` 同步寫入 `D:\GPIB9045\system\general.ini`
