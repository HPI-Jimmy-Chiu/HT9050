/* ht9045_wire_configdiointerfacecfg.js -- Config.DIOInterFaceCFG.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    DIOInterFaceCFG.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 機台設定檔（/api/system/）dio
 *   文字欄位     4 個（逐鍵比對過實檔確認存在）
 *   非文字控制項 0 個（radio group / checkbox / combo）
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 0 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 3 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'Config.DIOInterFaceCFG.html',
  slug: 'configdiointerfacecfg',
  fields: {
  },
  sysFields: {
    edPulseWidth:                  ['dio', 'Start Signal', 'Pluse Width'],
    edSignalAfterOff:              ['dio', 'DUT Signal', 'After Off'],
    edSignalBeforeOn:              ['dio', 'DUT Signal', 'Before On'],
    edTTLModeName:                 ['dio', 'Name', 'Data Type'],
  },
  optional: {
  },
  pending: {
  },
  kb: {
    edPulseWidth:                ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
    edSignalAfterOff:            ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
    edSignalBeforeOn:            ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
  }
});
