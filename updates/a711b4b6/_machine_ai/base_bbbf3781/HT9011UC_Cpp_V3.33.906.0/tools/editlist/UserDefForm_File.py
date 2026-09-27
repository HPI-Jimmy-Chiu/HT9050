# -*- coding: utf-8 -*-
# tools/editlist/UserDefForm_File.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only UserDefForm_File]
# Steven 20260924：UserDefForm_File（Tray.Data，elTrayForm）—— golden TfTrayForm（cTrayForm.cpp，1131 行）。
# 移植樹其他程式還在讀 fTrayForm 的真元件（uPAT_Function 讀 TrayName1->Text、csystem／fBuilder 呼叫
# fTrayForm->SaveSetupFile、TrayEdit[][]）→ 'adopt'：golden 名稱與型別都對得上的，替身就是移植樹那個物件
# （ELKeep 登記），兩邊共用同一份值；其餘才是新替身。
STRUCT = {
    'struct': 'UserDefForm_File',
    'prefix': 'TF',
    'class': 'TfTrayForm',
    'cpp': 'cTrayForm.cpp',
    'h': 'cTrayForm.h',
    'files': ['Tray.Data'],
    'lists': ['elTrayForm'],
    'adopt': {'object': 'fTrayForm', 'header': 'forms/fTrayForm.h'},
    'methods': ['TfTrayForm', 'FormShow', 'DoIniDataToForm', 'ReadFile', 'FormClose', 'spbSaveClick', 'SaveSetupFile'],
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TfTrayForm': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': ''},
    'members': ['bool fShow=false;   // golden cTrayForm.h:274 TfTrayForm::fShow',
                '#define TrayEdit (fTrayForm->TrayEdit)       // golden cTrayForm.h:249 —— 移植樹 fTrayForm 那一份（它的 SaveSetupFile 也用）',
                '#define asErrorMsg (fTrayForm->asErrorMsg)   // golden cTrayForm.h:279 —— asendic_Loader.cpp 讀 fTrayForm->asErrorMsg'],
    'replace': [
        ('TfTrayForm', 127, 138,
         '色感 MU-N GUI（uColorSensorInfo，移植樹沒有這個類別）：資料註冊照 golden，掛到移植樹的替代 TEdit（同 cTrayForm.cpp Init()）',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { '
         'elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorFTEnable"), &TestIF_File.sLoaderColorSenFTEnable, ECText, "Color Sensor", "sLoaderColorSenFTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); '
         'elTrayForm->Add(EL<TEdit>("TfTrayForm", "edColorSensorRTEnable"), &TestIF_File.sLoaderColorSenRTEnable, ECText, "Color Sensor", "sLoaderColorSenRTEnable", bNoShow, bEnable, bReadFromFile, "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0"); '
         'filerw::ELTodo("golden cTrayForm.cpp:129/:132-133 color sensor GUI (uColorSensorInfo) + LoadColorSensorMapping not ported"); }'),
        ('FormShow', 144, 144, 'LoadImage()：示意圖（HTML 自己有）', ';'),
        ('FormShow', 148, 148, 'Caption：視窗標題', ';'),
        ('FormShow', 150, 150, 'UpDateType()：「從 Type? 複製」下拉的選項（依目前分頁，頁面端的複製功能）', ';'),
        ('FormShow', 162, 175, 'bHasTrayCSV：從 fConfiguration->strngrdTray（Tray CSV）列出選項 —— 移植樹 fConfiguration 沒有這張表',
         'if(bHasTrayCSV) { filerw::ELTodo("golden cTrayForm.cpp:162-175 cbTrayType* items from fConfiguration->strngrdTray (Tray CSV) not ported"); }'),
        ('FormShow', 271, 281, '色感分頁顯示＋LoadColorSensorMapping（缺鍵會寫 ColorSensorType.ini）＋視窗位置',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=true; filerw::ELTodo("golden cTrayForm.cpp:274 LoadColorSensorMapping not ported"); } '
         'else { EL<TTabSheet>("TfTrayForm", "tsColorSensor")->TabVisible=false; }'),
        ('DoIniDataToForm', 330, 335, 'TMyTray1..3（TTMyTray 盤面示意圖）的 XItem／YItem：純畫面', ';'),
        ('ReadFile', 449, 450, 'rgDeviceDirClick／rgTrayDirClick：只切換示意圖與 fShowMessage 的圖（純畫面）', ';'),
        ('ReadFile', 452, 452, 'LoadColorSensorEnable()：色感 GUI 勾選框（uColorSensorInfo 未移植；值在 TestIF_File 字串，已由 elTrayForm 讀）', ';'),
        ('spbSaveClick', 616, 616, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");'),
        ('SaveSetupFile', 646, 646, 'SaveColorSensorEnable()：色感 GUI 勾選框 → TestIF_File 字串（uColorSensorInfo 未移植）',
         'if(USE_COLORSENSOR_MUN==eCSMUN_Install_Loader) filerw::ELTodo("golden cTrayForm.cpp:646 SaveColorSensorEnable (uColorSensorInfo GUI) not ported");'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fTrayForm.h', 'csystem.h',
                 'BarcodeReader.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的（同 IniConfig）
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）'],
    'blocks': [],
    'overrides': [],
}
