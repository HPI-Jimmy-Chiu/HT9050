/* ht9045_wire_trayform.js -- Setup.TrayForm.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260915  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cTrayForm.dfm
 * 抽取法      WriteIniData（widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             兩者都限定在該表單自己的 .cpp，不做全樹 id 反查。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 29 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 24 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 3 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 52 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'Setup.TrayForm.html',
  slug: 'trayform',
  fields: {
    Tp1Thick:                    ['tray', 'Type0', 'Think'],
    Tp1TickUp:                   ['tray', 'Type0', 'Pick Up'],
    Tp2Thick:                    ['tray', 'Type1', 'Think'],
    Tp2TickUp:                   ['tray', 'Type1', 'Pick Up'],
    Tp3Thick:                    ['tray', 'Type2', 'Think'],
    Tp3TickUp:                   ['tray', 'Type2', 'Pick Up'],
    TrayName1:                   ['tray', 'Type0', 'Name'],
    TrayName2:                   ['tray', 'Type1', 'Name'],
    XCT1:                        ['tray', 'Type0', 'X Division'],
    XCT2:                        ['tray', 'Type1', 'X Division'],
    XCT3:                        ['tray', 'Type2', 'X Division'],
    XPitch1:                     ['tray', 'Type0', 'X Pitch'],
    XPitch2:                     ['tray', 'Type1', 'X Pitch'],
    XPitch3:                     ['tray', 'Type2', 'X Pitch'],
    XST1:                        ['tray', 'Type0', 'X Start'],
    XST2:                        ['tray', 'Type1', 'X Start'],
    XST3:                        ['tray', 'Type2', 'X Start'],
    YCT1:                        ['tray', 'Type0', 'Y Division'],
    YCT2:                        ['tray', 'Type1', 'Y Division'],
    YCT3:                        ['tray', 'Type2', 'Y Division'],
    YPitch1:                     ['tray', 'Type0', 'Y Pitch'],
    YPitch2:                     ['tray', 'Type1', 'Y Pitch'],
    YPitch3:                     ['tray', 'Type2', 'Y Pitch'],
    YST1:                        ['tray', 'Type0', 'Y Start'],
    YST2:                        ['tray', 'Type1', 'Y Start'],
    YST3:                        ['tray', 'Type2', 'Y Start'],
    edMemo1:                     ['tray', 'Type0', 'Memo'],
    edMemo2:                     ['tray', 'Type1', 'Memo'],
    edMemo3:                     ['tray', 'Type2', 'Memo'],
  },
  optional: {
    XBItem1:                     ['tray', 'Type0', 'Block XItem'],   // 194/215 個配方有
    XBItem2:                     ['tray', 'Type1', 'Block XItem'],   // 194/215 個配方有
    XBItem3:                     ['tray', 'Type2', 'Block XItem'],   // 194/215 個配方有
    XBP1:                        ['tray', 'Type0', 'Block PitchX'],   // 210/215 個配方有
    XBP2:                        ['tray', 'Type1', 'Block PitchX'],   // 210/215 個配方有
    XBP3:                        ['tray', 'Type2', 'Block PitchX'],   // 210/215 個配方有
    XBStart1:                    ['tray', 'Type0', 'Block XStart'],   // 194/215 個配方有
    XBStart2:                    ['tray', 'Type1', 'Block XStart'],   // 194/215 個配方有
    XBStart3:                    ['tray', 'Type2', 'Block XStart'],   // 194/215 個配方有
    YBItem1:                     ['tray', 'Type0', 'Block YItem'],   // 194/215 個配方有
    YBItem2:                     ['tray', 'Type1', 'Block YItem'],   // 194/215 個配方有
    YBItem3:                     ['tray', 'Type2', 'Block YItem'],   // 194/215 個配方有
    YBP1:                        ['tray', 'Type0', 'Block PitchY'],   // 210/215 個配方有
    YBP2:                        ['tray', 'Type1', 'Block PitchY'],   // 210/215 個配方有
    YBP3:                        ['tray', 'Type2', 'Block PitchY'],   // 210/215 個配方有
    YBStart1:                    ['tray', 'Type0', 'Block YStart'],   // 194/215 個配方有
    YBStart2:                    ['tray', 'Type1', 'Block YStart'],   // 194/215 個配方有
    YBStart3:                    ['tray', 'Type2', 'Block YStart'],   // 194/215 個配方有
    YBTypeSize1:                 ['tray', 'Type0', 'Block Tray Type Size'],   // 194/215 個配方有
    YBTypeSize2:                 ['tray', 'Type1', 'Block Tray Type Size'],   // 194/215 個配方有
    YBTypeSize3:                 ['tray', 'Type2', 'Block Tray Type Size'],   // 194/215 個配方有
    edUnloadTrayCount:           ['tray', 'Other', 'Unload Tray Count'],   // 1/215 個配方有
    edtBinBoxAlarm:              ['tray', 'Bin Box', 'Alarm Count'],   // 3/215 個配方有
    edtBinBoxName:               ['tray', 'Bin Box', 'Name'],   // 3/215 個配方有
  },
  pending: {
    edBoatYDivision:             ['?', 'Type0', 'Cassette Z Item', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edBoatYPitch:                ['?', 'Type0', 'Cassette Z Pitch', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edBoatYStart:                ['?', 'Type0', 'Cassette Z Start', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    Tp1Thick:                    ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    Tp1TickUp:                   ['DOUBLE', 3, true, 60, 73.00],
    Tp2Thick:                    ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    Tp2TickUp:                   ['DOUBLE', 3, true, 60, 73.00],
    Tp3Thick:                    ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    Tp3TickUp:                   ['DOUBLE', 3, true, 60, 73.00],
    TrayName1:                   ['NO_SYMBOL', 0, false, 0, 0],
    TrayName2:                   ['NO_SYMBOL', 0, false, 0, 0],
    TrayName3:                   ['NO_SYMBOL', 0, false, 0, 0],
    XBItem1:                     ['INTEGER', 0, true, 1, 1000],
    XBItem2:                     ['INTEGER', 0, true, 1, 1000],
    XBItem3:                     ['INTEGER', 0, true, 1, 1000],
    XBP1:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    XBP2:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    XBP3:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    XBStart1:                    ['INTEGER', 0, true, 1, 1000],
    XBStart2:                    ['INTEGER', 0, true, 1, 1000],
    XBStart3:                    ['INTEGER', 0, true, 1, 1000],
    XCT1:                        ['INTEGER', 0, true, 1, 1000],
    XCT2:                        ['INTEGER', 0, true, 1, 1000],
    XCT3:                        ['INTEGER', 0, true, 1, 1000],
    XPitch1:                     ['DOUBLE', 3, true, 0.000, 1000.00],
    XST1:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    XST2:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    XST3:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBItem1:                     ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBItem2:                     ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBItem3:                     ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBP1:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBP2:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBP3:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBStart1:                    ['INTEGER', 0, true, 1, 1000],
    YBStart2:                    ['INTEGER', 0, true, 1, 1000],
    YBStart3:                    ['INTEGER', 0, true, 1, 1000],
    YBTypeSize1:                 ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBTypeSize2:                 ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YBTypeSize3:                 ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YCT1:                        ['INTEGER', 0, true, 1, 1000],
    YCT2:                        ['INTEGER', 0, true, 1, 1000],
    YCT3:                        ['INTEGER', 0, true, 1, 1000],
    YPitch1:                     ['DOUBLE', 3, true, 0.000, 1000.00],
    YPitch2:                     ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YPitch3:                     ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YST1:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YST2:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    YST3:                        ['DOUBLE', 3, true, 0.001, 1000.00],   // AI(W906-QFB) 20261002: golden cTrayForm.cpp:495 (else of CUSTOMER_CODE==CC_ASE_SG; was 3, true, 0.000~1000.00)
    edMemo1:                     ['NO_SYMBOL', 0, false, 0, 0],
    edMemo2:                     ['NO_SYMBOL', 0, false, 0, 0],
    edMemo3:                     ['NO_SYMBOL', 0, false, 0, 0],
    edUnloadTrayCount:           ['INTEGER', 1, true, 1, 20],
    edtBinBoxAlarm:              ['INTEGER', 0, true, 5000, 100],
    edtBinBoxName:               ['NO_SYMBOL', 0, false, 0, 0],
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 3 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edBoatYDivision          [Type0] Cassette Z Item
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edBoatYPitch             [Type0] Cassette Z Pitch
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edBoatYStart             [Type0] Cassette Z Start
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
