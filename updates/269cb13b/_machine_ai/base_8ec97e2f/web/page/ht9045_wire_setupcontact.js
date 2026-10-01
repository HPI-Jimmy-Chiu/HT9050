/* ht9045_wire_setupcontact.js -- Setup.Contact.html 的接線資料（行為在 ht9045_wire_engine.js）
 * ---------------------------------------------------------------------------
 * //Steven 20260918  （這個檔是產生的；標記由 gen_wire.py 的樣板寫入，重跑後仍在）
 * 當日完整變更紀錄：D:\docs\ChangeLog\CHANGES_20260918_Steven.md
 * ---------------------------------------------------------------------------
 * AI(W906-FW-GEN) 20260914：由 scratchpad/gen_wire.py 從 golden 機械產生。
 * 手改這個檔會在下次重跑產生器時被覆蓋 —— 要改請改產生器或 golden。
 *
 * 來源表單    cContact.dfm
 * 抽取法      WriteIniData / WriteIniDataGeneral / WriteIniDataNoLog
 *             （widget 與區段/鍵同一行）＋ HTEditList 一跳法，
 *             全部限定在該表單自己的 .cpp，不做全樹 id 反查。
 *             目標檔由呼叫的第 1 個參數解析（General 家族固定 Gerneral.ini），
 *             不假設「一頁一個檔」—— uteach 同一表單就寫兩個檔。
 * 小鍵盤      golden ShowQwertyKey(Sender, N_FLAGS, dp, checkRange, min, max)
 *             配上 .dfm 的 OnMouseDown = <handler>。
 *
 * 必接 28 個（該鍵在所有擁有該文件的配方裡都存在）
 * 選用 15 個（部分配方才有；讀取時缺了不算錯，存檔時自動略過）
 * 未接 4 個（見檔尾 PENDING —— 這些鍵在任何配方檔裡都不存在，抽取有誤）
 * 小鍵盤 42 個欄位有 golden 依據
 * ⚠ 其中 2 個的 golden 範圍是負值或 max<=0。golden 的
 *   myQwertyKeyBoard.cpp:252 會把那種範圍整組換成 0~65535，
 *   用在負值欄位上會把輸入夾成 0。本檔一律關掉那些夾限，
 *   欄位仍可輸入，只是少一道範圍檢查。
 */
HT9045Wire.register({
  page: 'Setup.Contact.html',
  slug: 'setupcontact',
  fields: {
    edAirForce:                  ['contact', 'Torque Control', 'Torque'],
    edAutoContactTest:           ['contact', 'Height Calibration', 'Auto Contact Test Count'],
    edContactBackUp1:            ['contact', 'Test Arm1', 'ContactBackUp'],
    edContactBackUp2:            ['contact', 'Test Arm2', 'ContactBackUp'],
    edContactHeight1:            ['contact', 'Test Arm1', 'Contact'],
    edContactHeight2:            ['contact', 'Test Arm2', 'Contact'],
    edContactOffsetArm1:         ['contact', 'Test Arm1', 'Contact'],
    edContactOffsetArm2:         ['contact', 'Test Arm2', 'Contact'],
    edD41:                       ['contact', 'Mode', 'fSocketInitialICCheckPositionOffset'],
    edDropOffset1:               ['contact', 'Test Arm1', 'Drop'],
    edDropOffset2:               ['contact', 'Test Arm2', 'Drop'],
    edDropSpeed:                 ['contact', 'Wait Time', 'Drop Speed'],
    edDropWaitTime:              ['contact', 'Wait Time', 'Drop Wait'],
    edForcePerPinG:              ['contact', 'Torque Control', 'Force Per Pin Kg'],
    edForcePerPinN:              ['contact', 'Torque Control', 'Force Per Pin'],
    edOrgPick1:                  ['contact', 'Test Arm1', 'ShuttlePickBackUp'],
    edOrgPick2:                  ['contact', 'Test Arm2', 'ShuttlePickBackUp'],
    edPickUp1:                   ['contact', 'Test Arm1', 'Pick Up'],
    edPickUp2:                   ['contact', 'Test Arm2', 'Pick Up'],
    edPinCount:                  ['contact', 'Torque Control', 'Pin Number'],
    edReleaseHeight1:            ['contact', 'Test Arm1', 'Place'],
    edReleaseHeight2:            ['contact', 'Test Arm2', 'Place'],
    edShtPickOffset1:            ['contact', 'Test Arm1', 'Pick Up'],
    edShtPickOffset2:            ['contact', 'Test Arm2', 'Pick Up'],
    edXDimension:                ['contact', 'Torque Control', 'X Dimension'],
    edYDimension:                ['contact', 'Torque Control', 'Y Dimension'],
    edtTorqueCmp:                ['contact', 'Torque Control', 'iIndexTorqueCmp'],
    edtTorqueMax:                ['contact', 'Torque Control', 'iIndexTorqueMax'],
  },
  optional: {
    edDieForcePerPinG:           ['contact', 'Torque Control', 'Die Force Per Pin Kg'],   // 5/215 個配方有
    edDieForcePerPinN:           ['contact', 'Torque Control', 'Die Force Per Pin'],   // 5/215 個配方有
    edDoubleForce:               ['contact', 'Torque Control', 'Double Force'],   // 214/215 個配方有
    edDropByPassDetect:          ['contact', 'Mode', 'dDropByPassDetect'],   // 195/215 個配方有
    edSidePushWaitTime:          ['contact', 'Wait Time', 'Side Push Wait Time'],   // 179/215 個配方有
    edTestContactCount:          ['contact', 'Height Calibration', 'Test Contact Count'],   // 2/215 個配方有
    edTestSec:                   ['contact', 'Height Calibration', 'Test Sec'],   // 2/215 個配方有
    edUpOffset1:                 ['contact', 'Test Arm1', 'Up'],   // 210/215 個配方有
    edUpOffset2:                 ['contact', 'Test Arm2', 'Up'],   // 210/215 個配方有
    edUpSpeed:                   ['contact', 'Wait Time', 'Up Speed'],   // 210/215 個配方有
    edUpWaitTime:                ['contact', 'Wait Time', 'Up Wait'],   // 210/215 個配方有
    edtPinOfDie:                 ['contact', 'Torque Control', 'Pin of Die'],   // 213/215 個配方有
    edtPurgeBdforePickShuttleOffSet: ['contact', 'Mode', 'Air Purge Before Pick Shuttle OffSet'],   // 102/215 個配方有
    edtPurgeBeforePickShuttleInterval: ['contact', 'Mode', 'Air Purge Before Pick Shuttle Interval'],   // 102/215 個配方有
    edtPurgeBeforePickShuttleTime: ['contact', 'Mode', 'Air Purge Before Pick Shuttle Time'],   // 102/215 個配方有
  },
  pending: {
    edAutoKSHTReleaseOfs:        ['?', 'Mode', 'AutoKSHTOfs', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edLoadCellHeight1:           ['?', 'Test Arm1', 'LoadCellZ1', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edLoadCellHeight2:           ['?', 'Test Arm2', 'LoadCellZ2', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
    edOutDiameter:               ['?', 'SLK Type', 'OutDiameter', '這個鍵在任何配方檔裡都不存在（抽取可能有誤）'],
  },
  kb: {
    edAutoKSHTReleaseOfs:        ['DOUBLE', 2, true, 10, 2],
    edContactHeight1:            ['DOUBLE', 3, false, 0, 0],
    edContactHeight2:            ['DOUBLE', 3, false, 0, 0],
    edContactOffsetArm1:         ['DOUBLE', 2, false, 0, 0],
    edContactOffsetArm2:         ['DOUBLE', 2, false, 0, 0],
    edContactRelativeZ1:         ['DOUBLE', 3, false, 0, 0],
    edContactRelativeZ2:         ['DOUBLE', 3, false, 0, 0],
    edD41:                       ['DOUBLE', 2, false, 0, 0],
    edDieForcePerPinG:           ['DOUBLE', 4, false, 0, 0],
    edDieForcePerPinN:           ['DOUBLE', 4, false, 0, 0],
    edDoubleForce:               ['DOUBLE', 2, false, 0, 0],
    edDropByPassDetect:          ['DOUBLE', 2, false, 0, 0],
    edDropOffset1:               ['DOUBLE', 3, false, 0, 0],
    edDropOffset2:               ['DOUBLE', 3, false, 0, 0],
    edDropSpeed:                 ['DOUBLE', 3, false, 0, 0],
    edDropWaitTime:              ['DOUBLE', 3, false, 0, 0],
    edForcePerDeviceKG:          ['DOUBLE', 2, true, 1.0, 120.0],
    edForcePerPinG:              ['DOUBLE', 4, false, 0, 0],
    edForcePerPinN:              ['DOUBLE', 4, false, 0, 0],
    edLoadCellHeight1:           ['DOUBLE', 3, false, 0, 0],
    edLoadCellHeight2:           ['DOUBLE', 3, false, 0, 0],
    edOrgPick1:                  ['DOUBLE', 3, false, 0, 0],
    edOrgPick2:                  ['DOUBLE', 3, false, 0, 0],
    edPickUp1:                   ['DOUBLE', 3, false, 0, 0],
    edPickUp2:                   ['DOUBLE', 3, false, 0, 0],
    edReleaseHeight1:            ['DOUBLE', 3, false, 0, 0],
    edReleaseHeight2:            ['DOUBLE', 3, false, 0, 0],
    edShtPickOffset1:            ['DOUBLE', 2, false, 0, 0],
    edShtPickOffset2:            ['DOUBLE', 2, false, 0, 0],
    edSidePushWaitTime:          ['DOUBLE', 3, false, 0, 0],
    edUpOffset1:                 ['DOUBLE', 3, false, 0, 0],
    edUpOffset2:                 ['DOUBLE', 3, false, 0, 0],
    edUpSpeed:                   ['DOUBLE', 3, false, 0, 0],
    edUpWaitTime:                ['DOUBLE', 3, false, 0, 0],
    edXDimension:                ['DOUBLE', 3, false, 0, 0],
    edYDimension:                ['DOUBLE', 3, false, 0, 0],
    edtPinOfDie:                 ['INTEGER', 0, true, 10, 10000],
    edtPurgeBdforePickShuttleOffSet: ['DOUBLE', 3, false, 0, 0],
    edtPurgeBeforePickShuttleInterval: ['DOUBLE', 3, false, 0, 0],
    edtPurgeBeforePickShuttleTime: ['DOUBLE', 3, false, 0, 0],
    edtTorqueCmp:                ['DOUBLE', 3, false, 0, 0],
    edtTorqueMax:                ['DOUBLE', 3, false, 0, 0],
  }
});

/* ---------------------------------------------------------------------------
 * 沒接的 4 個，以及為什麼
 * ---------------------------------------------------------------------------
 * 這些鍵在「全部 216 個配方」裡一次都沒出現過 —— 那不是配方相依，
 * 是抽取器抽錯了（例如 golden 那裡是前綴+後綴串接組成的鍵，正則只
 * 抓到字面前綴）。接下去會讓 preview 回 notFound 並被規則 2 擋下，
 * 更糟的情況是誤指到另一個真實存在的鍵，把值寫錯地方。
 * 「部分配方才有」的鍵不在這裡 —— 它們在上面的 optional。
 *
 *   edAutoKSHTReleaseOfs     [Mode] AutoKSHTOfs
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edLoadCellHeight1        [Test Arm1] LoadCellZ1
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edLoadCellHeight2        [Test Arm2] LoadCellZ2
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 *   edOutDiameter            [SLK Type] OutDiameter
 *       這個鍵在任何配方檔裡都不存在（抽取可能有誤）
 * --------------------------------------------------------------------------- */
