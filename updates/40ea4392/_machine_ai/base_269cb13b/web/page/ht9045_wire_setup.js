/* ht9045_wire_setup.js -- Setup.SetUp.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260915  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\HT9045\CHANGES_20260915_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cSetUp.dfm
 * 抽取法      WriteIniData（widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             兩者都限定在該表單自己的 .cpp，不做全樹 id 反查。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 14 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 1 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 13 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'Setup.SetUp.html',
  slug: 'setup',
  // Steven 20260916：頁面自己的存檔鈕（引擎不再注入浮動的 Save to recipe / Reload）
  // 依據：golden cSetUp.cpp sbUpdateClick
  saveBtn: 'sbUpdate',
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
  pending: {
    edtXCenterPitch:             ['?', 'Configuration', 'X Center Pitch', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    XPitch:                      ['DOUBLE', 2, true, 0.0, 1000.0],
    XShiftPitch:                 ['DOUBLE', 2, true, -4000.0, 4000.0],
    YPitch:                      ['DOUBLE', 2, true, 0.0, 1000.0],
    edAuto1Count:                ['INTEGER', 0, true, 40, 2],
    edAuto2Count:                ['INTEGER', 0, true, 40, 2],
    edAuto3Count:                ['INTEGER', 0, true, 40, 2],
    edDelayTime:                 ['DOUBLE', 0, true, 10.0, 0.05],
    edOverRange:                 ['DOUBLE', 0, true, 100.0, 0.0],
    edPreciserXPitch:            ['DOUBLE', 2, true, 0.0, 1000.0],
    edPreciserYPitch:            ['DOUBLE', 2, true, 0.0, 1000.0],
    edYOffset:                   ['DOUBLE', 2, true, -4000.0, 4000.0],
    edtGetValueDelayTime:        ['DOUBLE', 1, true, 0, 1],
    edtXCenterPitch:             ['DOUBLE', 2, true, 0.0, 1000.0],
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
