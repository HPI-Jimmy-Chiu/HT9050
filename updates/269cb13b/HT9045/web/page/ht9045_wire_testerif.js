/* ht9045_wire_testerif.js -- Setup.TesterIF.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260915  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cTesterIF.dfm
 * 抽取法      WriteIniData（widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             兩者都限定在該表單自己的 .cpp，不做全樹 id 反查。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 15 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 47 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 59 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'Setup.TesterIF.html',
  slug: 'testerif',
  fields: {
    cbDIOType:                   ['tester', 'DIO', 'TypeName'],
    cbbBaudRate:                 ['tester', 'RS-232C', 'BaudRate'],
    edBelowSec:                  ['tester', 'InitialMode', 'iEveryFirstDeviceUseInitialDelay'],
    edDummyTestTime:             ['tester', 'Time', 'Dummy Time'],
    edGPIBAddress:               ['tester', 'GP-IB', 'Address'],
    edInitialMaxTest:            ['tester', 'Time', 'Initial MAX Time'],
    edMaxTestTime:               ['tester', 'Time', 'MAX Time'],
    edOverSec:                   ['tester', 'InitialMode', 'iWhenPressStopOver'],
    edStartDelayTime:            ['tester', 'Time', 'Stary Delay'],
    edtInitialDelay_1:           ['tester', 'InitialMode', 'iInitialDelay'],
    edtInitialDelay_2:           ['tester', 'InitialMode', 'iInitialDelay_2'],
    edtInitialDelay_3:           ['tester', 'InitialMode', 'iInitialDelay_3'],
    edtInitialDelay_4:           ['tester', 'InitialMode', 'iInitialDelay_4'],
    edtInitialDelay_5:           ['tester', 'InitialMode', 'iInitialDelay_5'],
    edtInitialDelay_6:           ['tester', 'InitialMode', 'iInitialDelay_6'],
  },
  optional: {
    edDummyTestTime_EQC:         ['tester', 'Time', 'EQC Dummy Time'],   // 180/215 個配方有
    edDummyTestTime_RT:          ['tester', 'Time', 'RT Dummy Time'],   // 180/215 個配方有
    edInitialMaxTest_EQC:        ['tester', 'Time', 'EQC Initial MAX Time'],   // 180/215 個配方有
    edInitialMaxTest_RT:         ['tester', 'Time', 'RT Initial MAX Time'],   // 180/215 個配方有
    edMaxBinCount:               ['tester', 'RS-232C', 'Bin Count'],   // 214/215 個配方有
    edMaxTestTime_EQC:           ['tester', 'Time', 'EQC MAX Time'],   // 180/215 個配方有
    edMaxTestTime_RT:            ['tester', 'Time', 'RT MAX Time'],   // 180/215 個配方有
    edPurgeAir:                  ['tester', 'InitialMode', 'iPurgeAir'],   // 212/215 個配方有
    edStartDelayTime_EQC:        ['tester', 'Time', 'EQC Stary Delay'],   // 180/215 個配方有
    edStartDelayTime_RT:         ['tester', 'Time', 'RT Stary Delay'],   // 180/215 個配方有
    edTCPIP_Address:             ['tester', 'Tester TCPIP', 'Address'],   // 191/215 個配方有
    edTCPIP_Port:                ['tester', 'Tester TCPIP', 'Port'],   // 191/215 個配方有
    edtAfterTestedDelay:         ['tester', 'Time', 'After Tested Delay'],   // 214/215 個配方有
    edtAutoOnecycleHomStartTime: ['tester', 'Time', 'iAutoOnecycleHomStartTime'],   // 3/215 個配方有
    edtInitStartDelay:           ['tester', 'Time', 'Initial Stary Delay'],   // 214/215 個配方有
    edtInitStartDelayCT:         ['tester', 'Time', 'Initial Stary Delay CT'],   // 214/215 個配方有
    edtInitStartDelayCT_EQC:     ['tester', 'Time', 'EQC Initial Stary Delay CT'],   // 180/215 個配方有
    edtInitStartDelayCT_RT:      ['tester', 'Time', 'RT Initial Stary Delay CT'],   // 180/215 個配方有
    edtInitStartDelay_EQC:       ['tester', 'Time', 'EQC Initial Stary Delay'],   // 180/215 個配方有
    edtInitStartDelay_RT:        ['tester', 'Time', 'RT Initial Stary Delay'],   // 180/215 個配方有
    edtInitWaitTime:             ['tester', 'Time', 'Init Wait Time'],   // 214/215 個配方有
    edtInitialDec1:              ['tester', 'InitialMode', 'dInitialStartDelayDec1'],   // 212/215 個配方有
    edtInitialDec2:              ['tester', 'InitialMode', 'dInitialStartDelayDec2'],   // 212/215 個配方有
    edtInitialDec3:              ['tester', 'InitialMode', 'dInitialStartDelayDec3'],   // 212/215 個配方有
    edtInitialDec4:              ['tester', 'InitialMode', 'dInitialStartDelayDec4'],   // 212/215 個配方有
    edtInitialDecCount1:         ['tester', 'InitialMode', 'iStartDelayCount1'],   // 212/215 個配方有
    edtInitialDecCount2:         ['tester', 'InitialMode', 'iStartDelayCount2'],   // 212/215 個配方有
    edtInitialDecCount3:         ['tester', 'InitialMode', 'iStartDelayCount3'],   // 212/215 個配方有
    edtInitialDecCount4:         ['tester', 'InitialMode', 'iStartDelayCount4'],   // 212/215 個配方有
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
    edtTestingWaitTime:          ['tester', 'Time', 'Testing Wait Time'],   // 214/215 個配方有
    edt_UseSocketHeating:        ['tester', 'Time', 'iUseSocketHeating'],   // 26/215 個配方有
    iTestFinishToNextTestOver:   ['tester', 'InitialMode', 'iTestFinishToNextTestOver'],   // 214/215 個配方有
    iTeststartToNextTestStart:   ['tester', 'InitialMode', 'dTeststartToNextTestStart'],   // 214/215 個配方有
  },
  pending: {
  },
  kb: {
    edBelowSec:                  ['DOUBLE', 2, true, 0.0, 3000.0],
    edDummyTestTime:             ['DOUBLE', 0, true, 0.0, 5000.0],
    edDummyTestTime_EQC:         ['DOUBLE', 0, true, 0.0, 5000.0],
    edDummyTestTime_RT:          ['DOUBLE', 0, true, 0.0, 5000.0],
    edGPIBAddress:               ['INTEGER', 0, true, 64, 1],
    edInitialMaxTest:            ['INTEGER', 0, true, 60, 9999],
    edInitialMaxTest_EQC:        ['INTEGER', 0, true, 60, 9999],
    edInitialMaxTest_RT:         ['INTEGER', 0, true, 60, 9999],
    edMaxBinCount:               ['INTEGER', 0, true, 15, 255],
    edMaxTestTime:               ['INTEGER', 0, true, 60, 9999],
    edMaxTestTime_EQC:           ['INTEGER', 0, true, 60, 9999],
    edMaxTestTime_RT:            ['INTEGER', 0, true, 60, 9999],
    edOverSec:                   ['DOUBLE', 2, true, 0.0, 3000.0],
    edPurgeAir:                  ['INTEGER', 0, true, 0, 10000],
    edStartDelayTime:            ['DOUBLE', 0, true, 0.0, 5000.0],
    edStartDelayTime_EQC:        ['DOUBLE', 0, true, 0.0, 5000.0],
    edStartDelayTime_RT:         ['DOUBLE', 0, true, 0.0, 5000.0],
    edTCPIP_Port:                ['PORT', 0, false, 0, 0],
    edtAfterTestedDelay:         ['DOUBLE', 0, true, 0.0, 5000.0],
    edtAutoOnecycleHomStartTime: ['INTEGER', 0, true, 10000, 10],
    edtInitStartDelay:           ['DOUBLE', 2, false, 0, 0],   // AI(W906-QFB) 20261002: golden cTesterIF.cpp:1415 (else of CosFunction.bHiSiliconFunction&&CUSTOMER_CODE==CC_ASE_KaohSi; was 2, true, 30.0~3000.0)
    edtInitStartDelayCT:         ['INTEGER', 0, true, 0, 3000],
    edtInitStartDelayCT_EQC:     ['INTEGER', 0, true, 0, 3000],
    edtInitStartDelayCT_RT:      ['INTEGER', 0, true, 0, 3000],
    edtInitStartDelay_EQC:       ['DOUBLE', 2, false, 0, 0],   // AI(W906-QFB) 20261002: golden cTesterIF.cpp:1415 (else of CosFunction.bHiSiliconFunction&&CUSTOMER_CODE==CC_ASE_KaohSi; was 2, true, 30.0~3000.0)
    edtInitStartDelay_RT:        ['DOUBLE', 2, false, 0, 0],   // AI(W906-QFB) 20261002: golden cTesterIF.cpp:1415 (else of CosFunction.bHiSiliconFunction&&CUSTOMER_CODE==CC_ASE_KaohSi; was 2, true, 30.0~3000.0)
    edtInitWaitTime:             ['DOUBLE', 0, true, 0.0, 5000.0],
    edtInitialDec1:              ['DOUBLE', 0, true, 0.0, 5000.0],
    edtInitialDec2:              ['DOUBLE', 0, true, 0.0, 5000.0],
    edtInitialDec3:              ['DOUBLE', 0, true, 0.0, 5000.0],
    edtInitialDec4:              ['DOUBLE', 0, true, 0.0, 5000.0],
    edtInitialDecCount1:         ['INTEGER', 0, true, 0, 3000],
    edtInitialDecCount2:         ['INTEGER', 0, true, 0, 3000],
    edtInitialDecCount3:         ['INTEGER', 0, true, 0, 3000],
    edtInitialDecCount4:         ['INTEGER', 0, true, 0, 3000],
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
    edtTestingWaitTime:          ['DOUBLE', 0, true, 0.0, 5000.0],
    edt_UseSocketHeating:        ['DOUBLE', 2, true, 0.0, 3000.0],
    iTestFinishToNextTestOver:   ['DOUBLE', 2, true, 0.0, 3000.0],
    iTeststartToNextTestStart:   ['DOUBLE', 2, true, 0.0, 3000.0],
  }
});
