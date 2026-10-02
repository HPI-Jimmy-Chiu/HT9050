/* ht9045_wire_hwiosetview.js -- HW.IoSetView.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260916  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    iosetview.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 表格頁：golden 是 TStringGrid，整個 ioTable 載進格子、雙擊改、Save 整張寫回。
 *   這裡走引擎的 sysGrid 模式（表格 = 檔案），不是逐格 id 對照；
 *   Save 只送有改過的格子，鍵欄唯讀，legacy 的 JSON 快照 grid 被藏起來。
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 0 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 4 個欄位有 golden 依據
 */
HT9045Wire.register({
  page: 'HW.IoSetView.html',
  slug: 'hwiosetview',
  fields: {
  },
  optional: {
  },
  pending: {
  },
  kb: {
    btnModify:                   ['INTEGER', 0, false, 0, 0],
    edtPulseDelay:               ['INTEGER', 0, true, 100, 1],
    edtSendCount:                ['INTEGER', 0, true, 100, 1],
    edtTemp:                     ['INTEGER', 0, true, 100, 1],
  },
  // 表格模式：/api/system/ioTable 整張畫進 #strngrdIoTable，雙擊格子改、Save 只送改過的格子。
  sysGrid: {
    file: 'ioTable', host: 'strngrdIoTable', save: 'sbUpdate', reload: 'sbtReload',
    add: 'btnAddIO', del: 'btnDeleteIO',
    readOnly: [],
    kb: [["Type", "NO_SYMBOL|NO_SPACE"], ["Alias", "NO_SYMBOL|NO_SPACE"], ["Note", "NO_SYMBOL|NO_SPACE"], ["*", "INTEGER"]],
    filters: [{"el": "edtSearchIO", "kind": "search"}, {"el": "cbbType", "kind": "equals", "col": "IOType", "all": "All"}, {"el": "cbbLane", "kind": "equals", "col": "Lane", "all": "All"}]
  }
});
