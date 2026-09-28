# -*- coding: utf-8 -*-
# tools/editlist/ArmSpeed_File.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only ArmSpeed_File]
# Steven 20260924：ArmSpeed_File 族（ArmCondition.Data）—— golden TfSpeed（cSpeed.cpp，2517 行）。
# 沒有 HTEditList：golden 存檔鈕 spbSaveClick（:1433）自己逐鍵 WriteIniData（139 處）＋ReadWriteFile(false)，
# 不呼叫 SaveSetupFile —— A 形狀產生器（FormState 空跑 save）處理不了，改走 C 路（替身＋直接跑 golden 存檔鈕）。
# TTrackBar／TUpDown 的 Position／OnChange 照 VCL 語意（filerw::ELTrackBar）。
STRUCT = {
    'struct': 'ArmSpeed_File',
    'prefix': 'SP',
    'class': 'TfSpeed',
    'cpp': 'cSpeed.cpp',
    'h': 'cSpeed.h',
    'files': ['ArmCondition.Data'],
    'lists': [],
    'methods': ['TfSpeed', 'FormShow', 'ReadFile', 'ReadWriteFile', 'DoIniDataToForm',
                'tbAllSpeedChange', 'tbAccSpeedChange', 'tbEPControlChange', 'FormClose', 'spbSaveClick'],
    'save_methods': ['spbSaveClick', 'ReadWriteFile'],
    'params': {'TfSpeed': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'tbAllSpeedChange': '', 'tbAccSpeedChange': '', 'tbEPControlChange': ''},
    'members': ['AnsiString LastFileName;   // golden cSpeed.h: TfSpeed::LastFileName',
                'bool fShow=false;          // golden cSpeed.h: TfSpeed::fShow'],
    'replace': [
        ('TfSpeed', 38, 38, 'edtHPVacuumDelay->Hint：Default Recipe ChangeLog 用的提示字（HTML 不用）', ';'),
        ('FormShow', 48, 48, 'Caption：視窗標題', ';'),
        ('FormShow', 51, 52, 'Left／Top：視窗位置', ';'),
        ('FormShow', 208, 208, 'rbTemp->SetFocus()：焦點', ';'),
        ('FormShow', 146, 147, 'InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 → FileRW/_KitSuck.cpp 轉接（同一個判斷）',
         'if(FileRW_InOutArmSuckHasIC() || ShuttleHasIC() || IndexHasIC())'),
        ('ReadWriteFile', 797, 879,
         'LoaderUnload_StepMotor：Tray Y 步進速度全經 dmTrayMotor（移植樹沒有 dmTrayMotor，GATE S5）',
         'if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:797-879 tray-Y step motor speeds via dmTrayMotor not ported (GATE S5) -- not shown, not saved");'),
        ('spbSaveClick', 1523, 1526, 'MyMessageBox->Close()：訊息框（伺服器端沒有）', ';'),
        ('spbSaveClick', 1730, 1730, 'fShowMessage->sgdSpeedView->Refresh()：主畫面速度表重畫（HTML 端）', ';'),
        ('spbSaveClick', 1739, 1761, 'Recipe Parameter Default 比對（fRPDefault／SearchRecipeParameter 未移植）',
         'if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden cSpeed.cpp:1739-1761 Recipe Parameter Default compare not ported");'),
        ('spbSaveClick', 1784, 1784, 'dmTrayMotor->StartSetSpeed()：步進速度推到驅動器（移植樹沒有 dmTrayMotor）',
         'if(LoaderUnload_StepMotor) filerw::ELTodo("golden cSpeed.cpp:1784 dmTrayMotor->StartSetSpeed not ported");'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fObserver.h', 'csystem.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    # cAuthority.h 帶進 language.h（TWinControl 與 Public/HTEdit.h 重複）→ 只前置宣告要用的
    'decls': ['bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
              'extern bool authMainForm[12];        // cAuthority.h:56（golden cAuthority.h）',
              'extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h）'],
    'blocks': [],
    'overrides': [],
}
