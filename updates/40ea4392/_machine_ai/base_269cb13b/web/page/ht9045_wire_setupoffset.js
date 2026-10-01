/* ht9045_wire_setupoffset.js -- Setup.OffSet.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cOffSet.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 0 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 16 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 83 個欄位有 golden 依據
 * ⚠ 其中 81 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.OffSet.html',
  slug: 'setupoffset',
  fields: {
  },
  optional: {
  },
  pending: {
    edLodX:                      ['?', 'LoaderScale', 'LoaderX', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edLodY:                      ['?', 'LoaderScale', 'LoaderY', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edPreciserClose:             ['?', 'Preciser', 'Preciser Close', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edPreciserOpen:              ['?', 'Preciser', 'Preciser Open', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_IS1X:                     ['?', 'LoaderScale', 'Shuttle1X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_IS1Y:                     ['?', 'LoaderScale', 'Shuttle1Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_IS2X:                     ['?', 'LoaderScale', 'Shuttle2X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_IS2Y:                     ['?', 'LoaderScale', 'Shuttle2Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_OS1X:                     ['?', 'UnLoaderScale', 'Shuttle1X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_OS1Y:                     ['?', 'UnLoaderScale', 'Shuttle1Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_OS2X:                     ['?', 'UnLoaderScale', 'Shuttle2X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    ed_OS2Y:                     ['?', 'UnLoaderScale', 'Shuttle2Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_HP1X:                    ['?', 'LoaderScale', 'HotPlate1X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_HP1Y:                    ['?', 'LoaderScale', 'HotPlate1Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_HP2X:                    ['?', 'LoaderScale', 'HotPlate2X', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edt_HP2Y:                    ['?', 'LoaderScale', 'HotPlate2Y', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    EditPickA:                   ['DOUBLE', 2, false, 0, 0],
    EditPickB:                   ['DOUBLE', 2, false, 0, 0],
    EditPickC:                   ['DOUBLE', 2, false, 0, 0],
    EditPickD:                   ['DOUBLE', 2, false, 0, 0],
    EditPickE:                   ['DOUBLE', 2, false, 0, 0],
    EditPickF:                   ['DOUBLE', 2, false, 0, 0],
    EditPickG:                   ['DOUBLE', 2, false, 0, 0],
    EditPickH:                   ['DOUBLE', 2, false, 0, 0],
    EditPickI:                   ['DOUBLE', 2, false, 0, 0],
    EditPickJ:                   ['DOUBLE', 2, false, 0, 0],
    EditPickK:                   ['DOUBLE', 2, false, 0, 0],
    EditPickL:                   ['DOUBLE', 2, false, 0, 0],
    EditPickM:                   ['DOUBLE', 2, false, 0, 0],
    EditPickN:                   ['DOUBLE', 2, false, 0, 0],
    EditPickO:                   ['DOUBLE', 2, false, 0, 0],
    EditPickP:                   ['DOUBLE', 2, false, 0, 0],
    EdtOffsetAX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetAY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetBX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetBY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetCX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetCY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetDX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetDY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetEX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetEY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetFX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetFY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetGX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetGY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetHX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetHY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetIX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetIY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetJX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetJY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetKX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetKY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetLX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetLY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetMX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetMY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetNX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetNY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetOX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetOY:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetPX:                 ['DOUBLE', 2, false, 0, 0],
    EdtOffsetPY:                 ['DOUBLE', 2, false, 0, 0],
    EdtRelsA:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsB:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsC:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsD:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsE:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsF:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsG:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsH:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsI:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsJ:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsK:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsL:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsM:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsN:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsO:                    ['DOUBLE', 2, false, 0, 0],
    EdtRelsP:                    ['DOUBLE', 2, false, 0, 0],
    IndexArmOffSet1:             ['DOUBLE', 2, false, 0, 0],
    IndexArmOffSet2:             ['DOUBLE', 2, false, 0, 0],
    IndexArmOffSet3:             ['DOUBLE', 2, false, 0, 0],
    IndexArmOffSet4:             ['DOUBLE', 2, false, 0, 0],
    edArmX:                      ['DOUBLE', 2, false, 0, 0],
    edArmY:                      ['DOUBLE', 2, false, 0, 0],
    edOffsetContactForce:        ['DOUBLE', 3, true, 0.0, 10.0],
    edPickUp:                    ['DOUBLE', 2, false, 0, 0],
    edPitchX1:                   ['DOUBLE', 2, false, 0, 0],
    edPitchX2:                   ['DOUBLE', 2, false, 0, 0],
    edPitchX3:                   ['DOUBLE', 2, false, 0, 0],
    edPitchX4:                   ['DOUBLE', 2, false, 0, 0],
    edPitchY:                    ['DOUBLE', 2, false, 0, 0],
    edPreciserClose:             ['DOUBLE', 0.0, false, 0, 0],
    edPreciserOpen:              ['DOUBLE', 0.0, false, 0, 0],
    edRelease:                   ['DOUBLE', 2, false, 0, 0],
    edlLoadZ:                    ['DOUBLE', 2, false, 0, 0],
    edtARTPlace:                 ['DOUBLE', 2, false, 0, 0],
    edtTrayArmX:                 ['DOUBLE', 2, false, 0, 0],
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 16 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edLodX                   [LoaderScale] LoaderX
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edLodY                   [LoaderScale] LoaderY
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edPreciserClose          [Preciser] Preciser Close
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edPreciserOpen           [Preciser] Preciser Open
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_IS1X                  [LoaderScale] Shuttle1X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_IS1Y                  [LoaderScale] Shuttle1Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_IS2X                  [LoaderScale] Shuttle2X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_IS2Y                  [LoaderScale] Shuttle2Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_OS1X                  [UnLoaderScale] Shuttle1X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_OS1Y                  [UnLoaderScale] Shuttle1Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_OS2X                  [UnLoaderScale] Shuttle2X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   ed_OS2Y                  [UnLoaderScale] Shuttle2Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_HP1X                 [LoaderScale] HotPlate1X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_HP1Y                 [LoaderScale] HotPlate1Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_HP2X                 [LoaderScale] HotPlate2X
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edt_HP2Y                 [LoaderScale] HotPlate2Y
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
