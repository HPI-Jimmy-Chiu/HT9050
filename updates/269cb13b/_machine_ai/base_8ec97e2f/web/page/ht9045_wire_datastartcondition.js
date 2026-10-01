/* ht9045_wire_datastartcondition.js -- Data.StartCondition.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260916  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cStartCondition.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 5 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 0 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 37 個欄位有 golden 依據
 * ⚠ 其中 36 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Data.StartCondition.html',
  slug: 'datastartcondition',
  fields: {
  },
  optional: {
    editContactCountAlarm:       ['handlerCondition', 'HeadCondition', 'HeadContactSet[4]'],   // 215/216 個配方有
    edtInOutArmPickerAlmCnt:     ['handlerCondition', 'O_Count', 'O_20InOutArmLifeCntSet'],   // 46/216 個配方有
    edtKitNo1:                   ['handlerCondition', 'HeadCondition', 'Kit No 1'],   // 215/216 個配方有
    edtKitNo2:                   ['handlerCondition', 'HeadCondition', 'Kit No 2'],   // 215/216 個配方有
    edtKitNo3:                   ['handlerCondition', 'HeadCondition', 'Kit No 3'],   // 215/216 個配方有
  },
  // 存檔鈕依據：golden cStartCondition.cpp:708 sbSaveClick
  saveBtn: 'sbSave',
  pending: {
  },
  kb: {
    PanelAa:                     ['INTEGER', 0, false, 0, 0],
    PanelAb:                     ['INTEGER', 0, false, 0, 0],
    PanelAc:                     ['INTEGER', 0, false, 0, 0],
    PanelAd:                     ['INTEGER', 0, false, 0, 0],
    PanelAe:                     ['INTEGER', 0, false, 0, 0],
    PanelAf:                     ['INTEGER', 0, false, 0, 0],
    PanelAg:                     ['INTEGER', 0, false, 0, 0],
    PanelAh:                     ['INTEGER', 0, false, 0, 0],
    PanelBa:                     ['INTEGER', 0, false, 0, 0],
    PanelBb:                     ['INTEGER', 0, false, 0, 0],
    PanelBc:                     ['INTEGER', 0, false, 0, 0],
    PanelBd:                     ['INTEGER', 0, false, 0, 0],
    PanelBe:                     ['INTEGER', 0, false, 0, 0],
    PanelBf:                     ['INTEGER', 0, false, 0, 0],
    PanelBg:                     ['INTEGER', 0, false, 0, 0],
    PanelBh:                     ['INTEGER', 0, false, 0, 0],
    PanelCa:                     ['INTEGER', 0, false, 0, 0],
    PanelCb:                     ['INTEGER', 0, false, 0, 0],
    PanelCc:                     ['INTEGER', 0, false, 0, 0],
    PanelCd:                     ['INTEGER', 0, false, 0, 0],
    PanelCe:                     ['INTEGER', 0, false, 0, 0],
    PanelCf:                     ['INTEGER', 0, false, 0, 0],
    PanelCg:                     ['INTEGER', 0, false, 0, 0],
    PanelCh:                     ['INTEGER', 0, false, 0, 0],
    PanelDa:                     ['INTEGER', 0, false, 0, 0],
    PanelDb:                     ['INTEGER', 0, false, 0, 0],
    PanelDc:                     ['INTEGER', 0, false, 0, 0],
    PanelDd:                     ['INTEGER', 0, false, 0, 0],
    PanelDe:                     ['INTEGER', 0, false, 0, 0],
    PanelDf:                     ['INTEGER', 0, false, 0, 0],
    PanelDg:                     ['INTEGER', 0, false, 0, 0],
    PanelDh:                     ['INTEGER', 0, false, 0, 0],
    editContactCountAlarm:       ['INTEGER', 0, false, 0, 0],
    edtInArmA:                   ['INTEGER', 0, false, 0, 0],
    edtInOutArmPickerAlmCnt:     ['INTEGER', 0, false, 0, 0],
    edtSetXYOffsetLimit:         ['INTEGER', 0, false, 0, 0],
    edtSetZOffsetLimit:          ['INTEGER', 0, false, 0, 0],
  }
});
