# -*- coding: utf-8 -*-
# tools/formbridge/TfHotPlate.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py [--only TfHotPlate]
FORM = {
    'class': 'TfHotPlate',
    'cpp': 'cHotPlate.cpp',
    'h': 'cHotPlate.h',
    'page': 'Setup.HotPlate.html',
    'struct': 'HotPlateForm_File',
    'files': ['HotPlate.Data'],
    # golden 開表單＝FormShow（:35，內含 ReadFile＋DoIniDataToForm＋依條件改 Checked／Enabled）；
    # 存檔鈕＝spbSaveClick（:440，內含 SaveSetupFile＋存後 ReadFile）
    'methods': ['FormShow', 'DoIniDataToForm', 'spbSaveClick', 'SaveSetupFile'],
    'members': ['fShow'],
    'display': ['B_FormShow(J);'],
    'save': 'B_SaveSetupFile',
    'saveFlow': 'B_spbSaveClick',
    # 讀檔器 TfHotPlate::ReadFile 已在移植樹（forms/fHotPlate.cpp），golden 呼叫它的地方接過去
    'port_calls': {'ReadFile': 'fHotPlate->ReadFile'},
    'sourceGap': '',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'forms/fMain.h', 'forms/fHotPlate.h', 'LastSet.h', 'csystem.h', 'cAuthority.h',
                 'forms/fSecurity.h', 'aHotPlateSubstrate.h', 'ainarm_SearchPlacePlate.h'],
    'blocks': [
        # golden :47-58 從 Plate 資料庫（fConfiguration->strngrdHP，CSV）列出選項。
        # 移植樹的 fConfiguration 沒有 strngrdHP 這張表；bHasPlateCSV 開著時照實回報，不假裝有選項。
        ('FormShow', 47, 58, 'if(bHasPlateCSV)',
         'if(bHasPlateCSV)                                                            //Steven 20210629 : Plate Form改成CSV\n'
         '    {\n'
         '        J.Todo("golden FormShow :47-58 cbSelectHPFromDB items from fConfiguration->strngrdHP (Plate CSV) not ported");\n'
         '    }'),
    ],
    'overrides': [
        # golden :38 LoadImage() 載入示意圖（純畫面）
        ('FormShow', 'LoadImage();', '/* golden: LoadImage() —— 示意圖，HTML 自己有 */'),
        # golden :478 fMain->ChangeATCSiteUse()：移植樹 TfMain 沒有本體（forms/fMain.cpp:921 GATE W906-HOME-W1-ATCSITE）
        ('spbSaveClick', 'fMain->ChangeATCSiteUse();',
         'J.Todo("golden spbSaveClick: fMain->ChangeATCSiteUse() has no body in the port (GATE W906-HOME-W1-ATCSITE)");'),
    ],
}
