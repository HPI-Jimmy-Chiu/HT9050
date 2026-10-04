/* ht9045_wire_hwteach.js -- HW.teach.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260916  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260916_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    uteach.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 機台設定檔（/api/system/）gerneral、teach
 *   文字欄位     387 個（逐鍵比對過實檔確認存在）
 *   非文字控制項 0 個（radio group / checkbox / combo）
 *   契約有、本機實檔沒有 153 個（見檔尾 SYS ABSENT）
 * 必接 0 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 0 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 1 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 521 個欄位有 golden 依據
 * ⚠ 其中 117 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 *
 * AI(W906-KB-GOLDEN) 20261004 (2/2)：kb 有手改（gen_wire.py 不會產生這些；重跑產生器之後要照這裡補回）：
 *   * 8 個 elTeach ECDouble 欄位（golden uteach.cpp:3210-3217 → HTEditList::Add → THTEdit::OnWriteEdit 的 OnClick=EditClick
 *     HTEdit.cpp:72 → :216-223 N_DOUBLE, 3）：原本寫成 INTEGER，改 DOUBLE dp 3；
 *   * kb 尾端一段：golden 有小鍵盤、這裡原本沒有列的 10 格（EditSh1Speed／EditSh2Speed、8 格 edtInAlignPitchX*）＋ edtSpeed（golden ReadOnly 沒有小鍵盤，刻意保留 EastSun 的 TEACH-SPEEDBAR），
 *     以及 golden 點了不會開小鍵盤的 50 格（值 null ＝ 引擎 attachKeyboards 不掛小鍵盤：.dfm 沒有 OnClick／OnMouseDown
 *     又不在 elTeach，或 golden 是 ReadOnly）。
 *   範圍一律不加：使用者裁決 #51 = A（HT9011UC_Cpp_V3.33.906.0/docs/RULINGS_20261002.md:126，main 不夾 Teach 的值），只有旗標與 dp。
 */
HT9045Wire.register({
  page: 'HW.teach.html',
  slug: 'hwteach',
  fields: {
  },
  sysFields: {
    EdtTemp:                       ['teach', 'ArmAlignment', 'InArmAlignmentPitch1'],
    InSHZDownRange:                ['gerneral', 'Shuttle', 'iInShtZRange'],
    SetEditAuto1CassetteZStart:    ['teach', 'MAuto1Z', 'SetEditAuto1CassetteZStart'],
    SetEditAuto1Front:             ['teach', 'MAuto1Y', 'SetEditAuto1Front'],
    SetEditAuto1FrontBack:         ['teach', 'MAuto1Y', 'SetEditAuto1FrontBack'],
    SetEditAuto1Rear:              ['teach', 'MAuto1Y_CCW', 'SetEditAuto1Rear'],
    SetEditAuto1RearBack:          ['teach', 'MAuto1Y_CCW', 'SetEditAuto1RearBack'],
    SetEditAuto2CassetteZStart:    ['teach', 'MAuto2Z', 'SetEditAuto2CassetteZStart'],
    SetEditAuto2Front:             ['teach', 'MAuto2Y', 'SetEditAuto2Front'],
    SetEditAuto2FrontBack:         ['teach', 'MAuto2Y', 'SetEditAuto2FrontBack'],
    SetEditAuto2Rear:              ['teach', 'MAuto2Y_CCW', 'SetEditAuto2Rear'],
    SetEditAuto2RearBack:          ['teach', 'MAuto2Y_CCW', 'SetEditAuto2RearBack'],
    SetEditAutoClean:              ['teach', 'InArm', 'AutoCleanPick'],
    SetEditHP:                     ['teach', 'MInArmZE', 'SetEditHP'],
    SetEditLDCassetteZStart:       ['teach', 'MTrayZ', 'SetEditLDCassetteZStart'],
    SetEditLDFront:                ['teach', 'MLoaderY', 'SetEditLDFront'],
    SetEditLDFrontBack:            ['teach', 'MLoaderY', 'SetEditLDFrontBack'],
    SetEditLDRear:                 ['teach', 'MLoaderY_CCW', 'SetEditLDRear'],
    SetEditLDRearBack:             ['teach', 'MLoaderY_CCW', 'SetEditLDRearBack'],
    SetEditPickInRotate:           ['teach', 'MInArmZE', 'SetEditPickInRotate'],
    SetEditPickLoader:             ['teach', 'MInArmZE', 'SetEditPickLoader'],
    SetEditPickOutRotate:          ['teach', 'MOutArmZE', 'SetEditPickOutRotate'],
    SetEditPickOutSht:             ['teach', 'MOutArmZE', 'SetEditPickOutSht'],
    SetEditPlaceAuto:              ['teach', 'MOutArmZE', 'SetEditPlaceAuto'],
    SetEditPlaceFix:               ['teach', 'MOutArmZE', 'SetEditPlaceFix'],
    SetEditPlaceFix2:              ['teach', 'MOutArmZE', 'SetEditPlaceFix2'],
    SetEditPlaceInRotate:          ['teach', 'MInArmZE', 'SetEditPlaceInRotate'],
    SetEditPlaceInShuttle:         ['teach', 'MInArmZE', 'SetEditPlaceInShuttle'],
    SetEditPlaceNGBinBoxZ:         ['teach', 'MInArmZE', 'SetEditPlaceNGBinBoxZ'],
    SetEditPlaceOutRotate:         ['teach', 'MOutArmZE', 'SetEditPlaceOutRotate'],
    SetEditPlacePreciserZ:         ['teach', 'MInArmZE', 'SetEditPlacePreciserZ'],
    edShtCheckRange:               ['gerneral', 'Shuttle', 'CHECK_RANGE'],
    edtBinBoxX:                    ['teach', 'MOutArmX', 'edtBinBoxX'],
    edtBinBoxY:                    ['teach', 'MOutArmY', 'edtBinBoxY'],
    edtBinBoxZ:                    ['teach', 'MOutArmZE', 'edtBinBoxZ'],
    edtBt2DX:                      ['teach', 'MInArmX', 'edtBt2DX'],
    edtBt2DY:                      ['teach', 'MInArmY', 'edtBt2DY'],
    edtEditMagZStandby:            ['teach', 'MMagazine', 'edtEditMagZStandby'],
    edtEditMagZTray1:              ['teach', 'MMagazine', 'edtEditMagZTray1'],
    edtInAlignPitchXAa:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentAa'],
    edtInAlignPitchXAb:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentAb'],
    edtInAlignPitchXAc:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentAc'],
    edtInAlignPitchXAd:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentAd'],
    edtInAlignPitchXBa:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentBa'],
    edtInAlignPitchXBb:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentBb'],
    edtInAlignPitchXBc:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentBc'],
    edtInAlignPitchXBd:            ['teach', 'ArmAlignment', 'InArmPitchXAlignmentBd'],
    edtInArmAlignPitchY:           ['teach', 'ArmAlignment', 'InArmAlignmentPitch_Y'],
    edtInArmCCDXRadian:            ['teach', 'ArmAlignment', 'InArmCCDXRadian'],
    edtInArmCCDXResolution:        ['teach', 'ArmAlignment', 'InArmCCDXResolution'],
    edtInArmCCDYRadian:            ['teach', 'ArmAlignment', 'InArmCCDYRadian'],
    edtInArmCCDYResolution:        ['teach', 'ArmAlignment', 'InArmCCDYResolution'],
    edtInArmXAlignAa:              ['teach', 'ArmAlignment', 'InArmXAlignmentAa'],
    edtInArmXAlignAb:              ['teach', 'ArmAlignment', 'InArmXAlignmentAb'],
    edtInArmXAlignAc:              ['teach', 'ArmAlignment', 'InArmXAlignmentAc'],
    edtInArmXAlignAd:              ['teach', 'ArmAlignment', 'InArmXAlignmentAd'],
    edtInArmXAlignBa:              ['teach', 'ArmAlignment', 'InArmXAlignmentBa'],
    edtInArmXAlignBb:              ['teach', 'ArmAlignment', 'InArmXAlignmentBb'],
    edtInArmXAlignBc:              ['teach', 'ArmAlignment', 'InArmXAlignmentBc'],
    edtInArmXAlignBd:              ['teach', 'ArmAlignment', 'InArmXAlignmentBd'],
    edtInArmXBasePickerPos:        ['teach', 'MInArmX', 'edtInArmXBasePickerPos'],
    edtInArmXCCDPos:               ['teach', 'MInArmX', 'edtInArmXCCDPos'],
    edtInArmYAlignAa:              ['teach', 'ArmAlignment', 'InArmYAlignmentAa'],
    edtInArmYAlignAb:              ['teach', 'ArmAlignment', 'InArmYAlignmentAb'],
    edtInArmYAlignAc:              ['teach', 'ArmAlignment', 'InArmYAlignmentAc'],
    edtInArmYAlignAd:              ['teach', 'ArmAlignment', 'InArmYAlignmentAd'],
    edtInArmYAlignBa:              ['teach', 'ArmAlignment', 'InArmYAlignmentBa'],
    edtInArmYAlignBb:              ['teach', 'ArmAlignment', 'InArmYAlignmentBb'],
    edtInArmYAlignBc:              ['teach', 'ArmAlignment', 'InArmYAlignmentBc'],
    edtInArmYAlignBd:              ['teach', 'ArmAlignment', 'InArmYAlignmentBd'],
    edtInArmYBasePickerPos:        ['teach', 'MInArmY', 'edtInArmYBasePickerPos'],
    edtInArmYCCDPos:               ['teach', 'MInArmY', 'edtInArmYCCDPos'],
    edtInArmZAlignAa:              ['teach', 'ArmAlignment', 'InArmZAlignmentAa'],
    edtInArmZAlignAb:              ['teach', 'ArmAlignment', 'InArmZAlignmentAb'],
    edtInArmZAlignAc:              ['teach', 'ArmAlignment', 'InArmZAlignmentAc'],
    edtInArmZAlignAd:              ['teach', 'ArmAlignment', 'InArmZAlignmentAd'],
    edtInArmZAlignBa:              ['teach', 'ArmAlignment', 'InArmZAlignmentBa'],
    edtInArmZAlignBb:              ['teach', 'ArmAlignment', 'InArmZAlignmentBb'],
    edtInArmZAlignBc:              ['teach', 'ArmAlignment', 'InArmZAlignmentBc'],
    edtInArmZAlignBd:              ['teach', 'ArmAlignment', 'InArmZAlignmentBd'],
    edtOutAlignPitchXAa:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentAa'],
    edtOutAlignPitchXAb:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentAb'],
    edtOutAlignPitchXAc:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentAc'],
    edtOutAlignPitchXAd:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentAd'],
    edtOutAlignPitchXBa:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentBa'],
    edtOutAlignPitchXBb:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentBb'],
    edtOutAlignPitchXBc:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentBc'],
    edtOutAlignPitchXBd:           ['teach', 'ArmAlignment', 'OutArmPitchXAlignmentBd'],
    edtOutArmAlignPitchY:          ['teach', 'ArmAlignment', 'OutArmAlignmentPitch_Y'],
    edtOutArmCCDXRadian:           ['teach', 'ArmAlignment', 'OutArmCCDXRadian'],
    edtOutArmCCDXResolution:       ['teach', 'ArmAlignment', 'OutArmCCDXResolution'],
    edtOutArmCCDYRadian:           ['teach', 'ArmAlignment', 'OutArmCCDYRadian'],
    edtOutArmCCDYResolution:       ['teach', 'ArmAlignment', 'OutArmCCDYResolution'],
    edtOutArmXAlignAa:             ['teach', 'ArmAlignment', 'OutArmXAlignmentAa'],
    edtOutArmXAlignAb:             ['teach', 'ArmAlignment', 'OutArmXAlignmentAb'],
    edtOutArmXAlignAc:             ['teach', 'ArmAlignment', 'OutArmXAlignmentAc'],
    edtOutArmXAlignAd:             ['teach', 'ArmAlignment', 'OutArmXAlignmentAd'],
    edtOutArmXAlignBa:             ['teach', 'ArmAlignment', 'OutArmXAlignmentBa'],
    edtOutArmXAlignBb:             ['teach', 'ArmAlignment', 'OutArmXAlignmentBb'],
    edtOutArmXAlignBc:             ['teach', 'ArmAlignment', 'OutArmXAlignmentBc'],
    edtOutArmXAlignBd:             ['teach', 'ArmAlignment', 'OutArmXAlignmentBd'],
    edtOutArmXBasePickerPos:       ['teach', 'MOutArmX', 'edtOutArmXBasePickerPos'],
    edtOutArmXCCDPos:              ['teach', 'MOutArmX', 'edtOutArmXCCDPos'],
    edtOutArmYAlignAa:             ['teach', 'ArmAlignment', 'OutArmYAlignmentAa'],
    edtOutArmYAlignAb:             ['teach', 'ArmAlignment', 'OutArmYAlignmentAb'],
    edtOutArmYAlignAc:             ['teach', 'ArmAlignment', 'OutArmYAlignmentAc'],
    edtOutArmYAlignAd:             ['teach', 'ArmAlignment', 'OutArmYAlignmentAd'],
    edtOutArmYAlignBa:             ['teach', 'ArmAlignment', 'OutArmYAlignmentBa'],
    edtOutArmYAlignBb:             ['teach', 'ArmAlignment', 'OutArmYAlignmentBb'],
    edtOutArmYAlignBc:             ['teach', 'ArmAlignment', 'OutArmYAlignmentBc'],
    edtOutArmYAlignBd:             ['teach', 'ArmAlignment', 'OutArmYAlignmentBd'],
    edtOutArmYBasePickerPos:       ['teach', 'MOutArmY', 'edtOutArmYBasePickerPos'],
    edtOutArmYCCDPos:              ['teach', 'MOutArmY', 'edtOutArmYCCDPos'],
    edtOutArmZAlignAa:             ['teach', 'ArmAlignment', 'OutArmZAlignmentAa'],
    edtOutArmZAlignAb:             ['teach', 'ArmAlignment', 'OutArmZAlignmentAb'],
    edtOutArmZAlignAc:             ['teach', 'ArmAlignment', 'OutArmZAlignmentAc'],
    edtOutArmZAlignAd:             ['teach', 'ArmAlignment', 'OutArmZAlignmentAd'],
    edtOutArmZAlignBa:             ['teach', 'ArmAlignment', 'OutArmZAlignmentBa'],
    edtOutArmZAlignBb:             ['teach', 'ArmAlignment', 'OutArmZAlignmentBb'],
    edtOutArmZAlignBc:             ['teach', 'ArmAlignment', 'OutArmZAlignmentBc'],
    edtOutArmZAlignBd:             ['teach', 'ArmAlignment', 'OutArmZAlignmentBd'],
    setAuto1Z:                     ['teach', 'MAuto1Z', 'setAuto1Z'],
    setAuto2Z:                     ['teach', 'MAuto2Z', 'setAuto2Z'],
    setAuto3Z:                     ['teach', 'MAuto3Z', 'setAuto3Z'],
    setAuto4Z:                     ['teach', 'MAuto4Z', 'setAuto4Z'],
    setAuto5Z:                     ['teach', 'MAuto5Z', 'setAuto5Z'],
    setAuto6Z:                     ['teach', 'MAuto6Z', 'setAuto6Z'],
    setColorZ:                     ['teach', 'MColorZ', 'setColorZ'],
    setEdGabageX:                  ['teach', 'MInArmX', 'setEdGabageX'],
    setEdGabageY:                  ['teach', 'MInArmY', 'setEdGabageY'],
    setEdLoadCellY1:               ['teach', 'Index', 'iLoadCellY1'],
    setEdLoadCellY2:               ['teach', 'Index', 'iLoadCellY2'],
    setEdLoadCellZ1:               ['teach', 'Index', 'iLoadCellZ1Down'],
    setEdLoadCellZ2:               ['teach', 'Index', 'iLoadCellZ2Down'],
    setEditAlignInZAa:             ['teach', 'MInArmZA', 'setEditAlignInZAa'],
    setEditAlignInZAb:             ['teach', 'MInArmZC', 'setEditAlignInZAb'],
    setEditAlignInZAc:             ['teach', 'MInArmZE', 'setEditAlignInZAc'],
    setEditAlignInZAd:             ['teach', 'MInArmZG', 'setEditAlignInZAd'],
    setEditAlignInZBa:             ['teach', 'MInArmZB', 'setEditAlignInZBa'],
    setEditAlignInZBb:             ['teach', 'MInArmZD', 'setEditAlignInZBb'],
    setEditAlignInZBc:             ['teach', 'MInArmZF', 'setEditAlignInZBc'],
    setEditAlignInZBd:             ['teach', 'MInArmZH', 'setEditAlignInZBd'],
    setEditAlignOutZAa:            ['teach', 'MOutArmZA', 'setEditAlignOutZAa'],
    setEditAlignOutZAb:            ['teach', 'MOutArmZC', 'setEditAlignOutZAb'],
    setEditAlignOutZAc:            ['teach', 'MOutArmZE', 'setEditAlignOutZAc'],
    setEditAlignOutZAd:            ['teach', 'MOutArmZG', 'setEditAlignOutZAd'],
    setEditAlignOutZBa:            ['teach', 'MOutArmZB', 'setEditAlignOutZBa'],
    setEditAlignOutZBb:            ['teach', 'MOutArmZD', 'setEditAlignOutZBb'],
    setEditAlignOutZBc:            ['teach', 'MOutArmZF', 'setEditAlignOutZBc'],
    setEditAlignOutZBd:            ['teach', 'MOutArmZH', 'setEditAlignOutZBd'],
    setEditAuto1X:                 ['teach', 'MOutArmX', 'setEditAuto1X'],
    setEditAuto1Y:                 ['teach', 'MOutArmY', 'setEditAuto1Y'],
    setEditAuto2X:                 ['teach', 'MOutArmX', 'setEditAuto2X'],
    setEditAuto2Y:                 ['teach', 'MOutArmY', 'setEditAuto2Y'],
    setEditAuto3X:                 ['teach', 'MOutArmX', 'setEditAuto3X'],
    setEditAuto3Y:                 ['teach', 'MOutArmY', 'setEditAuto3Y'],
    setEditAuto4X:                 ['teach', 'MOutArmX', 'setEditAuto4X'],
    setEditAuto4Y:                 ['teach', 'MOutArmY', 'setEditAuto4Y'],
    setEditAuto5X:                 ['teach', 'MOutArmX', 'setEditAuto5X'],
    setEditAuto5Y:                 ['teach', 'MOutArmY', 'setEditAuto5Y'],
    setEditAuto6X:                 ['teach', 'MOutArmX', 'setEditAuto6X'],
    setEditAuto6Y:                 ['teach', 'MOutArmY', 'setEditAuto6Y'],
    setEditAutoCleanX:             ['teach', 'MInArmX', 'setEditAutoCleanX'],
    setEditAutoCleanY:             ['teach', 'MInArmY', 'setEditAutoCleanY'],
    setEditBGAViewX:               ['teach', 'MOutArmX', 'setEditBGAViewX'],
    setEditBGAViewY:               ['teach', 'MOutArmY', 'setEditBGAViewY'],
    setEditBGAViewZ:               ['teach', 'MOutArmZE', 'setEditBGAViewZ'],
    setEditBuffer10X:              ['teach', 'MCasArmX', 'setEditBuffer10X'],
    setEditBuffer10Z:              ['teach', 'MCasArmZ', 'setEditBuffer10Z'],
    setEditBuffer1X:               ['teach', 'MCasArmX', 'setEditBuffer1X'],
    setEditBuffer1Z:               ['teach', 'MCasArmZ', 'setEditBuffer1Z'],
    setEditBuffer2X:               ['teach', 'MCasArmX', 'setEditBuffer2X'],
    setEditBuffer2Z:               ['teach', 'MCasArmZ', 'setEditBuffer2Z'],
    setEditBuffer3X:               ['teach', 'MCasArmX', 'setEditBuffer3X'],
    setEditBuffer3Z:               ['teach', 'MCasArmZ', 'setEditBuffer3Z'],
    setEditBuffer4X:               ['teach', 'MCasArmX', 'setEditBuffer4X'],
    setEditBuffer4Z:               ['teach', 'MCasArmZ', 'setEditBuffer4Z'],
    setEditBuffer5X:               ['teach', 'MCasArmX', 'setEditBuffer5X'],
    setEditBuffer5Z:               ['teach', 'MCasArmZ', 'setEditBuffer5Z'],
    setEditBuffer6X:               ['teach', 'MCasArmX', 'setEditBuffer6X'],
    setEditBuffer6Z:               ['teach', 'MCasArmZ', 'setEditBuffer6Z'],
    setEditBuffer7X:               ['teach', 'MCasArmX', 'setEditBuffer7X'],
    setEditBuffer7Z:               ['teach', 'MCasArmZ', 'setEditBuffer7Z'],
    setEditBuffer8X:               ['teach', 'MCasArmX', 'setEditBuffer8X'],
    setEditBuffer8Z:               ['teach', 'MCasArmZ', 'setEditBuffer8Z'],
    setEditBuffer9X:               ['teach', 'MCasArmX', 'setEditBuffer9X'],
    setEditBuffer9Z:               ['teach', 'MCasArmZ', 'setEditBuffer9Z'],
    setEditCatchMagFront:          ['teach', 'MCatchMgzTray', 'setEditCatchMagFront'],
    setEditCatchMagRear:           ['teach', 'MCatchMgzTray', 'setEditCatchMagRear'],
    setEditFix1X:                  ['teach', 'MOutArmX', 'setEditFix1X'],
    setEditFix1Y:                  ['teach', 'MOutArmY', 'setEditFix1Y'],
    setEditFix2X:                  ['teach', 'MOutArmX', 'setEditFix2X'],
    setEditFix2Y:                  ['teach', 'MOutArmY', 'setEditFix2Y'],
    setEditFix3X:                  ['teach', 'MOutArmX', 'setEditFix3X'],
    setEditFix3Y:                  ['teach', 'MOutArmY', 'setEditFix3Y'],
    setEditFix4X:                  ['teach', 'MOutArmX', 'setEditFix4X'],
    setEditFix4Y:                  ['teach', 'MOutArmY', 'setEditFix4Y'],
    setEditFix5X:                  ['teach', 'MOutArmX', 'setEditFix5X'],
    setEditFix5Y:                  ['teach', 'MOutArmY', 'setEditFix5Y'],
    setEditFix6X:                  ['teach', 'MOutArmX', 'setEditFix6X'],
    setEditFix6Y:                  ['teach', 'MOutArmY', 'setEditFix6Y'],
    setEditHP1X:                   ['teach', 'MInArmX', 'setEditHP1X'],
    setEditHP1Y:                   ['teach', 'MInArmY', 'setEditHP1Y'],
    setEditHP2X:                   ['teach', 'MInArmX', 'setEditHP2X'],
    setEditHP2Y:                   ['teach', 'MInArmY', 'setEditHP2Y'],
    setEditHingeRotateEmpty:       ['teach', 'MLoadHingeR', 'setEditHingeRotateEmpty'],
    setEditHingeRotateLoader:      ['teach', 'MLoadHingeR', 'setEditHingeRotateLoader'],
    setEditInRB:                   ['teach', 'MInRotateB', 'setEditInRB'],
    setEditInRC:                   ['teach', 'MInRotateC', 'setEditInRC'],
    setEditInRD:                   ['teach', 'MInRotateD', 'setEditInRD'],
    setEditInRE:                   ['teach', 'MInRotateE', 'setEditInRE'],
    setEditInRF:                   ['teach', 'MInRotateF', 'setEditInRF'],
    setEditInRG:                   ['teach', 'MInRotateG', 'setEditInRG'],
    setEditInRH:                   ['teach', 'MInRotateH', 'setEditInRH'],
    setEditInSh1LtcSenZ1:          ['teach', 'MInSh1LtcSenZ1', 'setEditInSh1LtcSenZ1'],
    setEditInSh1LtcSenZ2:          ['teach', 'MInSh1LtcSenZ2', 'setEditInSh1LtcSenZ2'],
    setEditInSh2LtcSenZ1:          ['teach', 'MInSh2LtcSenZ1', 'setEditInSh2LtcSenZ1'],
    setEditInSh2LtcSenZ2:          ['teach', 'MInSh2LtcSenZ2', 'setEditInSh2LtcSenZ2'],
    setEditInSht1X:                ['teach', 'MInArmX', 'setEditInSht1X'],
    setEditInSht1Y:                ['teach', 'MInArmY', 'setEditInSht1Y'],
    setEditInSht2X:                ['teach', 'MInArmX', 'setEditInSht2X'],
    setEditInSht2Y:                ['teach', 'MInArmY', 'setEditInSht2Y'],
    setEditInX2120:                ['teach', 'MInArmPitchX2', 'setEditInX2120'],
    setEditInX240:                 ['teach', 'MInArmPitchX2', 'setEditInX240'],
    setEditInX3120:                ['teach', 'MInArmPitchX3', 'setEditInX3120'],
    setEditInX340:                 ['teach', 'MInArmPitchX3', 'setEditInX340'],
    setEditInX4120:                ['teach', 'MInArmPitchX4', 'setEditInX4120'],
    setEditInX440:                 ['teach', 'MInArmPitchX4', 'setEditInX440'],
    setEditInXPitch120:            ['teach', 'MInArmPitch', 'setEditInXPitch120'],
    setEditInXPitch40:             ['teach', 'MInArmPitch', 'setEditInXPitch40'],
    setEditInY15:                  ['teach', 'MInArmPitchY', 'setEditInY15'],
    setEditInY60:                  ['teach', 'MInArmPitchY', 'setEditInY60'],
    setEditInZSafeHeight:          ['teach', 'MInArmZA', 'setEditInZSafeHeight'],
    setEditInarmPlacementX:        ['teach', 'MInPlacementX', 'setEditInarmPlacementX'],
    setEditInarmPlacementXOffsetByBasicSuck: ['teach', 'MInArmX', 'setEditInarmPlacementXOffsetByBasicSuck'],
    setEditInarmPlacementY:        ['teach', 'MInPlacementY', 'setEditInarmPlacementY'],
    setEditInarmPlacementYOffsetByBasicSuck: ['teach', 'MInArmY', 'setEditInarmPlacementYOffsetByBasicSuck'],
    setEditIndex1ToSht1Y:          ['teach', 'MTestY1', 'setEditIndex1ToSht1Y'],
    setEditIndex1ToSht1Z:          ['teach', 'MTestZ1', 'setEditIndex1ToSht1Z'],
    setEditIndex1ToSocketY:        ['teach', 'MTestY1', 'setEditIndex1ToSocketY'],
    setEditIndex2ToSht2Y:          ['teach', 'MTestY2', 'setEditIndex2ToSht2Y'],
    setEditIndex2ToSht2Z:          ['teach', 'MTestZ2', 'setEditIndex2ToSht2Z'],
    setEditIndex2ToSocketY:        ['teach', 'MTestY2', 'setEditIndex2ToSocketY'],
    setEditLoadPortZ:              ['teach', 'MCaselevatorZ', 'setEditLoadPortZ'],
    setEditLoadSafeZ:              ['teach', 'MCaselevatorZ', 'setEditLoadSafeZ'],
    setEditLoadTemporaryZ:         ['teach', 'MCaselevatorZ', 'setEditLoadTemporaryZ'],
    setEditLoadXGabage:            ['teach', 'MInArmX', 'setEditLoadXGabage'],
    setEditLoadYGabage:            ['teach', 'MInArmY', 'setEditLoadYGabage'],
    setEditLoaderX:                ['teach', 'MInArmX', 'setEditLoaderX'],
    setEditLoaderY:                ['teach', 'MInArmY', 'setEditLoaderY'],
    setEditNGBinBoxX:              ['teach', 'MInArmX', 'setEditNGBinBoxX'],
    setEditNGBinBoxY:              ['teach', 'MInArmY', 'setEditNGBinBoxY'],
    setEditOutRB:                  ['teach', 'MOutRotateB', 'setEditOutRB'],
    setEditOutRC:                  ['teach', 'MOutRotateC', 'setEditOutRC'],
    setEditOutRD:                  ['teach', 'MOutRotateD', 'setEditOutRD'],
    setEditOutRE:                  ['teach', 'MOutRotateE', 'setEditOutRE'],
    setEditOutRF:                  ['teach', 'MOutRotateF', 'setEditOutRF'],
    setEditOutRG:                  ['teach', 'MOutRotateG', 'setEditOutRG'],
    setEditOutRH:                  ['teach', 'MOutRotateH', 'setEditOutRH'],
    setEditOutSht1X:               ['teach', 'MOutArmX', 'setEditOutSht1X'],
    setEditOutSht1Y:               ['teach', 'MOutArmY', 'setEditOutSht1Y'],
    setEditOutSht2X:               ['teach', 'MOutArmX', 'setEditOutSht2X'],
    setEditOutSht2Y:               ['teach', 'MOutArmY', 'setEditOutSht2Y'],
    setEditOutX2120:               ['teach', 'MOutArmPitchX2', 'setEditOutX2120'],
    setEditOutX240:                ['teach', 'MOutArmPitchX2', 'setEditOutX240'],
    setEditOutX3120:               ['teach', 'MOutArmPitchX3', 'setEditOutX3120'],
    setEditOutX340:                ['teach', 'MOutArmPitchX3', 'setEditOutX340'],
    setEditOutX4120:               ['teach', 'MOutArmPitchX4', 'setEditOutX4120'],
    setEditOutX440:                ['teach', 'MOutArmPitchX4', 'setEditOutX440'],
    setEditOutXPitch120:           ['teach', 'MOutArmPitch', 'setEditOutXPitch120'],
    setEditOutXPitch40:            ['teach', 'MOutArmPitch', 'setEditOutXPitch40'],
    setEditOutY15:                 ['teach', 'MOutArmPitchY', 'setEditOutY15'],
    setEditOutY60:                 ['teach', 'MOutArmPitchY', 'setEditOutY60'],
    setEditOutZSafeHeight:         ['teach', 'MOutArmZA', 'setEditOutZSafeHeight'],
    setEditPADViewX:               ['teach', 'MOutArmX', 'setEditPADViewX'],
    setEditPADViewY:               ['teach', 'MOutArmY', 'setEditPADViewY'],
    setEditPADViewZ:               ['teach', 'MOutArmZE', 'setEditPADViewZ'],
    setEditPreciserPitchClose:     ['teach', 'MPreciser', 'setEditPreciserPitchClose'],
    setEditPreciserPitchOpen:      ['teach', 'MPreciser', 'setEditPreciserPitchOpen'],
    setEditPreciserX:              ['teach', 'MInArmX', 'setEditPreciserX'],
    setEditPreciserY:              ['teach', 'MInArmY', 'setEditPreciserY'],
    setEditRotateOutX:             ['teach', 'MOutArmX', 'setEditRotateOutX'],
    setEditRotateOutY:             ['teach', 'MOutArmY', 'setEditRotateOutY'],
    setEditRotateX:                ['teach', 'MInArmX', 'setEditRotateX'],
    setEditRotateY:                ['teach', 'MInArmY', 'setEditRotateY'],
    setEditSafePosX:               ['teach', 'MOutArmX', 'setEditSafePosX'],
    setEditSafePosY:               ['teach', 'MOutArmY', 'setEditSafePosY'],
    setEditScannerAOIX:            ['teach', 'MOutArmX', 'setEditScannerAOIX'],
    setEditScannerAOIY:            ['teach', 'MOutArmY', 'setEditScannerAOIY'],
    setEditScannerAOIZ:            ['teach', 'MOutArmZE', 'setEditScannerAOIZ'],
    setEditSht1Pitch120:           ['teach', 'MShuttle1Pitch', 'setEditSht1Pitch120'],
    setEditSht1Pitch180:           ['teach', 'MShuttle1Pitch', 'setEditSht1Pitch180'],
    setEditSht1XGabage:            ['teach', 'MInArmX', 'setEditSht1XGabage'],
    setEditSht1YGabage:            ['teach', 'MInArmY', 'setEditSht1YGabage'],
    setEditSht2Pitch120:           ['teach', 'MShuttle2Pitch', 'setEditSht2Pitch120'],
    setEditSht2Pitch180:           ['teach', 'MShuttle2Pitch', 'setEditSht2Pitch180'],
    setEditSht2XGabage:            ['teach', 'MInArmX', 'setEditSht2XGabage'],
    setEditSht2YGabage:            ['teach', 'MInArmY', 'setEditSht2YGabage'],
    setEditStackedAuto1X:          ['teach', 'MStackedTrayX', 'setEditStackedAuto1X'],
    setEditStackedAuto1Z:          ['teach', 'MStackedTrayZ', 'setEditStackedAuto1Z'],
    setEditStackedAuto2X:          ['teach', 'MStackedTrayX', 'setEditStackedAuto2X'],
    setEditStackedAuto2Z:          ['teach', 'MStackedTrayZ', 'setEditStackedAuto2Z'],
    setEditStackedAuto3X:          ['teach', 'MStackedTrayX', 'setEditStackedAuto3X'],
    setEditStackedAuto3Z:          ['teach', 'MStackedTrayZ', 'setEditStackedAuto3Z'],
    setEditStackedConversionX:     ['teach', 'MStackedTrayX', 'setEditStackedConversionX'],
    setEditStackedConversionZ:     ['teach', 'MStackedTrayZ', 'setEditStackedConversionZ'],
    setEditStackedEmptyX:          ['teach', 'MStackedTrayX', 'setEditStackedEmptyX'],
    setEditStackedEmptyZ:          ['teach', 'MStackedTrayZ', 'setEditStackedEmptyZ'],
    setEditStackedLoaderX:         ['teach', 'MStackedTrayX', 'setEditStackedLoaderX'],
    setEditStackedLoaderZ:         ['teach', 'MStackedTrayZ', 'setEditStackedLoaderZ'],
    setEditTopViewKitZ:            ['teach', 'MAOIKit', 'setEditTopViewKitZ'],
    setEditTopViewKitZup:          ['teach', 'MAOIKit', 'setEditTopViewKitZup'],
    setEditTopViewX:               ['teach', 'MOutArmX', 'setEditTopViewX'],
    setEditTopViewY:               ['teach', 'MOutArmY', 'setEditTopViewY'],
    setEditTopView_Pick:           ['teach', 'MOutArmZE', 'setEditTopView_Pick'],
    setEditTopView_Place:          ['teach', 'MOutArmZE', 'setEditTopView_Place'],
    setEditTrayAuto1X:             ['teach', 'MTrayX', 'setEditTrayAuto1X'],
    setEditTrayAuto2X:             ['teach', 'MTrayX', 'setEditTrayAuto2X'],
    setEditTrayAuto3X:             ['teach', 'MTrayX', 'setEditTrayAuto3X'],
    setEditTrayBracketConversionZ: ['teach', 'MTrayBracketZ', 'setEditTrayBracketConversionZ'],
    setEditTrayBracketSaftZ:       ['teach', 'MTrayBracketZ', 'setEditTrayBracketSaftZ'],
    setEditTrayCleanX:             ['teach', 'MTrayX', 'setEditTrayCleanX'],
    setEditTrayColorX:             ['teach', 'MTrayX', 'setEditTrayColorX'],
    setEditTrayEmptyX:             ['teach', 'MTrayX', 'setEditTrayEmptyX'],
    setEditTrayIDX:                ['teach', 'MTrayX', 'setEditTrayIDX'],
    setEditTrayLoaderX:            ['teach', 'MTrayX', 'setEditTrayLoaderX'],
    setEditTrayMapX:               ['teach', 'MTrayX', 'setEditTrayMapX'],
    setEditTrayOCRX:               ['teach', 'MTrayX', 'setEditTrayOCRX'],
    setEditUnloadPort1Z:           ['teach', 'MUnloadRobotZ', 'setEditUnloadPort1Z'],
    setEditUnloadPort2Z:           ['teach', 'MUnloadRobotZ', 'setEditUnloadPort2Z'],
    setEditUnloadPort3Z:           ['teach', 'MUnloadRobotZ', 'setEditUnloadPort3Z'],
    setEditUnloadPort4Z:           ['teach', 'MUnloadRobotZ', 'setEditUnloadPort4Z'],
    setEditUnloadPortBufferZ:      ['teach', 'MUnloadRobotZ', 'setEditUnloadPortBufferZ'],
    setEditWaitTestZDown:          ['teach', 'MTestZ1', 'setEditWaitTestZDown'],
    setEditZ1A:                    ['teach', 'InArmZSub', 'Picker Aa'],
    setEditZ1B:                    ['teach', 'InArmZSub', 'Picker Ba'],
    setEditZ1C:                    ['teach', 'InArmZSub', 'Picker Ab'],
    setEditZ1D:                    ['teach', 'InArmZSub', 'Picker Bb'],
    setEditZ1E:                    ['teach', 'InArmZSub', 'Picker Ac'],
    setEditZ1F:                    ['teach', 'InArmZSub', 'Picker Bc'],
    setEditZ1G:                    ['teach', 'InArmZSub', 'Picker Ad'],
    setEditZ1H:                    ['teach', 'InArmZSub', 'Picker Bd'],
    setEditZ1I:                    ['teach', 'InArmZSub', 'Picker Ae'],
    setEditZ1J:                    ['teach', 'InArmZSub', 'Picker Be'],
    setEditZ1K:                    ['teach', 'InArmZSub', 'Picker Af'],
    setEditZ1L:                    ['teach', 'InArmZSub', 'Picker Bf'],
    setEditZ1M:                    ['teach', 'InArmZSub', 'Picker Ag'],
    setEditZ1N:                    ['teach', 'InArmZSub', 'Picker Bg'],
    setEditZ1O:                    ['teach', 'InArmZSub', 'Picker Ah'],
    setEditZ1P:                    ['teach', 'InArmZSub', 'Picker Bh'],
    setEditZ2A:                    ['teach', 'OutArmZSub', 'Picker Aa'],
    setEditZ2B:                    ['teach', 'OutArmZSub', 'Picker Ba'],
    setEditZ2C:                    ['teach', 'OutArmZSub', 'Picker Ab'],
    setEditZ2D:                    ['teach', 'OutArmZSub', 'Picker Bb'],
    setEditZ2E:                    ['teach', 'OutArmZSub', 'Picker Ac'],
    setEditZ2F:                    ['teach', 'OutArmZSub', 'Picker Bc'],
    setEditZ2G:                    ['teach', 'OutArmZSub', 'Picker Ad'],
    setEditZ2H:                    ['teach', 'OutArmZSub', 'Picker Bd'],
    setEditZ2I:                    ['teach', 'OutArmZSub', 'Picker Ae'],
    setEditZ2J:                    ['teach', 'OutArmZSub', 'Picker Be'],
    setEditZ2K:                    ['teach', 'OutArmZSub', 'Picker Af'],
    setEditZ2L:                    ['teach', 'OutArmZSub', 'Picker Bf'],
    setEditZ2M:                    ['teach', 'OutArmZSub', 'Picker Ag'],
    setEditZ2N:                    ['teach', 'OutArmZSub', 'Picker Bg'],
    setEditZ2O:                    ['teach', 'OutArmZSub', 'Picker Ah'],
    setEditZ2P:                    ['teach', 'OutArmZSub', 'Picker Bh'],
    setEdtAuto4:                   ['teach', 'MTrayX', 'setEdtAuto4'],
    setEdtAuto5:                   ['teach', 'MTrayX', 'setEdtAuto5'],
    setEdtAuto6:                   ['teach', 'MTrayX', 'setEdtAuto6'],
    setEdtHP1LaserX:               ['teach', 'MInArmX', 'setEdtHP1LaserX'],
    setEdtHP1LaserY:               ['teach', 'MInArmY', 'setEdtHP1LaserY'],
    setEdtHP2LaserX:               ['teach', 'MInArmX', 'setEdtHP2LaserX'],
    setEdtHP2LaserY:               ['teach', 'MInArmY', 'setEdtHP2LaserY'],
    setEdtINDecayX:                ['teach', 'MInArmX', 'setEdtINDecayX'],
    setEdtINDecayY:                ['teach', 'MInArmY', 'setEdtINDecayY'],
    setEdtOUTDecayX:               ['teach', 'MOutArmX', 'setEdtOUTDecayX'],
    setEdtOUTDecayY:               ['teach', 'MOutArmY', 'setEdtOUTDecayY'],
    setEmptyZ:                     ['teach', 'MEmptyZ', 'setEmptyZ'],
    setFix3L:                      ['teach', 'MFix3Full', 'setFix3L'],
    setFix3R:                      ['teach', 'MFix3Full', 'setFix3R'],
    setInPickX:                    ['teach', 'MInArmX', 'setInPickX'],
    setInPickY:                    ['teach', 'MInArmY', 'setInPickY'],
    setLoaderZ:                    ['teach', 'MLoaderZ', 'setLoaderZ'],
    setOutPickX:                   ['teach', 'MOutArmX', 'setOutPickX'],
    setOutPickY:                   ['teach', 'MOutArmY', 'setOutPickY'],
    seteditContactZ1Relative:      ['teach', 'MTestZ1', 'seteditContactZ1Relative'],
    seteditContactZ2Relative:      ['teach', 'MTestZ2', 'seteditContactZ2Relative'],
  },
  optional: {
  },
  pending: {
    setEditTestZSafePos:         ['system:teach', 'MTestZ1/MTestZ2', 'setEditTestZSafePos', '同一個 widget 綁到 2 個馬達區段；golden 同一個值寫兩個區段，一個 id 對不了兩個鍵'],
  },
  kb: {
    InSHZDownRange:              ['INTEGER', 0, true, 0, 500],
    SetEditAuto1CassetteZStart:  ['INTEGER', 0, false, 0, 0],
    SetEditAuto1Front:           ['INTEGER', 0, false, 0, 0],
    SetEditAuto1FrontBack:       ['INTEGER', 0, false, 0, 0],
    SetEditAuto1Rear:            ['INTEGER', 0, false, 0, 0],
    SetEditAuto1RearBack:        ['INTEGER', 0, false, 0, 0],
    SetEditAutoClean:            ['INTEGER', 0, false, 0, 0],
    SetEditHP:                   ['INTEGER', 0, false, 0, 0],
    SetEditLDCassetteZStart:     ['INTEGER', 0, false, 0, 0],
    SetEditLDFront:              ['INTEGER', 0, false, 0, 0],
    SetEditLDFrontBack:          ['INTEGER', 0, false, 0, 0],
    SetEditLDRearBack:           ['INTEGER', 0, false, 0, 0],
    SetEditPickInRotate:         ['INTEGER', 0, false, 0, 0],
    SetEditPickLoader:           ['INTEGER', 0, false, 0, 0],
    SetEditPickOutRotate:        ['INTEGER', 0, false, 0, 0],
    SetEditPickOutSht:           ['INTEGER', 0, false, 0, 0],
    SetEditPlaceAuto:            ['INTEGER', 0, false, 0, 0],
    SetEditPlaceFix:             ['INTEGER', 0, false, 0, 0],
    SetEditPlaceFix2:            ['INTEGER', 0, false, 0, 0],
    SetEditPlaceInRotate:        ['INTEGER', 0, false, 0, 0],
    SetEditPlaceInShuttle:       ['INTEGER', 0, false, 0, 0],
    SetEditPlaceOutRotate:       ['INTEGER', 0, false, 0, 0],
    SetEditPlacePreciserZ:       ['INTEGER', 0, false, 0, 0],
    edShtCheckRange:             ['INTEGER', 0, true, 500, 2500],
    edTopBtnRotate180:           ['INTEGER', 0, false, 0, 0],
    editsetEditZ3A:              ['INTEGER', 0, false, 0, 0],
    editsetEditZ3B:              ['INTEGER', 0, false, 0, 0],
    edtBinBoxX:                  ['INTEGER', 0, false, 0, 0],
    edtBinBoxY:                  ['INTEGER', 0, false, 0, 0],
    edtBinBoxZ:                  ['INTEGER', 0, false, 0, 0],
    edtBt2DX:                    ['INTEGER', 0, false, 0, 0],
    edtBt2DY:                    ['INTEGER', 0, false, 0, 0],
    edtEditInSht1OctSiteKit:     ['INTEGER', 0, false, 0, 0],
    edtEditInSht2OctSiteKit:     ['INTEGER', 0, false, 0, 0],
    edtEditRotateInBacklash:     ['INTEGER', 0, false, 0, 0],
    edtEditRotateOutBacklash:    ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXAa:          ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXAb:          ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXAc:          ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXBa:          ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXBb:          ['INTEGER', 0, false, 0, 0],
    edtInAlignPitchXBc:          ['INTEGER', 0, false, 0, 0],
    edtInArmAlignPitchY:         ['INTEGER', 0, false, 0, 0],
    edtInArmCCDXRadian:          ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3212) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtInArmCCDXResolution:      ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3210) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtInArmCCDYRadian:          ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3213) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtInArmCCDYResolution:      ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3211) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtInArmXAlignAa:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAb:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAc:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAd:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAe:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAf:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAg:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignAh:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBa:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBb:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBc:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBd:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBe:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBf:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBg:            ['INTEGER', 0, false, 0, 0],
    edtInArmXAlignBh:            ['INTEGER', 0, false, 0, 0],
    edtInArmXBasePickerPos:      ['INTEGER', 0, false, 0, 0],
    edtInArmXCCDPos:             ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAa:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAb:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAc:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAd:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAe:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAf:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAg:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignAh:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBa:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBb:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBc:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBd:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBe:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBf:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBg:            ['INTEGER', 0, false, 0, 0],
    edtInArmYAlignBh:            ['INTEGER', 0, false, 0, 0],
    edtInArmYBasePickerPos:      ['INTEGER', 0, false, 0, 0],
    edtInArmYCCDPos:             ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAa:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAb:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAc:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAd:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAe:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAf:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAg:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignAh:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBa:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBb:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBc:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBd:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBe:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBf:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBg:            ['INTEGER', 0, false, 0, 0],
    edtInArmZAlignBh:            ['INTEGER', 0, false, 0, 0],
    edtMoveTo:                   ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAa:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAb:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAc:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAd:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAe:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAf:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXAg:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBa:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBb:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBc:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBd:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBe:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBf:         ['INTEGER', 0, false, 0, 0],
    edtOutAlignPitchXBg:         ['INTEGER', 0, false, 0, 0],
    edtOutArmAlignPitchY:        ['INTEGER', 0, false, 0, 0],
    edtOutArmCCDXRadian:         ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3216) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtOutArmCCDXResolution:     ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3214) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtOutArmCCDYRadian:         ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3217) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtOutArmCCDYResolution:     ['DOUBLE', 3, false, 0, 0],   // AI(W906-KB-GOLDEN) 20261004: elTeach ECDouble (uteach.cpp:3215) -> EditClick N_DOUBLE, 3 (HTEdit.cpp:223); was INTEGER
    edtOutArmXAlignAa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignAh:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXAlignBh:           ['INTEGER', 0, false, 0, 0],
    edtOutArmXBasePickerPos:     ['INTEGER', 0, false, 0, 0],
    edtOutArmXCCDPos:            ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignAh:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYAlignBh:           ['INTEGER', 0, false, 0, 0],
    edtOutArmYBasePickerPos:     ['INTEGER', 0, false, 0, 0],
    edtOutArmYCCDPos:            ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignAh:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBa:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBb:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBc:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBd:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBe:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBf:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBg:           ['INTEGER', 0, false, 0, 0],
    edtOutArmZAlignBh:           ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto4_X:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto4_Y:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto5_X:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto5_Y:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto6_X:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmAuto6_Y:        ['INTEGER', 0, false, 0, 0],
    edtOutSortArmShtL_X:         ['INTEGER', 0, false, 0, 0],
    edtOutSortArmShtL_Y:         ['INTEGER', 0, false, 0, 0],
    edtOutSortSht_L:             ['INTEGER', 0, false, 0, 0],
    edtOutSortSht_R:             ['INTEGER', 0, false, 0, 0],
    edtOuttArmShtR_X:            ['INTEGER', 0, false, 0, 0],
    edtOuttArmShtR_Y:            ['INTEGER', 0, false, 0, 0],
    edtSetEditIS1BarCode:        ['INTEGER', 0, false, 0, 0],
    edtSetEditIS2BarCode:        ['INTEGER', 0, false, 0, 0],
    edtSetEditOS1BarCode:        ['INTEGER', 0, false, 0, 0],
    edtSetEditOS2BarCode:        ['INTEGER', 0, false, 0, 0],
    edtSetOutArmToSortShtPlace:  ['INTEGER', 0, false, 0, 0],
    edtSetPickSortSHT:           ['INTEGER', 0, false, 0, 0],
    edtSetPlaceSortAuto:         ['INTEGER', 0, false, 0, 0],
    edtSetSH1_16SiteKit:         ['INTEGER', 0, false, 0, 0],
    edtSetSH2_16SiteKit:         ['INTEGER', 0, false, 0, 0],
    edtSetSht1Laser:             ['INTEGER', 0, false, 0, 0],
    edtSetSht2Laser:             ['INTEGER', 0, false, 0, 0],
    edtSetSortZSafeHeight:       ['INTEGER', 0, false, 0, 0],
    edt_InPX1_Home_Pos:          ['INTEGER', 0, false, 0, 0],
    edt_InPX2_Home_Pos:          ['INTEGER', 0, false, 0, 0],
    edt_InPX3_Home_Pos:          ['INTEGER', 0, false, 0, 0],
    edt_InPX4_Home_Pos:          ['INTEGER', 0, false, 0, 0],
    edt_InPitchX_13:             ['INTEGER', 0, false, 0, 0],
    edt_InPitchX_24:             ['INTEGER', 0, false, 0, 0],
    edt_LoopPitch_DelayTime:     ['INTEGER', 0, false, 0, 0],
    edt_Loop_1_XPo1:             ['INTEGER', 0, false, 0, 0],
    edt_Loop_1_XPo2:             ['INTEGER', 0, false, 0, 0],
    edt_Loop_3_XPo1:             ['INTEGER', 0, false, 0, 0],
    edt_Loop_3_XPo2:             ['INTEGER', 0, false, 0, 0],
    edt_OutPX1_Home_Pos:         ['INTEGER', 0, false, 0, 0],
    edt_OutPX2_Home_Pos:         ['INTEGER', 0, false, 0, 0],
    edt_OutPX3_Home_Pos:         ['INTEGER', 0, false, 0, 0],
    edt_OutPX4_Home_Pos:         ['INTEGER', 0, false, 0, 0],
    edt_OutPitch_13:             ['INTEGER', 0, false, 0, 0],
    edt_OutPitch_24:             ['INTEGER', 0, false, 0, 0],
    edtsetSortXPitchMax:         ['INTEGER', 0, false, 0, 0],
    edtsetSortXPitchMin:         ['INTEGER', 0, false, 0, 0],
    setAuto1Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto1ZUp:                 ['INTEGER', 0, false, 0, 0],
    setAuto2Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto2ZUp:                 ['INTEGER', 0, false, 0, 0],
    setAuto3Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto3ZUp:                 ['INTEGER', 0, false, 0, 0],
    setAuto4Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto4ZUp:                 ['INTEGER', 0, false, 0, 0],
    setAuto5Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto5ZUp:                 ['INTEGER', 0, false, 0, 0],
    setAuto6Z:                   ['INTEGER', 0, false, 0, 0],
    setAuto6ZUp:                 ['INTEGER', 0, false, 0, 0],
    setColorZ:                   ['INTEGER', 0, false, 0, 0],
    setColorZUp:                 ['INTEGER', 0, false, 0, 0],
    setEdGabageX:                ['INTEGER', 0, false, 0, 0],
    setEdGabageY:                ['INTEGER', 0, false, 0, 0],
    setEdLoadCellY1:             ['INTEGER', 0, false, 0, 0],
    setEdLoadCellY2:             ['INTEGER', 0, false, 0, 0],
    setEdLoadCellZ1:             ['INTEGER', 0, false, 0, 0],
    setEdLoadCellZ2:             ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAa:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAb:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAc:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAd:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAe:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAf:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAg:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZAh:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBa:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBb:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBc:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBd:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBe:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBf:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBg:           ['INTEGER', 0, false, 0, 0],
    setEditAlignInZBh:           ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAa:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAb:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAc:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAd:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAe:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAf:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAg:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZAh:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBa:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBb:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBc:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBd:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBe:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBf:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBg:          ['INTEGER', 0, false, 0, 0],
    setEditAlignOutZBh:          ['INTEGER', 0, false, 0, 0],
    setEditAuto1X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto1Y:               ['INTEGER', 0, false, 0, 0],
    setEditAuto2X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto2Y:               ['INTEGER', 0, false, 0, 0],
    setEditAuto3X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto3Y:               ['INTEGER', 0, false, 0, 0],
    setEditAuto4X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto4Y:               ['INTEGER', 0, false, 0, 0],
    setEditAuto5X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto5Y:               ['INTEGER', 0, false, 0, 0],
    setEditAuto6X:               ['INTEGER', 0, false, 0, 0],
    setEditAuto6Y:               ['INTEGER', 0, false, 0, 0],
    setEditAutoCleanX:           ['INTEGER', 0, false, 0, 0],
    setEditAutoCleanY:           ['INTEGER', 0, false, 0, 0],
    setEditBGAViewX:             ['INTEGER', 0, false, 0, 0],
    setEditBGAViewY:             ['INTEGER', 0, false, 0, 0],
    setEditBGAViewZ:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer10X:            ['INTEGER', 0, false, 0, 0],
    setEditBuffer10Z:            ['INTEGER', 0, false, 0, 0],
    setEditBuffer1X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer1Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer2X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer2Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer3X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer3Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer4X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer4Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer5X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer5Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer6X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer6Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer7X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer7Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer8X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer8Z:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer9X:             ['INTEGER', 0, false, 0, 0],
    setEditBuffer9Z:             ['INTEGER', 0, false, 0, 0],
    setEditFix1X:                ['INTEGER', 0, false, 0, 0],
    setEditFix1Y:                ['INTEGER', 0, false, 0, 0],
    setEditFix2X:                ['INTEGER', 0, false, 0, 0],
    setEditFix2Y:                ['INTEGER', 0, false, 0, 0],
    setEditFix3X:                ['INTEGER', 0, false, 0, 0],
    setEditFix3Y:                ['INTEGER', 0, false, 0, 0],
    setEditFix4X:                ['INTEGER', 0, false, 0, 0],
    setEditFix4Y:                ['INTEGER', 0, false, 0, 0],
    setEditFix5X:                ['INTEGER', 0, false, 0, 0],
    setEditFix5Y:                ['INTEGER', 0, false, 0, 0],
    setEditFix6X:                ['INTEGER', 0, false, 0, 0],
    setEditFix6Y:                ['INTEGER', 0, false, 0, 0],
    setEditHP1X:                 ['INTEGER', 0, false, 0, 0],
    setEditHP1Y:                 ['INTEGER', 0, false, 0, 0],
    setEditHP2X:                 ['INTEGER', 0, false, 0, 0],
    setEditHP2Y:                 ['INTEGER', 0, false, 0, 0],
    setEditHingeRotateEmpty:     ['INTEGER', 0, false, 0, 0],
    setEditHingeRotateLoader:    ['INTEGER', 0, false, 0, 0],
    setEditInRA:                 ['INTEGER', 0, false, 0, 0],
    setEditInRB:                 ['INTEGER', 0, false, 0, 0],
    setEditInRC:                 ['INTEGER', 0, false, 0, 0],
    setEditInRD:                 ['INTEGER', 0, false, 0, 0],
    setEditInRE:                 ['INTEGER', 0, false, 0, 0],
    setEditInRF:                 ['INTEGER', 0, false, 0, 0],
    setEditInRG:                 ['INTEGER', 0, false, 0, 0],
    setEditInRH:                 ['INTEGER', 0, false, 0, 0],
    setEditInSh1LtcSenZ1:        ['INTEGER', 0, false, 0, 0],
    setEditInSh1LtcSenZ2:        ['INTEGER', 0, false, 0, 0],
    setEditInSh2LtcSenZ1:        ['INTEGER', 0, false, 0, 0],
    setEditInSh2LtcSenZ2:        ['INTEGER', 0, false, 0, 0],
    setEditInSht1Left:           ['INTEGER', 0, false, 0, 0],
    setEditInSht1Right:          ['INTEGER', 0, false, 0, 0],
    setEditInSht1X:              ['INTEGER', 0, false, 0, 0],
    setEditInSht1Y:              ['INTEGER', 0, false, 0, 0],
    setEditInSht2Left:           ['INTEGER', 0, false, 0, 0],
    setEditInSht2Right:          ['INTEGER', 0, false, 0, 0],
    setEditInSht2X:              ['INTEGER', 0, false, 0, 0],
    setEditInSht2Y:              ['INTEGER', 0, false, 0, 0],
    setEditInX2120:              ['INTEGER', 0, false, 0, 0],
    setEditInX240:               ['INTEGER', 0, false, 0, 0],
    setEditInX3120:              ['INTEGER', 0, false, 0, 0],
    setEditInX340:               ['INTEGER', 0, false, 0, 0],
    setEditInX4120:              ['INTEGER', 0, false, 0, 0],
    setEditInX440:               ['INTEGER', 0, false, 0, 0],
    setEditInXPitch120:          ['INTEGER', 0, false, 0, 0],
    setEditInXPitch40:           ['INTEGER', 0, false, 0, 0],
    setEditInY15:                ['INTEGER', 0, false, 0, 0],
    setEditInY60:                ['INTEGER', 0, false, 0, 0],
    setEditInZSafeHeight:        ['INTEGER', 0, true, 0, 200],
    setEditInarmPlacementX:      ['INTEGER', 0, false, 0, 0],
    setEditInarmPlacementXOffsetByBasicSuck: ['INTEGER', 0, false, 0, 0],
    setEditInarmPlacementY:      ['INTEGER', 0, false, 0, 0],
    setEditInarmPlacementYOffsetByBasicSuck: ['INTEGER', 0, false, 0, 0],
    setEditIndex1ToSht1Y:        ['INTEGER', 0, false, 0, 0],
    setEditIndex1ToSht1Z:        ['INTEGER', 0, false, 0, 0],
    setEditIndex1ToSocketY:      ['INTEGER', 0, false, 0, 0],
    setEditIndex2ToSht2Y:        ['INTEGER', 0, false, 0, 0],
    setEditIndex2ToSht2Z:        ['INTEGER', 0, false, 0, 0],
    setEditIndex2ToSocketY:      ['INTEGER', 0, false, 0, 0],
    setEditLoadPortZ:            ['INTEGER', 0, false, 0, 0],
    setEditLoadSafeZ:            ['INTEGER', 0, false, 0, 0],
    setEditLoadTemporaryZ:       ['INTEGER', 0, false, 0, 0],
    setEditLoadXGabage:          ['INTEGER', 0, false, 0, 0],
    setEditLoadYGabage:          ['INTEGER', 0, false, 0, 0],
    setEditLoaderX:              ['INTEGER', 0, false, 0, 0],
    setEditLoaderY:              ['INTEGER', 0, false, 0, 0],
    setEditOcrX:                 ['INTEGER', 0, false, 0, 0],
    setEditOcrY:                 ['INTEGER', 0, false, 0, 0],
    setEditOutRA:                ['INTEGER', 0, false, 0, 0],
    setEditOutRB:                ['INTEGER', 0, false, 0, 0],
    setEditOutRC:                ['INTEGER', 0, false, 0, 0],
    setEditOutRD:                ['INTEGER', 0, false, 0, 0],
    setEditOutRE:                ['INTEGER', 0, false, 0, 0],
    setEditOutRF:                ['INTEGER', 0, false, 0, 0],
    setEditOutRG:                ['INTEGER', 0, false, 0, 0],
    setEditOutRH:                ['INTEGER', 0, false, 0, 0],
    setEditOutSht1KitPos:        ['INTEGER', 0, false, 0, 0],
    setEditOutSht1Left:          ['INTEGER', 0, false, 0, 0],
    setEditOutSht1OneRowKit:     ['INTEGER', 0, false, 0, 0],
    setEditOutSht1Right:         ['INTEGER', 0, false, 0, 0],
    setEditOutSht1X:             ['INTEGER', 0, false, 0, 0],
    setEditOutSht1Y:             ['INTEGER', 0, false, 0, 0],
    setEditOutSht2KitPos:        ['INTEGER', 0, false, 0, 0],
    setEditOutSht2Left:          ['INTEGER', 0, false, 0, 0],
    setEditOutSht2OneRowKit:     ['INTEGER', 0, false, 0, 0],
    setEditOutSht2Right:         ['INTEGER', 0, false, 0, 0],
    setEditOutSht2X:             ['INTEGER', 0, false, 0, 0],
    setEditOutSht2Y:             ['INTEGER', 0, false, 0, 0],
    setEditOutX2120:             ['INTEGER', 0, false, 0, 0],
    setEditOutX240:              ['INTEGER', 0, false, 0, 0],
    setEditOutX3120:             ['INTEGER', 0, false, 0, 0],
    setEditOutX340:              ['INTEGER', 0, false, 0, 0],
    setEditOutX4120:             ['INTEGER', 0, false, 0, 0],
    setEditOutX440:              ['INTEGER', 0, false, 0, 0],
    setEditOutXPitch120:         ['INTEGER', 0, false, 0, 0],
    setEditOutXPitch40:          ['INTEGER', 0, false, 0, 0],
    setEditOutY15:               ['INTEGER', 0, false, 0, 0],
    setEditOutY60:               ['INTEGER', 0, false, 0, 0],
    setEditOutZSafeHeight:       ['INTEGER', 0, true, 0, 200],
    setEditPADViewX:             ['INTEGER', 0, false, 0, 0],
    setEditPADViewY:             ['INTEGER', 0, false, 0, 0],
    setEditPADViewZ:             ['INTEGER', 0, false, 0, 0],
    setEditPreciserPitchClose:   ['INTEGER', 0, false, 0, 0],
    setEditPreciserPitchOpen:    ['INTEGER', 0, false, 0, 0],
    setEditPreciserX:            ['INTEGER', 0, false, 0, 0],
    setEditPreciserY:            ['INTEGER', 0, false, 0, 0],
    setEditRotateA:              ['INTEGER', 0, false, 0, 0],
    setEditRotateOutA:           ['INTEGER', 0, false, 0, 0],
    setEditRotateOutX:           ['INTEGER', 0, false, 0, 0],
    setEditRotateOutY:           ['INTEGER', 0, false, 0, 0],
    setEditRotateX:              ['INTEGER', 0, false, 0, 0],
    setEditRotateY:              ['INTEGER', 0, false, 0, 0],
    setEditSafePosX:             ['INTEGER', 0, false, 0, 0],
    setEditSafePosY:             ['INTEGER', 0, false, 0, 0],
    setEditScannerAOIX:          ['INTEGER', 0, false, 0, 0],
    setEditScannerAOIY:          ['INTEGER', 0, false, 0, 0],
    setEditScannerAOIZ:          ['INTEGER', 0, false, 0, 0],
    setEditSht1BusyHalf:         ['INTEGER', 0, false, 0, 0],
    setEditSht1Pitch120:         ['INTEGER', 0, false, 0, 0],
    setEditSht1Pitch180:         ['INTEGER', 0, false, 0, 0],
    setEditSht1XGabage:          ['INTEGER', 0, false, 0, 0],
    setEditSht1YGabage:          ['INTEGER', 0, false, 0, 0],
    setEditSht2BusyHalf:         ['INTEGER', 0, false, 0, 0],
    setEditSht2Pitch120:         ['INTEGER', 0, false, 0, 0],
    setEditSht2Pitch180:         ['INTEGER', 0, false, 0, 0],
    setEditSht2XGabage:          ['INTEGER', 0, false, 0, 0],
    setEditSht2YGabage:          ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto1X:        ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto1Z:        ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto2X:        ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto2Z:        ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto3X:        ['INTEGER', 0, false, 0, 0],
    setEditStackedAuto3Z:        ['INTEGER', 0, false, 0, 0],
    setEditStackedConversionX:   ['INTEGER', 0, false, 0, 0],
    setEditStackedConversionZ:   ['INTEGER', 0, false, 0, 0],
    setEditStackedEmptyX:        ['INTEGER', 0, false, 0, 0],
    setEditStackedEmptyZ:        ['INTEGER', 0, false, 0, 0],
    setEditStackedLoaderX:       ['INTEGER', 0, false, 0, 0],
    setEditStackedLoaderZ:       ['INTEGER', 0, false, 0, 0],
    setEditTestZSafePos:         ['INTEGER', 0, false, 0, 0],
    setEditTopViewKitZ:          ['INTEGER', 0, false, 0, 0],
    setEditTopViewKitZup:        ['INTEGER', 0, false, 0, 0],
    setEditTopViewX:             ['INTEGER', 0, false, 0, 0],
    setEditTopViewY:             ['INTEGER', 0, false, 0, 0],
    setEditTopView_Pick:         ['INTEGER', 0, false, 0, 0],
    setEditTopView_Place:        ['INTEGER', 0, false, 0, 0],
    setEditTrayAuto1X:           ['INTEGER', 0, false, 0, 0],
    setEditTrayAuto2X:           ['INTEGER', 0, false, 0, 0],
    setEditTrayAuto3X:           ['INTEGER', 0, false, 0, 0],
    setEditTrayBracketConversionZ: ['INTEGER', 0, false, 0, 0],
    setEditTrayBracketSaftZ:     ['INTEGER', 0, false, 0, 0],
    setEditTrayCleanX:           ['INTEGER', 0, false, 0, 0],
    setEditTrayColorX:           ['INTEGER', 0, false, 0, 0],
    setEditTrayEmptyX:           ['INTEGER', 0, false, 0, 0],
    setEditTrayIDX:              ['INTEGER', 0, false, 0, 0],
    setEditTrayLoaderX:          ['INTEGER', 0, false, 0, 0],
    setEditTrayMapX:             ['INTEGER', 0, false, 0, 0],
    setEditTrayOCRX:             ['INTEGER', 0, false, 0, 0],
    setEditUnloadPort1Z:         ['INTEGER', 0, false, 0, 0],
    setEditUnloadPort2Z:         ['INTEGER', 0, false, 0, 0],
    setEditUnloadPort3Z:         ['INTEGER', 0, false, 0, 0],
    setEditUnloadPort4Z:         ['INTEGER', 0, false, 0, 0],
    setEditUnloadPortBufferZ:    ['INTEGER', 0, false, 0, 0],
    setEditWaitTestZDown:        ['INTEGER', 0, false, 0, 0],
    setEditZ1A:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1B:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1C:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1D:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1E:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1F:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1G:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1H:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1I:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1J:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1K:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1L:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1M:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1N:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1O:                  ['INTEGER', 0, false, 0, 0],
    setEditZ1P:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2I:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2J:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2K:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2L:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2M:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2N:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2O:                  ['INTEGER', 0, false, 0, 0],
    setEditZ2P:                  ['INTEGER', 0, false, 0, 0],
    setEdtAuto4:                 ['INTEGER', 0, false, 0, 0],
    setEdtAuto5:                 ['INTEGER', 0, false, 0, 0],
    setEdtAuto6:                 ['INTEGER', 0, false, 0, 0],
    setEdtHP1LaserX:             ['INTEGER', 0, false, 0, 0],
    setEdtHP1LaserY:             ['INTEGER', 0, false, 0, 0],
    setEdtHP2LaserX:             ['INTEGER', 0, false, 0, 0],
    setEdtHP2LaserY:             ['INTEGER', 0, false, 0, 0],
    setEdtINDecayX:              ['INTEGER', 0, false, 0, 0],
    setEdtINDecayY:              ['INTEGER', 0, false, 0, 0],
    setEdtINSmartSetupX:         ['INTEGER', 0, false, 0, 0],
    setEdtINSmartSetupY:         ['INTEGER', 0, false, 0, 0],
    setEdtINSmartSetupZ:         ['INTEGER', 0, false, 0, 0],
    setEdtOUTDecayX:             ['INTEGER', 0, false, 0, 0],
    setEdtOUTDecayY:             ['INTEGER', 0, false, 0, 0],
    setEdtOutSmartSetupX:        ['INTEGER', 0, false, 0, 0],
    setEdtOutSmartSetupY:        ['INTEGER', 0, false, 0, 0],
    setEdtOutSmartSetupZ:        ['INTEGER', 0, false, 0, 0],
    setEmptyZ:                   ['INTEGER', 0, false, 0, 0],
    setEmptyZUp:                 ['INTEGER', 0, false, 0, 0],
    setFix3L:                    ['INTEGER', 0, false, 0, 0],
    setFix3R:                    ['INTEGER', 0, false, 0, 0],
    setInPickX:                  ['INTEGER', 0, false, 0, 0],
    setInPickY:                  ['INTEGER', 0, false, 0, 0],
    setLoaderZ:                  ['INTEGER', 0, false, 0, 0],
    setLoaderZUp:                ['INTEGER', 0, false, 0, 0],
    setOutPickX:                 ['INTEGER', 0, false, 0, 0],
    setOutPickY:                 ['INTEGER', 0, false, 0, 0],
    setYCarPos:                  ['INTEGER', 0, false, 0, 0],
    setYOCRPos:                  ['INTEGER', 0, false, 0, 0],
    setYSurePos:                 ['INTEGER', 0, false, 0, 0],
    // AI(W906-KB-GOLDEN) 20261004 (2/2): hand-edited (not gen_wire.py output; re-apply after a regeneration). Golden uteach.dfm /
    //   uteach.cpp of HT9011UC_Code_V3.33.906.0_20260618; flags and dp only -- no range (user ruling #51 = A, RULINGS_20261002.md:126).
    // (a) golden opens a keypad here and the table had no row (the generic QWERTY was used):
    EditSh1Speed:                ['INTEGER', 0, false, 0, 0],   // dfm:5992-6005 OnClick=EditSh1SpeedClick -> uteach.cpp:4716-4719 ShowQwertyKey(N_INTEGER, 0, true, ...) (range dropped, #51 A)
    EditSh2Speed:                ['INTEGER', 0, false, 0, 0],   // dfm:5936-5949 OnClick=EditSh1SpeedClick -> uteach.cpp:4716-4719 (same handler)
    edtSpeed:                    ['INTEGER', 0, false, 0, 0],   // dfm:1192-1206 ReadOnly in golden (no keypad there): a DELIBERATE deviation -- EastSun's TEACH-SPEEDBAR (machine web/0057 + cpp/0087, in main) lets the operator type the jog speed here (HW.teach.html :761-773 / WebMotorAccess.cpp speedEvent), so INTEGER
    //   no OnClick in the .dfm, but elTeach->Add (uteach.cpp:3222-3233) -> THTEdit::OnWriteEdit OnClick = EditClick (HTEdit.cpp:72)
    //   -> ECInteger N_INTEGER, 0 (HTEdit.cpp:214), like their siblings edtInAlignPitchXAa..Ac / Ba..Bc above:
    edtInAlignPitchXAd:          ['INTEGER', 0, false, 0, 0],   // dfm:22519 no OnClick; elTeach uteach.cpp:3222
    edtInAlignPitchXAe:          ['INTEGER', 0, false, 0, 0],   // dfm:22526 no OnClick; elTeach uteach.cpp:3223
    edtInAlignPitchXAf:          ['INTEGER', 0, false, 0, 0],   // dfm:22533 no OnClick; elTeach uteach.cpp:3224
    edtInAlignPitchXAg:          ['INTEGER', 0, false, 0, 0],   // dfm:22540 no OnClick; elTeach uteach.cpp:3225
    edtInAlignPitchXBd:          ['INTEGER', 0, false, 0, 0],   // dfm:22547 no OnClick; elTeach uteach.cpp:3230
    edtInAlignPitchXBe:          ['INTEGER', 0, false, 0, 0],   // dfm:22554 no OnClick; elTeach uteach.cpp:3231
    edtInAlignPitchXBf:          ['INTEGER', 0, false, 0, 0],   // dfm:22561 no OnClick; elTeach uteach.cpp:3232
    edtInAlignPitchXBg:          ['INTEGER', 0, false, 0, 0],   // dfm:22568 no OnClick; elTeach uteach.cpp:3233
    // (c) null = golden opens NO keypad here (ht9045_wire_engine.js attachKeyboards: readonly, no QWERTY, the title says why).
    //   golden ReadOnly (written by golden code only: ScanNowMotorStatus :1336/:1342, the HOME end :1395/:1402):
    edtNowPosition:              null,   // dfm:1229-1242 ReadOnly = True, no OnClick
    edtSetToOffset:              null,   // dfm:1243-1256 ReadOnly = True, no OnClick
    //   TEdits with no OnClick / OnMouseDown in uteach.dfm and in no HTEditList (checked: every elTeach->Add, TECH_PARA / TECH_TWOPARA
    //   constructors uteach.cpp:61-75 / :159-178 set no event, no runtime ->OnClick= in the golden tree). Golden behaviour, kept:
    //   e.g. SetEditAuto1Front has OnClick (dfm:9723) but SetEditAuto2Front does not; setEditZ1A has it, setEditZ2A..H do not.
    seteditContactZ1Relative:    null,   // dfm:1975 no OnClick
    seteditContactZ2Relative:    null,   // dfm:1982 no OnClick
    setEditZ2G:                  null,   // dfm:3514 no OnClick
    setEditZ2E:                  null,   // dfm:3521 no OnClick
    setEditZ2C:                  null,   // dfm:3528 no OnClick
    setEditZ2A:                  null,   // dfm:3535 no OnClick
    setEditZ2H:                  null,   // dfm:3542 no OnClick
    setEditZ2F:                  null,   // dfm:3549 no OnClick
    setEditZ2D:                  null,   // dfm:3556 no OnClick
    setEditZ2B:                  null,   // dfm:3563 no OnClick
    setEditNGBinBoxX:            null,   // dfm:7279 no OnClick
    setEditNGBinBoxY:            null,   // dfm:7286 no OnClick
    edtBtBlowY:                  null,   // dfm:7396 no OnClick
    edtBtBlowX:                  null,   // dfm:7403 no OnClick
    SetEditPlaceNGBinBoxZ:       null,   // dfm:7625 no OnClick
    SetEditLDRear:               null,   // dfm:7895 no OnClick
    edtOutArmWaitX:              null,   // dfm:9525 no OnClick
    edtOutArmWaitY:              null,   // dfm:9533 no OnClick
    SetEditAuto2Front:           null,   // dfm:9841 no OnClick
    SetEditAuto2FrontBack:       null,   // dfm:9848 no OnClick
    SetEditAuto2Rear:            null,   // dfm:9855 no OnClick
    SetEditAuto2RearBack:        null,   // dfm:9862 no OnClick
    SetEditAuto2CassetteZStart:  null,   // dfm:9869 no OnClick
    edLeftTops65X:               null,   // dfm:14924 no OnClick
    edLeftTops65Y:               null,   // dfm:14931 no OnClick
    edRightBottoms65X:           null,   // dfm:14938 no OnClick
    edRightBottoms65Y:           null,   // dfm:14945 no OnClick
    edLeftTopl65X:               null,   // dfm:15039 no OnClick
    edLeftTopl65Y:               null,   // dfm:15046 no OnClick
    edRightTopl65X:              null,   // dfm:15053 no OnClick
    edRightTopl65Y:              null,   // dfm:15060 no OnClick
    edLeftBottoml65X:            null,   // dfm:15067 no OnClick
    edRightBottoml65X:           null,   // dfm:15074 no OnClick
    edLeftBottoml65Y:            null,   // dfm:15081 no OnClick
    edRightBottoml65Y:           null,   // dfm:15088 no OnClick
    edTopBtnCenterX:             null,   // dfm:15142 no OnClick
    edTopBtnCenterY:             null,   // dfm:15149 no OnClick
    edTopBtnRotate0:             null,   // dfm:15211 no OnClick
    setEditLoadPort4Z:           null,   // dfm:21020 no OnClick
    setEditLoadPort3Z:           null,   // dfm:21033 no OnClick
    setEditLoadPort2Z:           null,   // dfm:21046 no OnClick
    setEditLoadPort1Z:           null,   // dfm:21059 no OnClick
    setEditLoadPortBufferZ:      null,   // dfm:21072 no OnClick
    edtEditMagZTray1:            null,   // dfm:24269 no OnClick
    edtEditMagZStandby:          null,   // dfm:24319 no OnClick
    setEditCatchMagRear:         null,   // dfm:24412 no OnClick
    setEditCatchMagFront:        null,   // dfm:24426 no OnClick
    EdtTemp:                     null,   // dfm:24581 no OnClick; elTeach gives it EditClick (uteach.cpp:3149..) but golden moves it off screen (:267 Left=-200), so no operator reaches it
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
 *   setEditTestZSafePos      [MTestZ1/MTestZ2] setEditTestZSafePos
 *       同一個 widget 綁到 2 個馬達區段；golden 同一個值寫兩個區段，一個 id 對不了兩個鍵
 * --------------------------------------------------------------------------- */

/* --- SYS ABSENT ------------------------------------------------------------
 * 對照表沒問題（來源是 golden 同一行的 WriteIniData*），但這台機器的
 * 實體設定檔裡目前沒有這個鍵 —— 多半是機型沒有那些 site/軸。
 * 與 PENDING 的差別：PENDING 是抽取抽錯，接了會寫錯地方；
 * 這裡只是本機缺鍵，上機台後鍵存在就該接上，重跑產生器即可。
 *
 *   edLeftBottoml65X           teach      [MTopAOIArmX] edLeftBottoml65X
 *   edLeftBottoml65Y           teach      [MTopAOIArmY] edLeftBottoml65Y
 *   edLeftTopl65X              teach      [MTopAOIArmX] edLeftTopl65X
 *   edLeftTopl65Y              teach      [MTopAOIArmY] edLeftTopl65Y
 *   edLeftTops65X              teach      [MTopAOIArmX] edLeftTops65X
 *   edLeftTops65Y              teach      [MTopAOIArmY] edLeftTops65Y
 *   edRightBottoml65X          teach      [MTopAOIArmX] edRightBottoml65X
 *   edRightBottoml65Y          teach      [MTopAOIArmY] edRightBottoml65Y
 *   edRightBottoms65X          teach      [MTopAOIArmX] edRightBottoms65X
 *   edRightBottoms65Y          teach      [MTopAOIArmY] edRightBottoms65Y
 *   edRightTopl65X             teach      [MTopAOIArmX] edRightTopl65X
 *   edRightTopl65Y             teach      [MTopAOIArmY] edRightTopl65Y
 *   edTopBtnCenterX            teach      [MTopAOIArmX] edTopBtnCenterX
 *   edTopBtnCenterY            teach      [MTopAOIArmY] edTopBtnCenterY
 *   edTopBtnRotate0            teach      [MTopAOIArmR] edTopBtnRotate0
 *   edTopBtnRotate180          teach      [MTopAOIArmR] edTopBtnRotate180
 *   editsetEditZ3A             teach      [SortArmZSub] Picker Aa
 *   editsetEditZ3B             teach      [SortArmZSub] Picker Ab
 *   edtEditInSht1OctSiteKit    teach      [MInShuttle1] edtEditInSht1OctSiteKit
 *   edtEditInSht2OctSiteKit    teach      [MInShuttle2] edtEditInSht2OctSiteKit
 *   edtInAlignPitchXAe         teach      [ArmAlignment] InArmPitchXAlignmentAe
 *   edtInAlignPitchXAf         teach      [ArmAlignment] InArmPitchXAlignmentAf
 *   edtInAlignPitchXAg         teach      [ArmAlignment] InArmPitchXAlignmentAg
 *   edtInAlignPitchXBe         teach      [ArmAlignment] InArmPitchXAlignmentBe
 *   edtInAlignPitchXBf         teach      [ArmAlignment] InArmPitchXAlignmentBf
 *   edtInAlignPitchXBg         teach      [ArmAlignment] InArmPitchXAlignmentBg
 *   edtInArmXAlignAe           teach      [ArmAlignment] InArmXAlignmentAe
 *   edtInArmXAlignAf           teach      [ArmAlignment] InArmXAlignmentAf
 *   edtInArmXAlignAg           teach      [ArmAlignment] InArmXAlignmentAg
 *   edtInArmXAlignAh           teach      [ArmAlignment] InArmXAlignmentAh
 *   edtInArmXAlignBe           teach      [ArmAlignment] InArmXAlignmentBe
 *   edtInArmXAlignBf           teach      [ArmAlignment] InArmXAlignmentBf
 *   edtInArmXAlignBg           teach      [ArmAlignment] InArmXAlignmentBg
 *   edtInArmXAlignBh           teach      [ArmAlignment] InArmXAlignmentBh
 *   edtInArmYAlignAe           teach      [ArmAlignment] InArmYAlignmentAe
 *   edtInArmYAlignAf           teach      [ArmAlignment] InArmYAlignmentAf
 *   edtInArmYAlignAg           teach      [ArmAlignment] InArmYAlignmentAg
 *   edtInArmYAlignAh           teach      [ArmAlignment] InArmYAlignmentAh
 *   edtInArmYAlignBe           teach      [ArmAlignment] InArmYAlignmentBe
 *   edtInArmYAlignBf           teach      [ArmAlignment] InArmYAlignmentBf
 *   edtInArmYAlignBg           teach      [ArmAlignment] InArmYAlignmentBg
 *   edtInArmYAlignBh           teach      [ArmAlignment] InArmYAlignmentBh
 *   edtInArmZAlignAe           teach      [ArmAlignment] InArmZAlignmentAe
 *   edtInArmZAlignAf           teach      [ArmAlignment] InArmZAlignmentAf
 *   edtInArmZAlignAg           teach      [ArmAlignment] InArmZAlignmentAg
 *   edtInArmZAlignAh           teach      [ArmAlignment] InArmZAlignmentAh
 *   edtInArmZAlignBe           teach      [ArmAlignment] InArmZAlignmentBe
 *   edtInArmZAlignBf           teach      [ArmAlignment] InArmZAlignmentBf
 *   edtInArmZAlignBg           teach      [ArmAlignment] InArmZAlignmentBg
 *   edtInArmZAlignBh           teach      [ArmAlignment] InArmZAlignmentBh
 *   edtOutAlignPitchXAe        teach      [ArmAlignment] OutArmPitchXAlignmentAe
 *   edtOutAlignPitchXAf        teach      [ArmAlignment] OutArmPitchXAlignmentAf
 *   edtOutAlignPitchXAg        teach      [ArmAlignment] OutArmPitchXAlignmentAg
 *   edtOutAlignPitchXBe        teach      [ArmAlignment] OutArmPitchXAlignmentBe
 *   edtOutAlignPitchXBf        teach      [ArmAlignment] OutArmPitchXAlignmentBf
 *   edtOutAlignPitchXBg        teach      [ArmAlignment] OutArmPitchXAlignmentBg
 *   edtOutArmXAlignAe          teach      [ArmAlignment] OutArmXAlignmentAe
 *   edtOutArmXAlignAf          teach      [ArmAlignment] OutArmXAlignmentAf
 *   edtOutArmXAlignAg          teach      [ArmAlignment] OutArmXAlignmentAg
 *   edtOutArmXAlignAh          teach      [ArmAlignment] OutArmXAlignmentAh
 *   edtOutArmXAlignBe          teach      [ArmAlignment] OutArmXAlignmentBe
 *   edtOutArmXAlignBf          teach      [ArmAlignment] OutArmXAlignmentBf
 *   edtOutArmXAlignBg          teach      [ArmAlignment] OutArmXAlignmentBg
 *   edtOutArmXAlignBh          teach      [ArmAlignment] OutArmXAlignmentBh
 *   edtOutArmYAlignAe          teach      [ArmAlignment] OutArmYAlignmentAe
 *   edtOutArmYAlignAf          teach      [ArmAlignment] OutArmYAlignmentAf
 *   edtOutArmYAlignAg          teach      [ArmAlignment] OutArmYAlignmentAg
 *   edtOutArmYAlignAh          teach      [ArmAlignment] OutArmYAlignmentAh
 *   edtOutArmYAlignBe          teach      [ArmAlignment] OutArmYAlignmentBe
 *   edtOutArmYAlignBf          teach      [ArmAlignment] OutArmYAlignmentBf
 *   edtOutArmYAlignBg          teach      [ArmAlignment] OutArmYAlignmentBg
 *   edtOutArmYAlignBh          teach      [ArmAlignment] OutArmYAlignmentBh
 *   edtOutArmZAlignAe          teach      [ArmAlignment] OutArmZAlignmentAe
 *   edtOutArmZAlignAf          teach      [ArmAlignment] OutArmZAlignmentAf
 *   edtOutArmZAlignAg          teach      [ArmAlignment] OutArmZAlignmentAg
 *   edtOutArmZAlignAh          teach      [ArmAlignment] OutArmZAlignmentAh
 *   edtOutArmZAlignBe          teach      [ArmAlignment] OutArmZAlignmentBe
 *   edtOutArmZAlignBf          teach      [ArmAlignment] OutArmZAlignmentBf
 *   edtOutArmZAlignBg          teach      [ArmAlignment] OutArmZAlignmentBg
 *   edtOutArmZAlignBh          teach      [ArmAlignment] OutArmZAlignmentBh
 *   edtOutSortArmAuto4_X       teach      [MOutSortX] edtOutSortArmAuto4_X
 *   edtOutSortArmAuto4_Y       teach      [MOutSortY] edtOutSortArmAuto4_Y
 *   edtOutSortArmAuto5_X       teach      [MOutSortX] edtOutSortArmAuto5_X
 *   edtOutSortArmAuto5_Y       teach      [MOutSortY] edtOutSortArmAuto5_Y
 *   edtOutSortArmAuto6_X       teach      [MOutSortX] edtOutSortArmAuto6_X
 *   edtOutSortArmAuto6_Y       teach      [MOutSortY] edtOutSortArmAuto6_Y
 *   edtOutSortArmShtL_X        teach      [MOutSortX] edtOutSortArmShtL_X
 *   edtOutSortArmShtL_Y        teach      [MOutSortY] edtOutSortArmShtL_Y
 *   edtOutSortSht_L            teach      [MOutSortSht] edtOutSortSht_L
 *   edtOutSortSht_R            teach      [MOutSortSht] edtOutSortSht_R
 *   edtOuttArmShtR_X           teach      [MOutArmX] edtOuttArmShtR_X
 *   edtOuttArmShtR_Y           teach      [MOutArmY] edtOuttArmShtR_Y
 *   edtSetEditIS1BarCode       teach      [MInShuttle1] edtSetEditIS1BarCode
 *   edtSetEditIS2BarCode       teach      [MInShuttle2] edtSetEditIS2BarCode
 *   edtSetEditOS1BarCode       teach      [MInShuttle1] edtSetEditOS1BarCode
 *   edtSetEditOS2BarCode       teach      [MInShuttle2] edtSetEditOS2BarCode
 *   edtSetOutArmToSortShtPlace teach      [MOutArmZE] edtSetOutArmToSortShtPlace
 *   edtSetPickSortSHT          teach      [MOutSortAb] edtSetPickSortSHT
 *   edtSetPlaceSortAuto        teach      [MOutSortAb] edtSetPlaceSortAuto
 *   edtSetSH1_16SiteKit        teach      [MInShuttle1] edtSetSH1_16SiteKit
 *   edtSetSH2_16SiteKit        teach      [MInShuttle2] edtSetSH2_16SiteKit
 *   edtSetSht1Laser            teach      [MInShuttle1] edtSetSht1Laser
 *   edtSetSht2Laser            teach      [MInShuttle2] edtSetSht2Laser
 *   edtSetSortZSafeHeight      teach      [MOutSortAb] edtSetSortZSafeHeight
 *   edtsetSortXPitchMax        teach      [MOutSortPitchX] edtsetSortXPitchMax
 *   edtsetSortXPitchMin        teach      [MOutSortPitchX] edtsetSortXPitchMin
 *   setAuto1ZUp                teach      [MTrayZ] setAuto1ZUp
 *   setAuto2ZUp                teach      [MTrayZ] setAuto2ZUp
 *   setAuto3ZUp                teach      [MTrayZ] setAuto3ZUp
 *   setAuto4ZUp                teach      [MTrayZ] setAuto4ZUp
 *   setAuto5ZUp                teach      [MTrayZ] setAuto5ZUp
 *   setAuto6ZUp                teach      [MTrayZ] setAuto6ZUp
 *   setColorZUp                teach      [MTrayZ] setColorZUp
 *   setEditAlignInZAe          teach      [MInArmZA] setEditAlignInZAe
 *   setEditAlignInZAf          teach      [MInArmZC] setEditAlignInZAf
 *   setEditAlignInZAg          teach      [MInArmZE] setEditAlignInZAg
 *   setEditAlignInZAh          teach      [MInArmZG] setEditAlignInZAh
 *   setEditAlignInZBe          teach      [MInArmZB] setEditAlignInZBe
 *   setEditAlignInZBf          teach      [MInArmZD] setEditAlignInZBf
 *   setEditAlignInZBg          teach      [MInArmZF] setEditAlignInZBg
 *   setEditAlignInZBh          teach      [MInArmZH] setEditAlignInZBh
 *   setEditAlignOutZAe         teach      [MOutArmZA] setEditAlignOutZAe
 *   setEditAlignOutZAf         teach      [MOutArmZC] setEditAlignOutZAf
 *   setEditAlignOutZAg         teach      [MOutArmZE] setEditAlignOutZAg
 *   setEditAlignOutZAh         teach      [MOutArmZG] setEditAlignOutZAh
 *   setEditAlignOutZBe         teach      [MOutArmZB] setEditAlignOutZBe
 *   setEditAlignOutZBf         teach      [MOutArmZD] setEditAlignOutZBf
 *   setEditAlignOutZBg         teach      [MOutArmZF] setEditAlignOutZBg
 *   setEditAlignOutZBh         teach      [MOutArmZH] setEditAlignOutZBh
 *   setEditInRA                teach      [MInRotateKit] setEditInRA
 *   setEditInSht1Left          teach      [MInShuttle1] setEditInSht1Left
 *   setEditInSht1Right         teach      [MInShuttle1] setEditInSht1Right
 *   setEditInSht2Left          teach      [MInShuttle2] setEditInSht2Left
 *   setEditInSht2Right         teach      [MInShuttle2] setEditInSht2Right
 *   setEditLoadPort1Z          teach      [MLoadRobotZ] setEditLoadPort1Z
 *   setEditLoadPort2Z          teach      [MLoadRobotZ] setEditLoadPort2Z
 *   setEditLoadPort3Z          teach      [MLoadRobotZ] setEditLoadPort3Z
 *   setEditLoadPort4Z          teach      [MLoadRobotZ] setEditLoadPort4Z
 *   setEditLoadPortBufferZ     teach      [MLoadRobotZ] setEditLoadPortBufferZ
 *   setEditOutRA               teach      [MOutRotateKit] setEditOutRA
 *   setEditOutSht1KitPos       teach      [MInShuttle1] setEditOutSht1KitPos
 *   setEditOutSht1OneRowKit    teach      [MInShuttle1] setEditOutSht1OneRowKit
 *   setEditOutSht2KitPos       teach      [MInShuttle2] setEditOutSht2KitPos
 *   setEditOutSht2OneRowKit    teach      [MInShuttle2] setEditOutSht2OneRowKit
 *   setEditRotateA             teach      [MInRotateKit] setEditRotateA
 *   setEditRotateOutA          teach      [MOutRotateKit] setEditRotateOutA
 *   setEditSht1BusyHalf        teach      [MInShuttle1] setEditSht1BusyHalf
 *   setEditSht2BusyHalf        teach      [MInShuttle2] setEditSht2BusyHalf
 *   setEmptyZUp                teach      [MTrayZ] setEmptyZUp
 *   setLoaderZUp               teach      [MTrayZ] setLoaderZUp
 *   setYCarPos                 teach      [MLoaderY] setYCarPos
 *   setYOCRPos                 teach      [MLoaderY] setYOCRPos
 *   setYSurePos                teach      [MLoaderY] setYSurePos
 * --------------------------------------------------------------------------- */
