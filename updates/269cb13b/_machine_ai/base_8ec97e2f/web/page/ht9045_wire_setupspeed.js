/* ht9045_wire_setupspeed.js -- Setup.Speed.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cSpeed.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 65 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 23 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 62 個欄位有 golden 依據
 * ⚠ 其中 12 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.Speed.html',
  slug: 'setupspeed',
  fields: {
    Edt_CheckTime_In:            ['armCondition', 'Input Arm', 'Destroy Check Time'],
    Edt_CheckTime_Out:           ['armCondition', 'Output Arm', 'Destroy Check Time'],
    Edt_HeightCheck_In:          ['armCondition', 'Input Arm', 'Height Check'],
    edAllAccSpeed:               ['armCondition', 'All', 'Accel'],
    edAllSpeed:                  ['armCondition', 'All', 'Speed'],
    edEPControl:                 ['armCondition', 'All', 'EPControl'],
    edInArmAirOnTime:            ['armCondition', 'Input Arm', 'Counter Air ON Time'],
    edInArmDestroyAgainCount:    ['armCondition', 'Input Arm', 'Destroy Again Count'],
    edInArmDestroyAgainTime:     ['armCondition', 'Input Arm', 'Destroy Again Time'],
    edInArmRetryCount:           ['armCondition', 'Input Arm', 'Retry Count'],
    edInArmRetryMM:              ['armCondition', 'Input Arm', 'Retry Down'],
    edInArmShtWaitTime:          ['armCondition', 'Input Arm', 'Shuttle Wait Time'],
    edInPitchAcc:                ['armCondition', 'Input Arm', 'Open/Close Accel'],
    edInPitchSpd:                ['armCondition', 'Input Arm', 'Open/Close Speed'],
    edInRotAcc:                  ['armCondition', 'Input Arm', 'Rotate Input Accel'],
    edInRotSpd:                  ['armCondition', 'Input Arm', 'Rotate Input Speed'],
    edInVacumCheckTime:          ['armCondition', 'Input Arm', 'Vacuum Check Time'],
    edInXYAcc:                   ['armCondition', 'Input Arm', 'XY Accel'],
    edInXYSpd:                   ['armCondition', 'Input Arm', 'XY Speed'],
    edInZAcc:                    ['armCondition', 'Input Arm', 'Z Down Accel'],
    edInZSpd:                    ['armCondition', 'Input Arm', 'Z Down Speed'],
    edIndexAccDec:               ['armCondition', 'Index Arm', 'Z Down Accel'],
    edIndexAirOnTime:            ['armCondition', 'Index Arm', 'Counter Air ON Time'],
    edIndexArmRetryMM:           ['armCondition', 'Index Arm', 'Retry Down'],
    edIndexDestroyAgainCount:    ['armCondition', 'Index Arm', 'Destroy Again Count'],
    edIndexDestroyAgainTime:     ['armCondition', 'Index Arm', 'Destroy Again Time'],
    edIndexRetryCount:           ['armCondition', 'Index Arm', 'Retry Count'],
    edIndexSpeed:                ['armCondition', 'Index Arm', 'Z Down Speed'],
    edIndexVacumCheckTime:       ['armCondition', 'Index Arm', 'Vacuum Check Time'],
    edOutArmAirOnTime:           ['armCondition', 'Output Arm', 'Counter Air ON Time'],
    edOutArmDestroyAgainCount:   ['armCondition', 'Output Arm', 'Destroy Again Count'],
    edOutArmDestroyAgainTime:    ['armCondition', 'Output Arm', 'Destroy Again Time'],
    edOutArmRetryCount:          ['armCondition', 'Output Arm', 'Retry Count'],
    edOutArmRetryMM:             ['armCondition', 'Output Arm', 'Retry Down'],
    edOutPitchAcc:               ['armCondition', 'Output Arm', 'Open/Close Accel'],
    edOutPitchSpd:               ['armCondition', 'Output Arm', 'Open/Close Speed'],
    edOutRotAcc:                 ['armCondition', 'Output Arm', 'Rotate Output Accel'],
    edOutRotSpd:                 ['armCondition', 'Output Arm', 'Rotate Output Speed'],
    edOutVacumCheckTime:         ['armCondition', 'Output Arm', 'Vacuum Check Time'],
    edOutXAcc:                   ['armCondition', 'Output Arm', 'XY Accel'],
    edOutXSpd:                   ['armCondition', 'Output Arm', 'XY Speed'],
    edOutZAcc:                   ['armCondition', 'Output Arm', 'Z Down Accel'],
    edOutZSpd:                   ['armCondition', 'Output Arm', 'Z Down Speed'],
    edRelaseDelay:               ['armCondition', 'Input Arm', 'Release Delay Time'],
    edSecondADCIn:               ['armCondition', 'Input Arm', 'Two Speed ADC'],
    edSecondADCOut:              ['armCondition', 'Output Arm', 'Two Speed ADC'],
    edSecondSpeedIn:             ['armCondition', 'Input Arm', 'Two Speed Precent'],
    edSecondSpeedOut:            ['armCondition', 'Output Arm', 'Two Speed Precent'],
    edSht1Acc:                   ['armCondition', 'Shuttle', 'Shuttle 1 Accel'],
    edSht1Spd:                   ['armCondition', 'Shuttle', 'Shuttle 1 Speed'],
    edShtDeviceCheckTime:        ['armCondition', 'Shuttle', 'Device Check Time'],
    edTrayArmAirOnTime:          ['armCondition', 'Empty Tray Arm', 'Counter Air ON Time'],
    edTrayArmHandDown:           ['armCondition', 'Empty Tray Arm', 'Hand Down Time'],
    edTrayArmRetryCount:         ['armCondition', 'Empty Tray Arm', 'Retry Count'],
    edTrayVacumCheckTime:        ['armCondition', 'Empty Tray Arm', 'Vacuum Check Time'],
    edTrayXAcc:                  ['armCondition', 'Empty Tray Arm', 'Accel'],
    edTrayXSpd:                  ['armCondition', 'Empty Tray Arm', 'Speed'],
    edTwoSpeedDistance_In:       ['armCondition', 'Input Arm', 'Two Speed Distance'],
    edTwoSpeedDistance_Out:      ['armCondition', 'Output Arm', 'Two Speed Distance'],
    edtAutoSkipCT:               ['armCondition', 'Input Arm', 'Auto Skip CT'],
    edtHPVacuumDelay:            ['armCondition', 'Input Arm', 'HP Vacuum Check Time'],
    edtMonitoringIndexCycletime: ['armCondition', 'Index Arm', 'Monitoring_IndexCycletime'],
    edtMonitoringOutlier:        ['armCondition', 'Index Arm', 'Monitoring_Outlier'],
    edtMonitoringWindow:         ['armCondition', 'Index Arm', 'Monitoring_Window'],
    edtOutArmPreSuck:            ['armCondition', 'Output Arm', 'Wait before Air On'],
  },
  optional: {
    edAutoSpeedLow:              ['armCondition', 'Output Arm', 'AutoSpeedLow'],   // 208/215 個配方有
    edInArmCylinderDelay:        ['armCondition', 'Input Arm', 'Cylinder Delay'],   // 172/215 個配方有
    edInArmDieCleanDelay:        ['armCondition', 'Input Arm', 'Die Clean Delay'],   // 3/215 個配方有
    edInArmDieCleanHeight:       ['armCondition', 'Input Arm', 'Die Clean Height'],   // 3/215 個配方有
    edMagCatchYAcc:              ['armCondition', 'Magazine', 'Magazine CatchY Accel'],   // 177/215 個配方有
    edMagCatchYSpd:              ['armCondition', 'Magazine', 'Magazine CatchY Speed'],   // 177/215 個配方有
    edMagZAcc:                   ['armCondition', 'Magazine', 'MagazineZ Accel'],   // 177/215 個配方有
    edMagZSpd:                   ['armCondition', 'Magazine', 'MagazineZ Speed'],   // 177/215 個配方有
    edOutArmCylinderDelay:       ['armCondition', 'Output Arm', 'Cylinder Delay'],   // 172/215 個配方有
    edOutArmDieCleanDelay:       ['armCondition', 'Input Arm', 'Die Clean Delay'],   // 3/215 個配方有
    edOutArmShtWaitTime:         ['armCondition', 'Output Arm', 'Shuttle Wait Time'],   // 210/215 個配方有
    edPrecisorCloseSp:           ['armCondition', 'Input Arm', 'PrecisorCloseSpeed'],   // 2/215 個配方有
    edPrecisorOpenSp:            ['armCondition', 'Input Arm', 'PrecisorOpenSpeed'],   // 2/215 個配方有
    edSecondADCCatchY:           ['armCondition', 'Magazine', 'Two Speed ADC'],   // 177/215 個配方有
    edSecondSpeedCatchY:         ['armCondition', 'Magazine', 'Two Speed Precent'],   // 177/215 個配方有
    edShakeAccDec:               ['armCondition', 'Shuttle', 'Shake Accel'],   // 6/215 個配方有
    edShakeCycles:               ['armCondition', 'Shuttle', 'Shake Cycles'],   // 6/215 個配方有
    edShakeDelay:                ['armCondition', 'Shuttle', 'Delay between shakes'],   // 6/215 個配方有
    edShakeDistance:             ['armCondition', 'Shuttle', 'Shake distance'],   // 6/215 個配方有
    edSht2Acc:                   ['armCondition', 'Shuttle', 'Shuttle Sort Accel'],   // 2/215 個配方有
    edSht2Spd:                   ['armCondition', 'Shuttle', 'Shuttle Sort Speed'],   // 2/215 個配方有
    edTwoSpeedDistanceCatchY:    ['armCondition', 'Magazine', 'Two Speed Distance'],   // 177/215 個配方有
    edtIndexCycleTimetolerance:  ['armCondition', 'Index Arm', 'IndexCycleTimetolerance'],   // 199/215 個配方有
  },
  pending: {
  },
  kb: {
    Edt_CheckTime_In:            ['DOUBLE', 2, true, 1.0, 0.1],
    Edt_CheckTime_Out:           ['DOUBLE', 2, true, 1.0, 0.1],
    Edt_HeightCheck_In:          ['DOUBLE', 2, true, 1500, 500],
    edAutoSpeedLow:              ['INTEGER', 0, true, 100, 1],
    edInArmAirOnTime:            ['DOUBLE', 2, true, 10.0, 0.01],
    edInArmCylinderDelay:        ['DOUBLE', 2, true, 10.0, 0.1],
    edInArmDestroyAgainCount:    ['INTEGER', 2, false, 0, 0],
    edInArmDestroyAgainTime:     ['DOUBLE', 2, true, 10.0, 0.01],
    edInArmDieCleanDelay:        ['DOUBLE', 2, true, 10.0, 0.1],
    edInArmDieCleanHeight:       ['DOUBLE', 2, true, 20.0, 0.1],
    edInArmRetryCount:           ['INTEGER', 2, false, 0, 0],
    edInArmRetryMM:              ['DOUBLE', 2, false, 0, 0],
    edInArmShtWaitTime:          ['DOUBLE', 2, true, 10.0, 0.01],
    edInPitchAcc:                ['INTEGER', 0, true, 100, 1],
    edInVacumCheckTime:          ['DOUBLE', 2, true, 10.0, 0.01],
    edInXYAcc:                   ['INTEGER', 0, true, 100, 1],
    edInZAcc:                    ['INTEGER', 0, true, 100, 1],
    edIndexAccDec:               ['INTEGER', 0, true, 100, 1],
    edIndexAirOnTime:            ['DOUBLE', 2, true, 10.0, 0.01],
    edIndexArmRetryMM:           ['DOUBLE', 2, false, 0, 0],
    edIndexDestroyAgainCount:    ['INTEGER', 2, false, 0, 0],
    edIndexDestroyAgainTime:     ['DOUBLE', 2, true, 10.0, 0.01],
    edIndexRetryCount:           ['INTEGER', 2, false, 0, 0],
    edIndexVacumCheckTime:       ['DOUBLE', 2, true, 10.0, 0.01],
    edMagCatchYAcc:              ['INTEGER', 0, true, 100, 1],
    edMagZAcc:                   ['INTEGER', 0, true, 100, 1],
    edOutArmAirOnTime:           ['DOUBLE', 2, true, 10.0, 0.01],
    edOutArmCylinderDelay:       ['DOUBLE', 2, true, 10.0, 0.1],
    edOutArmDestroyAgainCount:   ['INTEGER', 2, false, 0, 0],
    edOutArmDestroyAgainTime:    ['DOUBLE', 2, true, 10.0, 0.01],
    edOutArmDieCleanDelay:       ['DOUBLE', 2, true, 10.0, 0.1],
    edOutArmRetryCount:          ['INTEGER', 2, false, 0, 0],
    edOutArmRetryMM:             ['DOUBLE', 2, false, 0, 0],
    edOutArmShtWaitTime:         ['DOUBLE', 2, false, 0, 0],
    edOutPitchAcc:               ['INTEGER', 0, true, 100, 1],
    edOutRotAcc:                 ['INTEGER', 0, true, 100, 1],
    edOutVacumCheckTime:         ['DOUBLE', 2, true, 10.0, 0.01],
    edOutXAcc:                   ['INTEGER', 0, true, 100, 1],
    edOutZAcc:                   ['INTEGER', 0, true, 100, 1],
    edRelaseDelay:               ['DOUBLE', 2, true, 3.0, 0.1],
    edShakeAccDec:               ['INTEGER', 100, true, 100, 1],
    edShakeCycles:               ['INTEGER', 1, true, 100, 1],
    edShakeDelay:                ['DOUBLE', 0.0, false, 0, 0],
    edShakeDistance:             ['INTEGER', 5, true, 30, 5],
    edSht1Acc:                   ['INTEGER', 0, true, 100, 1],
    edSht2Acc:                   ['INTEGER', 0, true, 100, 1],
    edShtDeviceCheckTime:        ['DOUBLE', 2, true, 10.0, 0.01],
    edTrayArmAirOnTime:          ['DOUBLE', 2, true, 10.0, 0.01],
    edTrayArmHandDown:           ['DOUBLE', 2, true, 10.0, 0.01],
    edTrayArmRetryCount:         ['INTEGER', 2, false, 0, 0],
    edTrayVacumCheckTime:        ['DOUBLE', 2, true, 10.0, 0.01],
    edTrayXAcc:                  ['INTEGER', 0, true, 100, 1],
    edTwoSpeedDistance_In:       ['DOUBLE', 2, true, 10.0, 0.01],
    edTwoSpeedDistance_Out:      ['DOUBLE', 2, true, 10.0, 0.01],
    edtAutoSkipCT:               ['INTEGER', 0, true, 1000, 5],
    edtHPVacuumDelay:            ['DOUBLE', 2, true, 10.0, 0.01],
    edtIndexCycleTimetolerance:  ['INTEGER', 15, true, 5, 200],
    edtMonitoringIndexCycletime: ['DOUBLE', 2, true, 0.01, 10.0],
    edtMonitoringOutlier:        ['DOUBLE', 2, true, 0.01, 30.0],
    edtMonitoringWindow:         ['INTEGER', 0, true, 500, 1],
    edtOutArmPreSuck:            ['DOUBLE', 2, true, 10.0, 0.01],
    edtTryAcc:                   ['INTEGER', 0, true, 100, 1],
  }
});
