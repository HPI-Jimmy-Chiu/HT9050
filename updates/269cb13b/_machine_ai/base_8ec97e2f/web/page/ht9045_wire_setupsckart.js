/* ht9045_wire_setupsckart.js -- Setup.SCK_ART.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    SCK_ART.dfm
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
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 17 個欄位有 golden 依據
 * ⚠ 其中 4 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.SCK_ART.html',
  slug: 'setupsckart',
  fields: {
  },
  optional: {
  },
  pending: {
  },
  kb: {
    edARTAutoSkip:               ['INTEGER', 0, true, 1000, 1],
    edARTPassRate:               ['DOUBLE', 2, true, 0.01, 100.00],
    edBestYield:                 ['DOUBLE', 2, true, 0.0, 100.00],
    edLotCount:                  ['INTEGER', 0, false, 0, 0],
    edSPBinYield:                ['DOUBLE', 2, true, 0.0, 100.00],
    edTemp:                      ['INTEGER', 0, false, 0, 0],
    edlRTTryCnt:                 ['INTEGER', 0, true, 1, 20],
    edtAlmAutoCloseSite:         ['INTEGER', 0, true, 32, 1],
    edtInputCount:               ['INTEGER', 0, false, 0, 0],
    edtLdCntLimN:                ['INTEGER', 0, true, 0, 9999],
    edtLdCntLimP:                ['INTEGER', 0, true, 0, 9999],
    edtLowYieldForFT:            ['DOUBLE', 2, true, 0.0, 100.00],
    edtMRejectCnt:               ['INTEGER', 0, false, 0, 0],
    palInRemoveCnt:              ['INTEGER', 0, false, 0, 0],
    palInputJamCnt:              ['INTEGER', 0, false, 0, 0],
    palOutRemoveCnt:             ['INTEGER', 0, false, 0, 0],
    palOutputJamCnt:             ['INTEGER', 0, false, 0, 0],
  }
});
