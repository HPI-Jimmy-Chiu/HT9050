/* ht9045_wire_setupcleaning.js -- Setup.Cleaning.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    uCleaning.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 1 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 65 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 9 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 48 個欄位有 golden 依據
 * ⚠ 其中 7 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.Cleaning.html',
  slug: 'setupcleaning',
  fields: {
    XCT1:                        ['hotPlate', 'Hotplate Form', 'X Division'],
  },
  optional: {
    IndexArmSpeed:               ['handlerCondition', 'Configuration', 'iAutoClean_MotorSpeed[2]'],   // 215/216 個配方有
    OutArmSpeed:                 ['handlerCondition', 'Configuration', 'iAutoClean_MotorSpeed[0]'],   // 215/216 個配方有
    PageTypeName:                ['handlerCondition', 'Configuration', 'cAutoClean_PackageTray'],   // 184/216 個配方有
    ShuttleSpeed:                ['handlerCondition', 'Configuration', 'iAutoClean_MotorSpeed[1]'],   // 215/216 個配方有
    XCT2:                        ['handlerCondition', 'Configuration', 'iAutoClean_XDivision_Tray'],   // 205/216 個配方有
    XPitch1:                     ['handlerCondition', 'Configuration', 'dAutoClean_XPitch_Kit'],   // 205/216 個配方有
    XPitch2:                     ['handlerCondition', 'Configuration', 'dAutoClean_XPitch_Tray'],   // 205/216 個配方有
    XST1:                        ['handlerCondition', 'Configuration', 'dAutoClean_XStart_Kit'],   // 205/216 個配方有
    XST2:                        ['handlerCondition', 'Configuration', 'dAutoClean_XStart_Tray'],   // 205/216 個配方有
    YCT1:                        ['handlerCondition', 'Configuration', 'iAutoClean_YDivision_Kit'],   // 205/216 個配方有
    YCT2:                        ['handlerCondition', 'Configuration', 'iAutoClean_YDivision_Tray'],   // 205/216 個配方有
    YPitch1:                     ['handlerCondition', 'Configuration', 'dAutoClean_YPitch_Kit'],   // 205/216 個配方有
    YPitch2:                     ['handlerCondition', 'Configuration', 'dAutoClean_YPitch_Tray'],   // 205/216 個配方有
    YST1:                        ['handlerCondition', 'Configuration', 'dAutoClean_YStart_Kit'],   // 205/216 個配方有
    YST2:                        ['handlerCondition', 'Configuration', 'dAutoClean_YStart_Tray'],   // 205/216 個配方有
    cbbCleanPadCount:            ['handlerCondition', 'Configuration', 'iAutoClean_DeveicePices'],   // 215/216 個配方有
    edACContactCleanHeight:      ['handlerCondition', 'Configuration', 'iAutoClean_ContactCleanHeight'],   // 215/216 個配方有
    edACContactCount:            ['handlerCondition', 'Configuration', 'iAutoClean_ContactCount'],   // 215/216 個配方有
    edACContactShiftHeight:      ['handlerCondition', 'Configuration', 'iAutoClean_ContactShiftHeight'],   // 215/216 個配方有
    edAlarmCount:                ['handlerCondition', 'Configuration', 'iAutoClean_AlarmCount'],   // 215/216 個配方有
    edAutoCleanAirForce_Kg:      ['handlerCondition', 'Configuration', 'fAutoClean_AireForce'],   // 215/216 個配方有
    edCleanPadDeviation:         ['handlerCondition', 'Configuration', 'iAutoClean_iPadThickness'],   // 180/216 個配方有
    edContactTime:               ['handlerCondition', 'Configuration', 'iAutoClean_ContactTime'],   // 215/216 個配方有
    edDropOffset1:               ['handlerCondition', 'Configuration', 'iAutoClean_DropHigh'],   // 180/216 個配方有
    edIndexPickOffset:           ['handlerCondition', 'Configuration', 'iAutoClean_IndexPickOffset'],   // 189/216 個配方有
    edIndexReleaseOffset:        ['handlerCondition', 'Configuration', 'iAutoClean_IndexReleaseOffset'],   // 189/216 個配方有
    edIntervalContact:           ['handlerCondition', 'Configuration', 'iAutoClean_IntervalContact'],   // 215/216 個配方有
    edPinSingleGf:               ['handlerCondition', 'Configuration', 'fAutoClean_DevicePinForceGf'],   // 215/216 個配方有
    edPinsCount:                 ['handlerCondition', 'Configuration', 'iAutoClean_DevicePinCount'],   // 215/216 個配方有
    edShuttle1PickOffset:        ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle1PickOffset'],   // 215/216 個配方有
    edShuttle2PickOffset:        ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle2PickOffset'],   // 189/216 個配方有
    edtAutoCleanDieForce:        ['handlerCondition', 'Configuration', 'fAutoClean_DieForce'],   // 17/216 個配方有
    edtBufferKitLDX:             ['handlerCondition', 'Configuration', 'dBufferKitLDX'],   // 205/216 個配方有
    edtBufferKitLDY:             ['handlerCondition', 'Configuration', 'dBufferKitLDY'],   // 205/216 個配方有
    edtBufferKitLTX:             ['handlerCondition', 'Configuration', 'dBufferKitLTX'],   // 205/216 個配方有
    edtBufferKitLTY:             ['handlerCondition', 'Configuration', 'dBufferKitLTY'],   // 205/216 個配方有
    edtBufferKitRDX:             ['handlerCondition', 'Configuration', 'dBufferKitRDX'],   // 205/216 個配方有
    edtBufferKitRDY:             ['handlerCondition', 'Configuration', 'dBufferKitRDY'],   // 205/216 個配方有
    edtBufferKitRTX:             ['handlerCondition', 'Configuration', 'dBufferKitRTX'],   // 205/216 個配方有
    edtBufferKitRTY:             ['handlerCondition', 'Configuration', 'dBufferKitRTY'],   // 205/216 個配方有
    edtConseFailureCountByHead_Normal: ['handlerCondition', 'Configuration', 'iAutoClean_ConseFailureCountByHead_Normal'],   // 184/216 個配方有
    edtConseFailureCountByHead_Retest: ['handlerCondition', 'Configuration', 'iAutoClean_ConseFailureCountByHead_Retest'],   // 184/216 個配方有
    edtConseFailureCountBySocket_Normal: ['handlerCondition', 'Configuration', 'iAutoClean_ConseFailureCountBySocket_Normal'],   // 184/216 個配方有
    edtConseFailureCountBySocket_Retest: ['handlerCondition', 'Configuration', 'iAutoClean_ConseFailureCountBySocket_Retest'],   // 184/216 個配方有
    edtFailAlarmSiteYield:       ['handlerCondition', 'Configuration', 'iAutoClean_FailAlarmSiteYield'],   // 184/216 個配方有
    edtFailAlarmSiteYieldDifferent: ['handlerCondition', 'Configuration', 'iAutoClean_FailAlarmSiteYieldDifferentCount'],   // 184/216 個配方有
    edtHotplatePickOffset:       ['handlerCondition', 'Configuration', 'HotplatlPickOffset'],   // 189/216 個配方有
    edtHotplatePitchOffset:      ['handlerCondition', 'Configuration', 'HotplatlPitchOffset'],   // 189/216 個配方有
    edtHotplatePlaceOffset:      ['handlerCondition', 'Configuration', 'HotplatlPlaceOffset'],   // 189/216 個配方有
    edtHotplateXOffset:          ['handlerCondition', 'Configuration', 'HotplatlXOffset'],   // 189/216 個配方有
    edtHotplateYOffset:          ['handlerCondition', 'Configuration', 'HotplatlYOffset'],   // 189/216 個配方有
    edtInArmAirOn:               ['handlerCondition', 'AutoClean', 'InArmAirOn'],   // 41/216 個配方有
    edtInArmVacuum:              ['handlerCondition', 'AutoClean', 'InArmVacuum'],   // 41/216 個配方有
    edtInArmZSpeed:              ['handlerCondition', 'Configuration', 'iAutoClean_MotorSpeed[3]'],   // 215/216 個配方有
    edtIndexAirOn:               ['handlerCondition', 'AutoClean', 'IndexAirOn'],   // 41/216 個配方有
    edtIndexVacuum:              ['handlerCondition', 'AutoClean', 'IndexVacuum'],   // 41/216 個配方有
    edtLowYieldCount:            ['handlerCondition', 'Configuration', 'iAutoClean_LowYieldCount'],   // 184/216 個配方有
    edtLowYieldLimit:            ['handlerCondition', 'Configuration', 'iAutoClean_LowYieldLimit'],   // 184/216 個配方有
    edtShuttle1PlaceOffset:      ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle1PlaceOffset'],   // 210/216 個配方有
    edtShuttle1XOffset:          ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle1XOffset'],   // 210/216 個配方有
    edtShuttle1YOffset:          ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle1YOffset'],   // 210/216 個配方有
    edtShuttle2PlaceOffset:      ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle2PlaceOffset'],   // 215/216 個配方有
    edtShuttle2XOffset:          ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle2XOffset'],   // 189/216 個配方有
    edtShuttle2YOffset:          ['handlerCondition', 'Configuration', 'iAutoClean_Shuttle2YOffset'],   // 189/216 個配方有
    edtShuttlePitch:             ['handlerCondition', 'Configuration', 'ShuttlePitchOffset'],   // 189/216 個配方有
  },
  // 存檔鈕依據：golden AutoClean/uCleaning.cpp:1778 sbCleanSaveClick
  saveBtn: 'sbCleanSave',
  pending: {
    edACSmartCTF:                ['?', 'Configuration', 'iACSmart_Count CTF', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edAdaptiveIntervalAdj:       ['?', 'Configuration', 'iAdaptiveACIntervalAdj', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edAdaptiveIntervalMax:       ['?', 'Configuration', 'iAdaptiveACIntervalMax', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edAdaptiveIntervalMin:       ['?', 'Configuration', 'iAdaptiveACIntervalMin', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edTimeCT:                    ['?', 'Configuration', 'iAutoCleanTimeCT', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edtACSmart:                  ['?', 'Configuration', 'iACSmart_Count', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edtACSmart_ACContactCount:   ['?', 'Configuration', 'iACSmart_ContactCount', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edtACSmart_ContactTime:      ['?', 'Configuration', 'iACSmart_ContactTime', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edtACSmart_DropOffset1:      ['?', 'Configuration', 'iACSmart_DropHigh', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    IndexArmSpeed:               ['INTEGER', 0, true, 100, 1],
    OutArmSpeed:                 ['INTEGER', 0, true, 100, 1],
    ShuttleSpeed:                ['INTEGER', 0, true, 100, 1],
    XCT1:                        ['INTEGER', 0, true, 100, 1],
    XCT2:                        ['INTEGER', 0, true, 100, 1],
    XPitch1:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    XPitch2:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    XST1:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    XST2:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    YCT1:                        ['INTEGER', 0, true, 100, 1],
    YCT2:                        ['INTEGER', 0, true, 100, 1],
    YPitch1:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    YPitch2:                     ['DOUBLE', 2, true, 0.00, 1000.00],
    YST1:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    YST2:                        ['DOUBLE', 2, true, 0.00, 1000.00],
    edACContactCleanHeight:      ['DOUBLE', 2, false, 0, 0],
    edACContactCount:            ['INTEGER', 0, true, 100, 1],
    edACContactShiftHeight:      ['DOUBLE', 2, true, 0.01, 30.0],
    edACInitalContactCount:      ['INTEGER', 0, false, 0, 0],
    edAdaptiveIntervalAdj:       ['INTEGER', 10, true, 1000, 1],
    edAdaptiveIntervalMax:       ['INTEGER', 10, true, 9999, 10],
    edAdaptiveIntervalMin:       ['INTEGER', 10, true, 9999, 1],
    edAlarmCount:                ['INTEGER', 0, true, 100000, 1],
    edAutoCleanAirForce_Kg:      ['DOUBLE', 2, false, 0, 0],
    edAutoCleanAirForce_N:       ['DOUBLE', 2, false, 0, 0],
    edCleanPadDeviation:         ['DOUBLE', 1, false, 0, 0],
    edContactTime:               ['DOUBLE', 1, false, 0, 0],
    edDevicePices:               ['INTEGER', 0, false, 0, 0],
    edDropOffset1:               ['DOUBLE', 1, false, 0, 0],
    edIntervalContact:           ['INTEGER', 0, true, 100000, 1],
    edPinSingleGf:               ['DOUBLE', 2, false, 0, 0],
    edPinSingleN:                ['DOUBLE', 2, false, 0, 0],
    edPinsCount:                 ['INTEGER', 0, true, 20000, 1],
    edtACSmart_ACContactCount:   ['INTEGER', 0, true, 100, 1],
    edtAutoCleanDieForce:        ['DOUBLE', 2, false, 0, 0],
    edtConseFailureCountByHead_Normal: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountByHead_Retest: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountBySocket_Normal: ['INTEGER', 0, true, 100000, 1],
    edtConseFailureCountBySocket_Retest: ['INTEGER', 0, true, 100000, 1],
    edtFailAlarmSiteYield:       ['INTEGER', 0, true, 100, 1],
    edtFailAlarmSiteYieldDifferent: ['INTEGER', 0, true, 100000, 1],
    edtInArmAirOn:               ['DOUBLE', 2, true, 10.0, 0.01],
    edtInArmVacuum:              ['DOUBLE', 2, true, 10.0, 0.01],
    edtInArmZSpeed:              ['INTEGER', 0, true, 100, 1],
    edtIndexAirOn:               ['DOUBLE', 2, true, 10.0, 0.01],
    edtIndexVacuum:              ['DOUBLE', 2, true, 10.0, 0.01],
    edtLowYieldCount:            ['INTEGER', 0, true, 100000, 1],
    edtLowYieldLimit:            ['INTEGER', 0, true, 100, 1],
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 9 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edACSmartCTF             [Configuration] iACSmart_Count CTF
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edAdaptiveIntervalAdj    [Configuration] iAdaptiveACIntervalAdj
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edAdaptiveIntervalMax    [Configuration] iAdaptiveACIntervalMax
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edAdaptiveIntervalMin    [Configuration] iAdaptiveACIntervalMin
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edTimeCT                 [Configuration] iAutoCleanTimeCT
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edtACSmart               [Configuration] iACSmart_Count
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edtACSmart_ACContactCount [Configuration] iACSmart_ContactCount
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edtACSmart_ContactTime   [Configuration] iACSmart_ContactTime
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edtACSmart_DropOffset1   [Configuration] iACSmart_DropHigh
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
