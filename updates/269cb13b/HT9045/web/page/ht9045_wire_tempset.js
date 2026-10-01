/* ht9045_wire_tempset.js -- Setup.Temp_Set.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260915  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    uTemp_Set.dfm
 * 抽取法      WriteIniData（widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             兩者都限定在該表單自己的 .cpp，不做全樹 id 反查。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 56 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 86 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 10 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 156 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'Setup.Temp_Set.html',
  slug: 'tempset',
  fields: {
    edAbitColdTime:              ['temperature', 'Time', 'Cool Time'],
    edAbitInitWaitTime:          ['temperature', 'Time', 'A.Initial'],
    edAmbTemp:                   ['temperature', 'Ambient', 'Temperature'],
    edArm1NoFullsiteOffset_1:    ['temperature', 'Index', 'Arm1 No Fullsite Offset1'],
    edArm1NoFullsiteOffset_2:    ['temperature', 'Index', 'Arm1 No Fullsite Offset2'],
    edArm1NoFullsiteOffset_3:    ['temperature', 'Index', 'Arm1 No Fullsite Offset3'],
    edArm1NoFullsiteOffset_4:    ['temperature', 'Index', 'Arm1 No Fullsite Offset4'],
    edArm1NoFullsiteOffset_5:    ['temperature', 'Index', 'Arm1 No Fullsite Offset5'],
    edArm1Offset:                ['temperature', 'Index', 'Arm1 Offset'],
    edArm2NoFullsiteOffset_1:    ['temperature', 'Index', 'Arm2 No Fullsite Offset1'],
    edArm2NoFullsiteOffset_2:    ['temperature', 'Index', 'Arm2 No Fullsite Offset2'],
    edArm2NoFullsiteOffset_3:    ['temperature', 'Index', 'Arm2 No Fullsite Offset3'],
    edArm2NoFullsiteOffset_4:    ['temperature', 'Index', 'Arm2 No Fullsite Offset4'],
    edArm2NoFullsiteOffset_5:    ['temperature', 'Index', 'Arm2 No Fullsite Offset5'],
    edArm2Offset:                ['temperature', 'Index', 'Arm2 Offset'],
    edBelowSec:                  ['tester', 'InitialMode', 'iEveryFirstDeviceUseInitialDelay'],
    edChamberCoolTemp:           ['temperature', 'Cooling', 'ChamberCoolTemp'],
    edHighBase:                  ['temperature', 'High OffSet', 'Base'],
    edInitialStart1Time:         ['temperature', 'Time', 'iInitialStart1Time'],
    edInitialStart2Time:         ['temperature', 'Time', 'iInitialStart2Time'],
    edInitialWaitTime:           ['temperature', 'Time', 'H.Initial'],
    edJamSoakTime:               ['temperature', 'Time', 'Jam Soak'],
    edLowBase:                   ['temperature', 'Low OffSet', 'Base'],
    edMidBase:                   ['temperature', 'Mid. OffSet', 'Base'],
    edOverSec:                   ['tester', 'InitialMode', 'iWhenPressStopOver'],
    edSoakTime:                  ['temperature', 'Time', 'Soak'],
    edSocketAirCoolingOff:       ['temperature', 'Index', 'dSocketAirCoolingOffTimer'],
    edSocketAirCoolingOn:        ['temperature', 'Index', 'dSocketAirCoolingOnTimer'],
    edWorkTemp:                  ['temperature', 'Mode', 'Temperature'],
    edtATC_HotGunTemp:           ['temperature', 'ATC', 'dATC_HotGunTemp'],
    edtATC_HotGunTime:           ['temperature', 'ATC', 'dATC_HotGunTime'],
    edtBoostDuration_LB:         ['temperature', 'LB Temp Function', 'dBoostDuration[3]'],
    edtBoostDuration_Long:       ['temperature', 'Boost Function', 'dBoostDuration[0]'],
    edtBoostDuration_Short:      ['temperature', 'Boost Function', 'dBoostDuration[2]'],
    edtBoostOffset:              ['temperature', 'LB Temp Function', 'dBoostOffset[5]'],
    edtBoostOffset_LB:           ['temperature', 'LB Temp Function', 'dBoostOffset[3]'],
    edtBoostOffset_Long:         ['temperature', 'Boost Function', 'dBoostOffset[0]'],
    edtBoostOffset_Short:        ['temperature', 'Boost Function', 'dBoostOffset[2]'],
    edtBoostTempMin:             ['temperature', 'LB Temp Function', 'dBoostIdleTime[5]'],
    edtIdleTime_LB:              ['temperature', 'LB Temp Function', 'dBoostIdleTime[3]'],
    edtIdleTime_Long:            ['temperature', 'Boost Function', 'dBoostIdleTime[0]'],
    edtIdleTime_Short:           ['temperature', 'Boost Function', 'dBoostIdleTime[2]'],
    edtInitialDelay_1:           ['tester', 'InitialMode', 'iInitialDelay'],
    edtInitialDelay_2:           ['tester', 'InitialMode', 'iInitialDelay_2'],
    edtInitialDelay_3:           ['tester', 'InitialMode', 'iInitialDelay_3'],
    edtInitialDelay_4:           ['tester', 'InitialMode', 'iInitialDelay_4'],
    edtInitialDelay_5:           ['tester', 'InitialMode', 'iInitialDelay_5'],
    edtInitialDelay_6:           ['tester', 'InitialMode', 'iInitialDelay_6'],
    edtLBTempMin:                ['temperature', 'LB Temp Function', 'dBoostIdleTime[4]'],
    edtLBTempOffset:             ['temperature', 'LB Temp Function', 'dBoostOffset[4]'],
    edtLBTimeOut:                ['temperature', 'LB Temp Function', 'dBoostTimeOut'],
    edtPostBoost_LB:             ['temperature', 'LB Temp Function', 'dPostBoostDuration[3]'],
    edtPostBoost_Long:           ['temperature', 'Boost Function', 'dPostBoostDuration[0]'],
    edtPostBoost_Short:          ['temperature', 'Boost Function', 'dPostBoostDuration[2]'],
    edtTestDock:                 ['temperature', 'Time', 'iUseTesterDocking'],
    edtThreshold:                ['temperature', 'LB Temp Function', 'Threshold'],
  },
  optional: {
    cbbATC_RecipeFile:           ['temperature', 'ATC', 'File Name'],   // 124/215 個配方有
    edATCAmbTemp:                ['temperature', 'ATC', 'ATCAmbientTemperature'],   // 214/215 個配方有
    edATCContFailOffset1:        ['temperature', 'ATC', 'ConsFail_0'],   // 108/215 個配方有
    edATCContFailOffset2:        ['temperature', 'ATC', 'ConsFail_1'],   // 108/215 個配方有
    edATCContFailOffset3:        ['temperature', 'ATC', 'ConsFail_2'],   // 108/215 個配方有
    edATCContFailOffset4:        ['temperature', 'ATC', 'ConsFail_3'],   // 108/215 個配方有
    edATCContFailOffsetCnt:      ['temperature', 'ATC', 'iATCConFailOffsetCount'],   // 108/215 個配方有
    edATCInitialOffset1:         ['temperature', 'ATC', 'Initial_0'],   // 108/215 個配方有
    edATCInitialOffset2:         ['temperature', 'ATC', 'Initial_1'],   // 108/215 個配方有
    edATCInitialOffset3:         ['temperature', 'ATC', 'Initial_2'],   // 108/215 個配方有
    edATCInitialOffset4:         ['temperature', 'ATC', 'Initial_3'],   // 108/215 個配方有
    edATCOfsTime:                ['temperature', 'ATC', 'ATC_OFS_ST'],   // 24/215 個配方有
    edATCQAModeOffset1:          ['temperature', 'ATC', 'ATCQAModeOffset1'],   // 108/215 個配方有
    edATCQAModeOffset2:          ['temperature', 'ATC', 'ATCQAModeOffset2'],   // 108/215 個配方有
    edATCQAModeOffset3:          ['temperature', 'ATC', 'ATCQAModeOffset3'],   // 108/215 個配方有
    edATCQAModeOffset4:          ['temperature', 'ATC', 'ATCQAModeOffset4'],   // 108/215 個配方有
    edATCTestTimeOffset:         ['temperature', 'ATC', 'iATCTestTimeOffsetTime'],   // 108/215 個配方有
    edATCTestTimeOffset1:        ['temperature', 'ATC', 'dATCTestTimeOffset1'],   // 108/215 個配方有
    edATCTestTimeOffset2:        ['temperature', 'ATC', 'dATCTestTimeOffset2'],   // 108/215 個配方有
    edATCTestTimeOffset3:        ['temperature', 'ATC', 'dATCTestTimeOffset3'],   // 108/215 個配方有
    edATCTestTimeOffset4:        ['temperature', 'ATC', 'dATCTestTimeOffset4'],   // 108/215 個配方有
    edAmbGuardband:              ['temperature', 'Ambient', 'iAmbGuardband'],   // 207/215 個配方有
    edAmbHotGuartbent:           ['temperature', 'Ambient', 'fAmbientHotGuartbent'],   // 214/215 個配方有
    edAtcFileName:               ['temperature', 'ATC', 'File Name'],   // 124/215 個配方有
    edChillerTemp:               ['temperature', 'ATC', 'Chiller Temp'],   // 125/215 個配方有
    edContinuousSec:             ['temperature', 'ATC', 'dDelayAfterSOTContinue'],   // 6/215 個配方有
    edDewPointAlarmInterval:     ['temperature', 'Mode', 'DewPointAlarmInterval'],   // 43/215 個配方有
    edDewPointRange:             ['temperature', 'Mode', 'DewPointRange'],   // 43/215 個配方有
    edFixedTemp:                 ['temperature', 'DUT Setting', 'dFixedTemp'],   // 214/215 個配方有
    edIndexSoakTime:             ['temperature', 'Time', 'iIndexSoakTime'],   // 214/215 個配方有
    edLBTempAlarmTime:           ['temperature', 'Mode', 'iLBTempAlmInterval'],   // 40/215 個配方有
    edLBTempHighSettingValue:    ['temperature', 'LB Temp Function', 'LB_Temp_High_Setting'],   // 105/215 個配方有
    edLBTempLowSettingValue:     ['temperature', 'LB Temp Function', 'LB_Temp_Low_Setting'],   // 105/215 個配方有
    edOSTime:                    ['temperature', 'Time', 'iOSTime'],   // 214/215 個配方有
    edTJTempRange_High:          ['temperature', 'ATC', 'TJTempRange_High'],   // 90/215 個配方有
    edTJTempRange_Low:           ['temperature', 'ATC', 'TJTempRange_Low'],   // 90/215 個配方有
    edTSDTimeOut:                ['temperature', 'ATC', 'TSDTimeOut'],   // 107/215 個配方有
    edTempDownContactDelay:      ['temperature', 'InitialMode', 'iCintactDelayCntForInitTempOffset'],   // 214/215 個配方有
    edTempOffsetCount:           ['temperature', 'InitialMode', 'iCintactCntForTempOffsetAtInitial'],   // 214/215 個配方有
    edTempReadyRange:            ['temperature', 'InitialMode', 'iTempReadyRange'],   // 115/215 個配方有
    edt3SigmaTempMonitior_Set3xSigmaValue: ['temperature', 'Sigma', 'SigmaTempMonitior_Set3xSigmaValue'],   // 197/215 個配方有
    edt3SigmaTempMonitior_SetCount: ['temperature', 'Sigma', 'SigmaTempMonitior_SetCount'],   // 197/215 個配方有
    edtATCInPC1:                 ['temperature', 'ATC', 'InPC[0]'],   // 114/215 個配方有
    edtATCInPC2:                 ['temperature', 'ATC', 'InPC[1]'],   // 114/215 個配方有
    edtATCInPC3:                 ['temperature', 'ATC', 'InPC[2]'],   // 114/215 個配方有
    edtATCInPC4:                 ['temperature', 'ATC', 'InPC[3]'],   // 114/215 個配方有
    edtATCSP2:                   ['temperature', 'ATC', 'SP2'],   // 114/215 個配方有
    edtATC_Type:                 ['temperature', 'ATC', 'Type Name'],   // 55/215 個配方有
    edtBoostDuration_Mid:        ['temperature', 'Boost Function', 'dBoostDuration[1]'],   // 18/215 個配方有
    edtBoostOffset_Mid:          ['temperature', 'Boost Function', 'dBoostOffset[1]'],   // 18/215 個配方有
    edtChamberBoostOffset:       ['temperature', 'ChamberBoostMode', 'iChamberBoostOffset'],   // 202/215 個配方有
    edtChamberBoostTime:         ['temperature', 'ChamberBoostMode', 'iChamberBoostTime'],   // 202/215 個配方有
    edtDelayAfterSOT:            ['temperature', 'ATC', 'dDelayAfterSOT'],   // 131/215 個配方有
    edtHeatGunTempATC:           ['temperature', 'Index', 'ATC_HeatGunTemp'],   // 183/215 個配方有
    edtHotGunFLowLimit_H:        ['temperature', 'Mode', 'iHotGunFLowLimit_H'],   // 205/215 個配方有
    edtHotGunFLowLimit_L:        ['temperature', 'Mode', 'iHotGunFLowLimit_L'],   // 205/215 個配方有
    edtIdleTime_Mid:             ['temperature', 'Boost Function', 'dBoostIdleTime[1]'],   // 18/215 個配方有
    edtInitialDelay_10:          ['tester', 'InitialMode', 'dInitialDelay_10'],   // 214/215 個配方有
    edtInitialDelay_10_RT:       ['tester', 'InitialMode', 'dInitialDelay_10_RT'],   // 214/215 個配方有
    edtInitialDelay_1_RT:        ['tester', 'InitialMode', 'dInitialDelay_1_RT'],   // 214/215 個配方有
    edtInitialDelay_2_RT:        ['tester', 'InitialMode', 'dInitialDelay_2_RT'],   // 214/215 個配方有
    edtInitialDelay_3_RT:        ['tester', 'InitialMode', 'dInitialDelay_3_RT'],   // 214/215 個配方有
    edtInitialDelay_4_RT:        ['tester', 'InitialMode', 'dInitialDelay_4_RT'],   // 214/215 個配方有
    edtInitialDelay_5_RT:        ['tester', 'InitialMode', 'dInitialDelay_5_RT'],   // 214/215 個配方有
    edtInitialDelay_6_RT:        ['tester', 'InitialMode', 'dInitialDelay_6_RT'],   // 214/215 個配方有
    edtInitialDelay_7:           ['tester', 'InitialMode', 'iInitialDelay_7'],   // 214/215 個配方有
    edtInitialDelay_7_RT:        ['tester', 'InitialMode', 'dInitialDelay_7_RT'],   // 214/215 個配方有
    edtInitialDelay_8:           ['tester', 'InitialMode', 'iInitialDelay_8'],   // 214/215 個配方有
    edtInitialDelay_8_RT:        ['tester', 'InitialMode', 'dInitialDelay_8_RT'],   // 214/215 個配方有
    edtInitialDelay_9:           ['tester', 'InitialMode', 'iInitialDelay_9'],   // 214/215 個配方有
    edtInitialDelay_9_RT:        ['tester', 'InitialMode', 'dInitialDelay_9_RT'],   // 214/215 個配方有
    edtInputVHigh:               ['temperature', 'ATC', 'dTjInputVHigh'],   // 131/215 個配方有
    edtInputVLow:                ['temperature', 'ATC', 'dTjInputVLow'],   // 131/215 個配方有
    edtLBAirOnTemp:              ['temperature', 'Mode', 'dLBAirOnTemp'],   // 40/215 個配方有
    edtPostBoost_Mid:            ['temperature', 'Boost Function', 'dPostBoostDuration[1]'],   // 18/215 個配方有
    edtPowerFollow_FullPower:    ['temperature', 'ATC', 'iPowerFollow_FullPower'],   // 60/215 個配方有
    edtPowerFollower_Many2one:   ['temperature', 'ATC', 'iPowerFollower_Many2one'],   // 60/215 個配方有
    edtPowerFollower_PFSlope:    ['temperature', 'ATC', 'dPowerFollower_PFSlope'],   // 60/215 個配方有
    edtPowerFollower_WGain:      ['temperature', 'ATC', 'dPowerFollower_WGain'],   // 60/215 個配方有
    edtSetTJ_Offset:             ['temperature', 'ATC', 'dTjOffset'],   // 153/215 個配方有
    edtSetTJ_Slope:              ['temperature', 'ATC', 'dTjSlope'],   // 153/215 個配方有
    edt_TempRiseTemp:            ['temperature', 'ATC', 'dTempRiseTemp'],   // 11/215 個配方有
    edt_TempRise_Delay:          ['temperature', 'ATC', 'dTempRiseDelay'],   // 11/215 個配方有
    edt_TjAvgTimes:              ['temperature', 'ATC', 'Tj Avg Times'],   // 125/215 個配方有
    iTestFinishToNextTestOver:   ['tester', 'InitialMode', 'iTestFinishToNextTestOver'],   // 214/215 個配方有
    iTeststartToNextTestStart:   ['tester', 'InitialMode', 'dTeststartToNextTestStart'],   // 214/215 個配方有
  },
  pending: {
    edAbLow:                     ['?', 'AmbientHotLowOffSet', 'Base', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edAbMid:                     ['?', 'AmbientHotMidOffSet', 'Base', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edSHighBase:                 ['?', 'SHigh OffSet', 'Base', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edTempAlwaysSameAlarm:       ['?', 'Setup', 'iCheckSameTempTime', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edtSetTempature2AirMachine:  ['?', 'InitialMode', 'AirStreamSocket_Offset', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_AirVolumeLmt_Index:      ['?', 'InitialMode', 'iAirVolumeLmt', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_Defrost_Time_Too_Lower:  ['?', 'InitialMode', 'Defrost_Time_Too_Lower', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_SetAirstreamTemperatureRang_Index: ['?', 'InitialMode', 'SetAirstreamTemperatureRang_Index', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_SetAirstreamTemperatureRang_Socket: ['?', 'InitialMode', 'dSetAirstreamTemperatureRang_Socket', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_SetIndexAirstreamTemp:   ['?', 'InitialMode', 'AirStreamIndex_Offset', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    edATCAmbTemp:                ['DOUBLE', 2, true, 19.00, -5.00],
    edATCContFailOffsetCnt:      ['INTEGER', 1, true, 1.0, 30.0],
    edATCOfsTime:                ['INTEGER', 0, true, 0, 60000],
    edATCTestTimeOffset:         ['INTEGER', 0, true, 0, 1000],
    edAbLow:                     ['DOUBLE', 2, false, 0, 0],
    edAbMid:                     ['DOUBLE', 2, false, 0, 0],
    edAbitColdTime:              ['INTEGER', 0, true, 10000, 0],
    edAbitInitWaitTime:          ['INTEGER', 0, true, 10000, 0],
    edAmbGuardband:              ['INTEGER', 2, true, 10, 1],
    edAmbHotGuartbent:           ['INTEGER', 1, true, 1.0, 30.0],
    edAmbTemp:                   ['DOUBLE', 2, true, 50.0, 10.0],   // AI(W906-QFB) 20261002: golden uTemp_Set.cpp:5733 (else of CUSTOMER_CODE==CC_ChipMos_ZHUBEI; was 2, true, 23.0~30.0)
    edArm1Offset:                ['DOUBLE', 2, true, -2.00, 2.00],
    edArm2Offset:                ['DOUBLE', 2, true, -2.00, 2.00],
    edAtcFileName:               ['NO_SYMBOL', 0, false, 0, 0],
    edBelowSec:                  ['DOUBLE', 2, true, 0.0, 3000.0],
    edChamberCoolTemp:           ['DOUBLE', 2, false, 0, 0],
    edChillerTemp:               ['INTEGER', 0, true, 5, 40],
    edDewPointAlarmInterval:     ['INTEGER', 2, true, 1, 30],
    edDewPointRange:             ['DOUBLE', 2, true, 1.0, 30.0],
    edFFC_Arm1TimeOff_01:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_02:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_03:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_04:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_05:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_06:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_07:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_08:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_09:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOff_10:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_01:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_02:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_03:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_04:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_05:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_06:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_07:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_08:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_09:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm1TimeOn_10:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_01:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_02:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_03:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_04:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_05:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_06:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_07:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_08:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_09:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOff_10:        ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_01:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_02:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_03:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_04:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_05:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_06:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_07:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_08:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_09:         ['INTEGER', 0, true, 10000, 0],
    edFFC_Arm2TimeOn_10:         ['INTEGER', 0, true, 10000, 0],
    edFixedTemp:                 ['DOUBLE', 2, false, 0, 0],
    edHighBase:                  ['DOUBLE', 2, false, 0, 0],
    edIndexSoakTime:             ['INTEGER', 0, true, 0, 1000],
    edInitialStart1Time:         ['INTEGER', 0, true, 10000, 0],
    edInitialStart2Time:         ['INTEGER', 0, true, 10000, 0],
    edInitialWaitTime:           ['INTEGER', 0, true, 10000, 0],
    edJamSoakTime:               ['INTEGER', 0, true, 10000, 0],
    edLBTempAlarmTime:           ['INTEGER', 2, true, 30, 300],
    edLBTempHighSettingValue:    ['DOUBLE', 1, true, 0.0, 100.0],
    edLBTempLowSettingValue:     ['DOUBLE', 1, true, 0.0, 100.0],
    edLowBase:                   ['DOUBLE', 2, false, 0, 0],
    edMidBase:                   ['DOUBLE', 2, false, 0, 0],
    edOSTime:                    ['INTEGER', 0, true, 0, 1000],
    edOverSec:                   ['DOUBLE', 2, true, 0.0, 3000.0],
    edSHighBase:                 ['DOUBLE', 2, false, 0, 0],
    edSoakTime:                  ['INTEGER', 0, true, 10000, 0],
    edSocketAirCoolingOff:       ['INTEGER', 0, true, 10000, 0],
    edSocketAirCoolingOn:        ['INTEGER', 0, true, 10000, 0],
    edTJTempRange_High:          ['DOUBLE', 2, true, 20.0, 1.0],
    edTJTempRange_Low:           ['DOUBLE', 2, true, 30.0, 1.0],
    edTempAlwaysSameAlarm:       ['INTEGER', 0, true, 20, 600],
    edTempDownContactDelay:      ['INTEGER', 2, true, 1, 300],
    edTempOffsetCount:           ['INTEGER', 2, true, 0, 30],
    edTempReadyRange:            ['INTEGER', 0, true, 0, 10],
    edWorkTemp:                  ['DOUBLE', 2, false, 0, 0],
    edt3SigmaTempMonitior_SetCount: ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MaxD:        ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MaxI:        ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MaxP:        ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MinD:        ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MinI:        ['INTEGER', 0, true, 99999, 0],
    edtATCPIDOffset_MinP:        ['INTEGER', 0, true, 99999, 0],
    edtATCSP2:                   ['DOUBLE', 2, false, 0, 0],
    edtATC_HotGunTemp:           ['DOUBLE', 2, true, 70.00, 30.00],
    edtATC_HotGunTime:           ['DOUBLE', 2, true, 30.0, 0.0],
    edtATC_PackageTemp_01:       ['DOUBLE', 2, false, 0, 0],
    edtATC_PackageTemp_02:       ['DOUBLE', 2, false, 0, 0],
    edtATC_PackageTemp_03:       ['DOUBLE', 2, false, 0, 0],
    edtBoostDuration_LB:         ['DOUBLE', 2, true, 0.0, 1200.0],
    edtBoostDuration_Long:       ['DOUBLE', 2, true, 0.0, 1200.0],
    edtBoostDuration_Mid:        ['DOUBLE', 2, true, 0.0, 1200.0],
    edtBoostDuration_Short:      ['DOUBLE', 2, true, 0.0, 1200.0],
    edtBoostOffset:              ['DOUBLE', 2, true, -5.0, 5.0],
    edtBoostOffset_LB:           ['DOUBLE', 2, true, -30.0, 30.0],
    edtBoostOffset_Long:         ['DOUBLE', 2, true, -30.0, 30.0],
    edtBoostOffset_Mid:          ['DOUBLE', 2, true, -30.0, 30.0],
    edtBoostOffset_Short:        ['DOUBLE', 2, true, -30.0, 30.0],
    edtBoostTempMin:             ['DOUBLE', 1, true, 0.0, 100.0],
    edtChamberBoostOffset:       ['INTEGER', 2, true, 0, 30],
    edtChamberBoostTime:         ['INTEGER', 2, true, 1, 30],
    edtHeatGunTempATC:           ['DOUBLE', 2, true, 70.00, 30.00],
    edtHotGunFLowLimit_H:        ['INTEGER', 0, true, 0, 1000],
    edtHotGunFLowLimit_L:        ['INTEGER', 0, true, 0, 1000],
    edtIdleTime_LB:              ['DOUBLE', 2, true, 0.0, 1200.0],
    edtIdleTime_Long:            ['DOUBLE', 2, true, 0.0, 1200.0],
    edtIdleTime_Mid:             ['DOUBLE', 2, true, 0.0, 1200.0],
    edtIdleTime_Short:           ['DOUBLE', 2, true, 0.0, 1200.0],
    edtInitialDelay_1:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_10:          ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_10_RT:       ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_1_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_2:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_2_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_3:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_3_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_4:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_4_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_5:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_5_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_6:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_6_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_7:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_7_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_8:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_8_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_9:           ['DOUBLE', 2, true, 0.0, 3000.0],
    edtInitialDelay_9_RT:        ['DOUBLE', 2, true, 0.0, 3000.0],
    edtLBAirOnTemp:              ['DOUBLE', 2, true, 40.0, 100.0],
    edtLBTempMin:                ['DOUBLE', 1, true, 0.0, 100.0],
    edtLBTempOffset:             ['DOUBLE', 2, true, -30.0, 30.0],
    edtLBTimeOut:                ['DOUBLE', 2, true, 300.0, 1200.0],
    edtPostBoost_LB:             ['DOUBLE', 2, true, 0.0, 1200.0],
    edtPostBoost_Long:           ['DOUBLE', 2, true, 0.0, 1200.0],
    edtPostBoost_Mid:            ['DOUBLE', 2, true, 0.0, 1200.0],
    edtPostBoost_Short:          ['DOUBLE', 2, true, 0.0, 1200.0],
    edtSetTempature2AirMachine:  ['INTEGER', 0, true, 20.0, -20.0],
    edtTestDock:                 ['INTEGER', 2, true, 0, 30],
    edtThreshold:                ['DOUBLE', 2, true, 0.0, 30.0],
    edt_AirVolumeLmt_Index:      ['DOUBLE', 2, true, 1000.00, 350.00],
    edt_AirVolumeLmt_Socket:     ['DOUBLE', 2, true, 1000.00, 350.00],
    edt_Defrost_Time_Too_Lower:  ['DOUBLE', 2, true, 1200.00, 600.00],
    edt_SetAirstreamTemperatureRang_Index: ['INTEGER', 0, true, 20.0, -20.0],
    edt_SetAirstreamTemperatureRang_Socket: ['INTEGER', 0, true, 20.0, -20.0],
    edt_SetIndexAirstreamTemp:   ['INTEGER', 0, true, 20.0, -20.0],
    edt_TjAvgTimes:              ['INTEGER', 2, true, 0, 30],
    iTestFinishToNextTestOver:   ['DOUBLE', 2, true, 0.0, 3000.0],
    iTeststartToNextTestStart:   ['DOUBLE', 2, true, 0.0, 3000.0],
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 10 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edAbLow                  [AmbientHotLowOffSet] Base
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edAbMid                  [AmbientHotMidOffSet] Base
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edSHighBase              [SHigh OffSet] Base
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edTempAlwaysSameAlarm    [Setup] iCheckSameTempTime
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edtSetTempature2AirMachine [InitialMode] AirStreamSocket_Offset
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_AirVolumeLmt_Index   [InitialMode] iAirVolumeLmt
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_Defrost_Time_Too_Lower [InitialMode] Defrost_Time_Too_Lower
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_SetAirstreamTemperatureRang_Index [InitialMode] SetAirstreamTemperatureRang_Index
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_SetAirstreamTemperatureRang_Socket [InitialMode] dSetAirstreamTemperatureRang_Socket
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_SetIndexAirstreamTemp [InitialMode] AirStreamIndex_Offset
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
