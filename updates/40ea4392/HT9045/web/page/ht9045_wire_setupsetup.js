/* ht9045_wire_setupsetup.js -- Setup.SetUp.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cSetUp.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 14 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 1 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 13 個欄位有 golden 依據
 * ⚠ 其中 3 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.SetUp.html',
  slug: 'setupsetup',
  fields: {
  },
  optional: {
    CoSocketCombo:               ['handlerCondition', 'Configuration', 'SocketCountt'],   // 214/216 個配方有
    XPitch:                      ['handlerCondition', 'Configuration', 'X Pitch'],   // 215/216 個配方有
    XShiftPitch:                 ['handlerCondition', 'Configuration', 'X Shift Pitch'],   // 213/216 個配方有
    YPitch:                      ['handlerCondition', 'Configuration', 'Y Pitch'],   // 215/216 個配方有
    edAuto1Count:                ['handlerCondition', 'Configuration', 'Auto1 Tray Count'],   // 202/216 個配方有
    edAuto2Count:                ['handlerCondition', 'Configuration', 'Auto2 Tray Count'],   // 202/216 個配方有
    edAuto3Count:                ['handlerCondition', 'Configuration', 'Auto3 Tray Count'],   // 202/216 個配方有
    edDelayTime:                 ['handlerCondition', 'Configuration', 'dReadTorqueDelayTime'],   // 215/216 個配方有
    edOverRange:                 ['handlerCondition', 'Configuration', 'dReadTorque'],   // 215/216 個配方有
    edPreciserXPitch:            ['handlerCondition', 'Configuration', 'Preciser X Pitch'],   // 212/216 個配方有
    edPreciserYPitch:            ['handlerCondition', 'Configuration', 'Preciser Y Pitch'],   // 212/216 個配方有
    edYOffset:                   ['handlerCondition', 'Configuration', 'Y Offset'],   // 214/216 個配方有
    edtGetValueDelayTime:        ['handlerCondition', 'Configuration', 'SendGetValueDelayTime'],   // 156/216 個配方有
    edtRTCFileName:              ['handlerCondition', 'Configuration', 'RTCFileName'],   // 206/216 個配方有
  },
  // 存檔鈕依據：golden cSetUp.cpp sbUpdateClick
  saveBtn: 'sbUpdate',
  pending: {
    edtXCenterPitch:             ['?', 'Configuration', 'X Center Pitch', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    XPitch:                      ['DOUBLE', 3, true, 0.0, 1000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:3420 (else of IniConfig.bSPILFunction==true; was 2, true, 0.0~1000.0)
    XShiftPitch:                 ['DOUBLE', 3, true, -4000.0, 4000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:4749 (else of IniConfig.bSPILFunction==true; was 2, false, 0~0)
    YPitch:                      ['DOUBLE', 3, true, 0.0, 1000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:3420 (else of IniConfig.bSPILFunction==true; was 2, true, 0.0~1000.0)
    edAuto1Count:                ['INTEGER', 0, true, 40, 2],
    edAuto2Count:                ['INTEGER', 0, true, 40, 2],
    edAuto3Count:                ['INTEGER', 0, true, 40, 2],
    edDelayTime:                 ['DOUBLE', 0, true, 10.0, 0.05],
    edOverRange:                 ['DOUBLE', 0, true, 100.0, 0.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:4841 (range was dropped by gen_wire.py:309-315)
    edPreciserXPitch:            ['DOUBLE', 3, true, 0.0, 1000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:3420 (else of IniConfig.bSPILFunction==true; was 2, true, 0.0~1000.0)
    edPreciserYPitch:            ['DOUBLE', 3, true, 0.0, 1000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:3420 (else of IniConfig.bSPILFunction==true; was 2, true, 0.0~1000.0)
    edYOffset:                   ['DOUBLE', 3, true, -4000.0, 4000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:4749 (else of IniConfig.bSPILFunction==true; was 2, false, 0~0)
    edtGetValueDelayTime:        ['DOUBLE', 1, true, 0, 1],
    edtXCenterPitch:             ['DOUBLE', 3, true, 0.0, 1000.0],   // AI(W906-QFB) 20261002: golden cSetUp.cpp:3420 (else of IniConfig.bSPILFunction==true; was 2, true, 0.0~1000.0)
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 1 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edtXCenterPitch          [Configuration] X Center Pitch
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
