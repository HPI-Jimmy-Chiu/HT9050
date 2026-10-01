/* ht9045_wire_diointerfacecfg.js -- Setup.DIOInterfaceCfg.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * AI(W906-BA-MERGE) 20260915：由 tools/pagewire/merge_wire.py 產生。
 * 手改這個檔會在下次重跑時被覆蓋 —— 要改請改 merge_wire.py 或來源檔。
 *
 *   我們的來源  （無）
 *   同事的來源  D:\HT9050\HT9045_Client_20260915\client\ht9045_wire_diointerfacecfg.js
 *
 * ★ fields / optional 的分類**不採信任何一方的註解**，一律對本機
 *   64 個工單（D:\HT9045\IniData\Data，只算目錄）重算。每筆後的 N/M 是實測值：
 *   M = 擁有該文件的工單數，N = 其中有這個鍵的。N==M 才是 fields。
 *
 *   必接 0   選用 0   待判 4（其中衝突 0、0 命中 0）
 */
HT9045Wire.register({
  page: 'Setup.DIOInterfaceCfg.html',
  slug: 'diointerfacecfg',
  // 必接：擁有該文件的工單「全部」都有這個鍵。
  fields: {
  },
  // 選用：只有部分工單有。讀取時缺了會停用該欄位，存檔時自動略過。
  optional: {
  },
  // 待判：不接線。理由逐筆寫在後面。
  pending: {
    edPulseWidth:         ['5 bit binary(4ch).ini', 'Start Signal', 'Pluse Width', '他 原本就列 pending：文件 5 bit binary(4ch).ini 不在伺服器認得的清單裡'],
    edSignalAfterOff:     ['5 bit binary(4ch).ini', 'DUT Signal', 'After Off', '他 原本就列 pending：文件 5 bit binary(4ch).ini 不在伺服器認得的清單裡'],
    edSignalBeforeOn:     ['5 bit binary(4ch).ini', 'DUT Signal', 'Before On', '他 原本就列 pending：文件 5 bit binary(4ch).ini 不在伺服器認得的清單裡'],
    edTTLModeName:        ['5 bit binary(4ch).ini', 'Name', 'Data Type', '他 原本就列 pending：文件 5 bit binary(4ch).ini 不在伺服器認得的清單裡'],
  },
  // 小鍵盤：同事從 golden ShowQwertyKey + .dfm OnMouseDown 抽的，原樣收下。
  kb: {
    edPulseWidth:         ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
    edSignalAfterOff:     ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
    edSignalBeforeOn:     ['INTEGER', 0, true, 10, 500],   // AI(W906-QFB) 20261002: golden DIOInterFaceCFG.cpp:187 (else of CosFunction.bTTLUseUSec; was 0, true, 1~500000)
  },
});
