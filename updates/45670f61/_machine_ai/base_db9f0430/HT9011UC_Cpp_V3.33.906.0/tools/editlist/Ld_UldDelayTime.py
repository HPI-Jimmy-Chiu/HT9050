# -*- coding: utf-8 -*-
# tools/editlist/Ld_UldDelayTime.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only Ld_UldDelayTime]
# Steven 20260924：Ld_UldDelayTime（UdUld.Data，elUdUld）—— golden TfLd_ULd（cLd_ULd.cpp，263 行）。
# 建構子就是註冊（golden 在 HT9045.cpp:191 CreateForm 時跑，早於 TfConfiguration :207）；
# 開頁 FormShow（ReadFile＋權限），存檔鈕 spbSaveClick → SaveSetupFile → elUdUld->SaveEditTextToFile。
STRUCT = {
    'struct': 'Ld_UldDelayTime',
    'prefix': 'LU',
    'class': 'TfLd_ULd',
    'cpp': 'cLd_ULd.cpp',
    'h': 'cLd_ULd.h',
    'files': ['UdUld.Data'],
    'lists': ['elUdUld'],
    'methods': ['TfLd_ULd', 'FormShow', 'ReadFile', 'DoIniDataToForm', 'FormClose', 'spbSaveClick', 'SaveSetupFile'],
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TfLd_ULd': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': ''},
    'members': ['AnsiString LastFileName;   // golden cLd_ULd.h: TfLd_ULd::LastFileName',
                'bool fShow=false;          // golden cLd_ULd.h: TfLd_ULd::fShow'],
    'replace': [
        ('FormShow', 90, 90, 'LoadImage()：示意圖（HTML 自己有）', ';'),
        ('FormShow', 94, 95, 'Caption／SetDefaultPos()：視窗標題與位置（HTML 不用）', ';'),
        ('FormClose', 173, 173, 'rbTemp->SetFocus()：焦點（HTML 端）', ';'),
        ('spbSaveClick', 185, 185, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關',
         'filerw::ELMark("closed");'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'blocks': [],
    'overrides': [],
}
