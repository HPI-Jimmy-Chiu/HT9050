/* ht9045_wire_setupbarcode.js -- Setup.BarCode.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    BarCode.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 6 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 12 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 33 個欄位有 golden 依據
 * ⚠ 其中 7 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.BarCode.html',
  slug: 'setupbarcode',
  fields: {
  },
  optional: {
    edt2DEnd:                    ['handlerCondition', 'Lot Verification', '2DID End'],   // 22/216 個配方有
    edt2DStart:                  ['handlerCondition', 'Lot Verification', '2DID Start'],   // 22/216 個配方有
    edtLotIDEnd:                 ['handlerCondition', 'Lot Verification', 'Lot ID End'],   // 22/216 個配方有
    edtLotIDStart:               ['handlerCondition', 'Lot Verification', 'Lot ID Start'],   // 22/216 個配方有
    edtLotIDSubstr:              ['handlerCondition', 'Lot Verification', 'Lot ID Substr'],   // 22/216 個配方有
    edtLotIDVerify:              ['handlerCondition', 'Lot Verification', 'Lot ID Name'],   // 22/216 個配方有
  },
  pending: {
    edClip_Address_Auto1:        ['?', 'UnloaderClip', 'UnloaderClip_IP_Auto1', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Address_Auto2:        ['?', 'UnloaderClip', 'UnloaderClip_IP_Auto2', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Address_Auto3:        ['?', 'UnloaderClip', 'UnloaderClip_IP_Auto3', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Address_Fix1:         ['?', 'UnloaderClip', 'UnloaderClip_IP_Fix1', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Address_Fix2:         ['?', 'UnloaderClip', 'UnloaderClip_IP_Fix2', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Address_Fix3:         ['?', 'UnloaderClip', 'UnloaderClip_IP_Fix3', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Auto1:           ['?', 'UnloaderClip', 'UnloaderClip_Port_Auto1', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Auto2:           ['?', 'UnloaderClip', 'UnloaderClip_Port_Auto2', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Auto3:           ['?', 'UnloaderClip', 'UnloaderClip_Port_Auto3', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Fix1:            ['?', 'UnloaderClip', 'UnloaderClip_Port_Fix1', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Fix2:            ['?', 'UnloaderClip', 'UnloaderClip_Port_Fix2', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edClip_Port_Fix3:            ['?', 'UnloaderClip', 'UnloaderClip_Port_Fix3', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    IPPort1:                     ['PORT', 0, false, 0, 0],
    edAutoAdjustLightTimeOut:    ['INTEGER', 2, true, 0, 100000],
    edBarcodePosDelayTime:       ['INTEGER', 2, true, 0, 5000],
    edBarcodeRetryCount:         ['INTEGER', 0, true, 0, 100],
    edBarcodeScanDelayTime:      ['INTEGER', 2, true, 0, 100000],
    edConsecutiveFailure:        ['INTEGER', 0, true, 0, 100],
    edFirstDelay:                ['INTEGER', 2, true, 0, 5000],
    edHandShakeTimeOut:          ['INTEGER', 2, true, 0, 100000],
    edRetryOffsetMove:           ['INTEGER', 0, true, -10, 10],   // AI(W906-QFB) 20261002: golden BarCode.cpp:6682 (range was dropped by gen_wire.py:309-315)
    edRetryShiftOffsetMove:      ['INTEGER', 0, true, -10, 10],   // AI(W906-QFB) 20261002: golden BarCode.cpp:6682 (range was dropped by gen_wire.py:309-315)
    edSFCAutoRetry:              ['INTEGER', 0, true, 0, 100],
    edSFCExposureTimeOut:        ['INTEGER', 2, true, 0, 100000],
    edSFCGetResultTimeOut:       ['INTEGER', 2, true, 0, 100000],
    edSFCStartDelay:             ['INTEGER', 2, true, 0, 5000],
    edSFCUse2PhotoOffset:        ['INTEGER', 0, true, 0, 100],
    edShtDuplicateRetryCnt:      ['INTEGER', 0, true, 0, 100],
    edShuttle_1A_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_1B_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_2A_Port:           ['PORT', 0, false, 0, 0],
    edShuttle_2B_Port:           ['PORT', 0, false, 0, 0],
    edTriggerTime:               ['INTEGER', 2, true, 0, 100000],
    ed_2D_YieldIgnoreCnt:        ['INTEGER', 2, true, 1, 100],
    edt1stLineLength_2DBarCode:  ['INTEGER', 0, true, 100, 0],   // AI(W906-QFB) 20261002: golden BarCode.cpp:9189 (range was dropped by gen_wire.py:309-315)
    edt2DEnd:                    ['INTEGER', 0, true, 0, 100],
    edt2DIDYield:                ['DOUBLE', 3, true, 0.001, 100.0],
    edt2DStart:                  ['INTEGER', 0, true, 0, 100],
    edt2ndLineLength_2DBarCode:  ['INTEGER', 0, true, 100, 0],   // AI(W906-QFB) 20261002: golden BarCode.cpp:9189 (range was dropped by gen_wire.py:309-315)
    edtCheckSumLength:           ['INTEGER', 0, true, 0, 100],
    edtLotIDEnd:                 ['INTEGER', 0, true, 0, 100],
    edtLotIDStart:               ['INTEGER', 0, true, 0, 100],
    edtOffsetX:                  ['DOUBLE', 2, true, 30.0, -30.0],   // AI(W906-QFB) 20261002: golden BarCode.cpp:8206 (range was dropped by gen_wire.py:309-315)
    edtOffsetY:                  ['DOUBLE', 2, true, 30.0, -30.0],   // AI(W906-QFB) 20261002: golden BarCode.cpp:8206 (range was dropped by gen_wire.py:309-315)
    edtXPitch:                   ['DOUBLE', 2, true, 150.0, 0.0],   // AI(W906-QFB) 20261002: golden BarCode.cpp:9166 (range was dropped by gen_wire.py:309-315)
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 12 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edClip_Address_Auto1     [UnloaderClip] UnloaderClip_IP_Auto1
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Address_Auto2     [UnloaderClip] UnloaderClip_IP_Auto2
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Address_Auto3     [UnloaderClip] UnloaderClip_IP_Auto3
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Address_Fix1      [UnloaderClip] UnloaderClip_IP_Fix1
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Address_Fix2      [UnloaderClip] UnloaderClip_IP_Fix2
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Address_Fix3      [UnloaderClip] UnloaderClip_IP_Fix3
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Auto1        [UnloaderClip] UnloaderClip_Port_Auto1
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Auto2        [UnloaderClip] UnloaderClip_Port_Auto2
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Auto3        [UnloaderClip] UnloaderClip_Port_Auto3
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Fix1         [UnloaderClip] UnloaderClip_Port_Fix1
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Fix2         [UnloaderClip] UnloaderClip_Port_Fix2
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edClip_Port_Fix3         [UnloaderClip] UnloaderClip_Port_Fix3
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
