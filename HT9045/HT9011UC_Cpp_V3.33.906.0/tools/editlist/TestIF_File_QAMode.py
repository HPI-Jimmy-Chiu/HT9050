# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_QAMode.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_QAMode
#
# Steven 團隊 20260925：TestIF_File 的 TfQAMode 半邊（<recipe>\Tester.Data [QA Mode]／[QA Sampling]）——
# golden TfQAMode（QAMode.cpp，V912，337 行）。結構名加後綴 _QAMode：TestIF_File 已有 A 形狀 FileRW/TestIF_File.cpp（TFTestIF）
# 與 C 形狀 TestIF_File_YieldMonitoring／TestIF_File_SetUp；另一位工程師的 Setup.TesterIF 用別的名字。
# 沒有 HTEditList：存檔鈕 btnApplyClick（:73）＝ A02 守衛 → DoFormToData（:56，widget → TestIF_File）→ 12 個
# WriteIniData（:92-105）→ fMain->BackupSetupFile → ReadFile → DoIniDataToForm → fBinSel->ReadFile。
# C 路 PageSave 在跑 golden 之前先擋 mustSend 缺值（400），所以 golden DoFormToData 直接寫機台正在用的 TestIF_File 也安全。
# 開頁 FormShow（:113）＝ golden 重建 cbQAModeBin／cbbQASampleTray 的 Items → ReadFile（:179）→ DoIniDataToForm → 權限／顯示。
#
# ⚠ golden btnApplyClick **不寫** config.ini。config.ini [Index] QAMode_Backup* 是 QABackupStatus（:286-336）寫的，它的呼叫點是
#   TfMain::SetRunStartMode（main.cpp:546-551，CosFunction.bVerifyMode 且切進／切出 rsmQAMode），不是這張表單 ——
#   照 V912 的讀寫位置，這裡不轉（見 FileRW/TestIF_File_QAMode.cpp 檔頭）。
_F = 'TfQAMode'

STRUCT = {
    'struct': 'TestIF_File_QAMode',
    'prefix': 'QA',
    'class': _F,
    'cpp': 'QAMode.cpp',
    'h': 'QAMode.h',
    'files': ['<recipe>\\Tester.Data [QA Mode]／[QA Sampling]'],
    'lists': [],
    'methods': ['TfQAMode', 'FormShow', 'FormClose', 'ReadFile', 'DoIniDataToForm', 'DoFormToData', 'btnApplyClick',
                'ShowTrayDirectIMG'],
    'save_methods': ['btnApplyClick', 'DoFormToData'],
    'params': {'TfQAMode': '', 'FormShow': '', 'FormClose': '', 'btnApplyClick': ''},
    'members': ['bool fShow=false;   // golden QAMode.h:58（本 TU 自己的；移植樹 fQAMode->fShow 是 forms/fQAMode.h 的另一個，Command.cpp 的讀者仍是 #if 0）'],
    'replace': [
        ('btnApplyClick', 79, 79, 'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（PageSave 沒寫檔就照 golden FormClose 還原替身）',
         'filerw::ELMark("closed");'),
        ('FormShow', 126, 127, 'Left／Top：視窗位置（HTML 不用；vclcompat 沒有 Width／Height）', ';'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fLotInfo.h', 'forms/fBinSel.h'],
    'decls': ['void MyDBIProcess(AnsiString S1, AnsiString S2);   // golden aHotPlateSubstrate.h:924（aHotPlateSubstrate.h 與 HTEditList.h 衝突，只宣告這一支；ShowTrayDirectIMG :274）'],
    'overrides': [],
}
