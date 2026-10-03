# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_VacuumUnit.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_VacuumUnit
#
# Steven 團隊 20260925：golden TfVacuumUnit（VacuumUnit\VacuumUnit.cpp，V912，603 行）—— HW.VacuumUnit.html。
# HTEditList elVacuumUnit（golden main.cpp:1542 new；Initial :32-136 註冊）→ <recipe>\HandlerCondition.Data：
#   [Vacuum Threshold] IndexArm1_<row>_<col>／IndexArm2_…／InArm_…／OutArm_… → TestIF_File.iVaccumThrd*[row][col]（cprod.h:2564-2567）
#   [Tray] CheckBox1／Edit1 → 表單成員 bTest1／iTest1（golden 的測試元件，照註冊）
# 結構名加前綴 TestIF_File_：資料是 TestIF_File 的欄位；另一位工程師的 Setup.Cleaning（也寫 HandlerCondition.Data）用別的名字。
#
# 面板：golden Initial 執行期 new TMyVacuumPanel（MyVacuumPanel.cpp:16-272：IO 位址、畫布、DO 按鈕、myld1）。C 路只用得到
#   GroupBox（Visible：SetPanelPos／ShowSuckMode 設）與 edSV（elVacuumUnit 的來源元件）→ decls 的 VU_TMyVacuumPanel 具名替身，
#   名稱＝頁面 id（web/page/hwidgets.js makeVacuumPanel：myPal<Arm>_<iCol>_<iRow>、…_edSV）。
# 存檔鈕 spbSaveClick（:378）＝ A02 守衛 → SaveSetupFile（elVacuumUnit->SaveEditTextToFile → BackupSetupFile）→ ReadFile → SECS。
# 硬體動作不轉（網頁停用，web/page/ht9045_vacuumunit_c.js）：btnSV（MyVacuumPanel.cpp:385 WriteVaccumThreshold → ECAT-VC8）、
#   btnSetInArmClick（:551，同上＋改 edSV）、bplOn／bplOff（MyVacuumPanel.cpp:391 Acm_DaqDoSetBitEx 吸／破真空）、
#   sbResetClick（:409，面板事件狀態＋下一個 tick 重寫 VC8 閥值模式）、tmr1Timer（:255 讀 VC8 現值／閥值／DI）。
_F = 'TfVacuumUnit'


def _mk(arr, scr):
    return ('EL<TControl>("%s", "%s"); for(int iCol=0; iCol<{MAX}; iCol++) for(int iRow=0; iRow<2; iRow++) '
            '%s[iCol][iRow]=new VU_TMyVacuumPanel("%s", iCol, iRow, "%s");' % (_F, scr, arr, arr, scr))


_INDEX = ('EL<TEdit>("%s", "edSV"); ' % _F +
          _mk('myPalArm1', 'scrlbxIndexArm1').replace('{MAX}', 'iIndexColMax') + ' ' +
          _mk('myPalArm2', 'scrlbxIndexArm2').replace('{MAX}', 'iIndexColMax'))
_INOUT = (_mk('myPalInArm', 'scrlbxInArm').replace('{MAX}', 'iInOutColMax') + ' ' +
          _mk('myPalOutArm', 'scrlbxOutArm').replace('{MAX}', 'iInOutColMax'))

# golden ShowSuckMode :170-186 照原句，只加兩個守衛：golden 陣列是 [TOTAL_VACUUM_UNIT][2]、只建了 iIndexColMax 欄；
# FTestSuck.iMaxCol 大於 iIndexColMax（例 HT9050：NEW_MAX_Index_Col=8、USE_46_SUCKER_DB=0 → iIndexColMax=4）時 golden 會
# 取到 NULL 的 myPalArm2[iCol][iRow]->GroupBox（BCB：access violation，FormShow 在這裡中斷）。伺服器端 NULL 取值會讓 wb_serve 當掉
# → 跳過並記 todo（開頁回應 session.todo 看得到）。
_SUCK = ('for(int iRow=0; iRow<FTestSuck.iMaxRow; iRow++) for(int iCol=0; iCol<FTestSuck.iMaxCol; iCol++) { '
         'if(iCol>=TOTAL_VACUUM_UNIT || iRow>=2 || myPalArm2[iCol][iRow]==NULL || myPalArm1[iCol][iRow]==NULL) { '
         'AnsiString t; t.sprintf("golden VacuumUnit.cpp:170-186 ShowSuckMode: FTestSuck %dx%d but only %d index columns were created '
         '(iIndexColMax) -- golden would access-violate on myPalArm[%d][%d]; skipped", FTestSuck.iMaxRow, FTestSuck.iMaxCol, iIndexColMax, iCol, iRow); '
         'filerw::ELTodo(t.c_str()); continue; } '
         'if(iRow<FTestSuck.iShtRow && iCol<FTestSuck.iShtCol) { myPalArm2[iCol][iRow]->GroupBox->Visible=true; myPalArm1[iCol][iRow]->GroupBox->Visible=true; } '
         'else { myPalArm2[iCol][iRow]->GroupBox->Visible=false; myPalArm1[iCol][iRow]->GroupBox->Visible=false; } }')

STRUCT = {
    'struct': 'TestIF_File_VacuumUnit',
    'prefix': 'VU',
    'class': _F,
    'cpp': 'VacuumUnit\\VacuumUnit.cpp',
    'h': 'VacuumUnit\\VacuumUnit.h',
    'files': ['<recipe>\\HandlerCondition.Data [Vacuum Threshold]／[Tray]'],
    'lists': ['elVacuumUnit'],
    'methods': ['TfVacuumUnit', 'Initial', 'SetPanelPos', 'ShowSuckMode', 'FormShow', 'FormClose', 'ReadFile',
                'DoIniDataToForm', 'SaveSetupFile', 'spbSaveClick'],
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TfVacuumUnit': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': ''},
    'members': [
        'AnsiString LastFileName;   // golden VacuumUnit.h:77',
        'bool fShow=false;          // golden VacuumUnit.h:92',
        'int iCount=0;              // golden VacuumUnit.h:93（建構子 :29 設 5）',
        'int iIndexColMax=0;        // golden VacuumUnit.h:100（Initial :38-48 設）',
        'int iInOutColMax=0;        // golden VacuumUnit.h:101',
        'int iTest1=0;              // golden VacuumUnit.h:102（elVacuumUnit [Tray] Edit1；VCL 表單記憶體清零 → 0）',
        'bool bTest1=false;         // golden VacuumUnit.h:103（elVacuumUnit [Tray] CheckBox1）',
        'VU_TMyVacuumPanel *myPalArm2  [TOTAL_VACUUM_UNIT][2];     // golden VacuumUnit.h:81（TMyVacuumPanel* → 替身，見 decls）',
        'VU_TMyVacuumPanel *myPalArm1  [TOTAL_VACUUM_UNIT][2];     // golden VacuumUnit.h:82',
        'VU_TMyVacuumPanel *myPalInArm [TOTAL_VACUUM_UNIT/2][2];   // golden VacuumUnit.h:83',
        'VU_TMyVacuumPanel *myPalOutArm[TOTAL_VACUUM_UNIT/2][2];   // golden VacuumUnit.h:84',
        '#define FTestSuck VU_KitOf(1)   // golden FTestSuck.iShtRow／iShtCol／iMaxRow／iMaxCol（ShowSuckMode）→ FileRW/_KitSuck.cpp',
    ],
    'replace': [
        ('Initial', 52, 60, 'grpIndexArm2／grpIndexArm1 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排（vclcompat TControl 沒有 Width／Height）', ';'),
        ('Initial', 62, 89, 'new TMyVacuumPanel(this,kind,iCol,iRow)＋GroupBox->Parent=scrlbxIndexArm1/2：面板是執行期元件（IO 位址／畫布／DO 按鈕，'
         'MyVacuumPanel.cpp:16-272）。C 路只需要 GroupBox（Visible）與 edSV → VU_TMyVacuumPanel 具名替身（名稱＝頁面 id）；'
         'HT9046LS（kind 0/1）與 HT9045（kind 4/5）兩支只差 IO 位址與吸嘴排序，替身相同。樣板 edSV（DFM Text=\'0\'）先建，面板 edSV 照 MyVacuumPanel.cpp:183 抄它',
         _INDEX),
        ('Initial', 91, 99, 'grpInarm／grpOutarm 的 Top／Left／Width／Height：GroupBox 版面，HTML 自己排', ';'),
        ('Initial', 100, 110, 'new TMyVacuumPanel(this,2/3,iCol,iRow)＋GroupBox->Parent=scrlbxInArm/OutArm：同上（In／Out Arm 面板）', _INOUT),
        ('ShowSuckMode', 170, 186, 'golden 原句＋陣列邊界／NULL 守衛（golden 會 access violation 的情形改成跳過＋todo，見本檔 _SUCK 註解）', _SUCK),
        ('FormShow', 216, 216, 'Caption=S：視窗標題（HTML 不用）', ';'),
        ('FormShow', 217, 220, 'this->Top／Left／Width／Height：視窗位置大小（HTML 不用）', ';'),
        ('FormShow', 222, 222, 'tmr1->Enabled=true：tmr1Timer（:255）每 tick 讀 ECAT-VC8 現值／閥值／DI（MyLaneIO／Acm_*）＝即時硬體顯示，不是讀寫檔', ';'),
        ('FormShow', 227, 252, 'myPal*->RefreshDOIO()：讀 ECAT-VC8 DO（Acm_DaqDoGetBitEx，MyVacuumPanel.cpp:467）更新 bplOn／bplOff 顯示＝即時硬體顯示；'
         '其後 :248-252 golden 本來就是註解', ';'),
        ('FormClose', 308, 308, 'tmr1->Enabled=false：同 FormShow :222（即時硬體顯示）', ';'),
        ('spbSaveClick', 384, 384, 'Close()：golden A02 權限不足時關表單 → 記 closed（PageSave 沒寫檔就跑 golden FormClose 的 ReadFile 還原）',
         'filerw::ELMark("closed");'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'vclcompat/SysUtils.h', 'forms/fMain.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'decls': [
        'const int TOTAL_VACUUM_UNIT=8;     // golden VacuumUnit.h:21（移植樹 VacuumUnit/VacuumUnit.h 帶 MyVacuumPanel.h 等真元件，本 TU 不 include）',
        'const int VACUUM_UNIT_WIDTH=81;    // golden VacuumUnit.h:22（SetPanelPos 的 Left，只存值）',
        'const int VACUUM_UNIT_HEIGHT=177;  // golden VacuumUnit.h:23',
        'void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket，1＝FTestSuck）',
        '// golden TMyKitSuck 用到的四個欄位的轉接物件：FTestSuck 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
        'struct VU_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };',
        'static inline VU_Kit VU_KitOf(int w) { VU_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }',
        '// golden TMyVacuumPanel（VacuumUnit/MyVacuumPanel.h:12）的 C 路替身：只有 golden VacuumUnit.cpp 讀寫的兩個成員。',
        '//   GroupBox：golden ctor :151 new、:159 Visible=false；Parent＝scrlbx*（VacuumUnit.cpp:69/72/85/88/105/108）',
        '//   edSV    ：golden ctor :153 new、:178 Parent=GroupBox、:183 Text＝樣板 edSV->Text（DFM \'0\'）；elVacuumUnit 的來源元件',
        '//   名稱＝頁面 id：<arr>_<iCol>_<iRow>（面板）、<arr>_<iCol>_<iRow>_edSV（web/page/hwidgets.js makeVacuumPanel）',
        'struct VU_TMyVacuumPanel {',
        '    TGroupBox *GroupBox;',
        '    TEdit     *edSV;',
        '    VU_TMyVacuumPanel(const char* arr, int iCol, int iRow, const char* parent) {',
        '        AnsiString gb; gb.sprintf("%s_%d_%d", arr, iCol, iRow);',
        '        AnsiString sv=gb+"_edSV";',
        '        GroupBox=filerw::EL<TGroupBox>("TfVacuumUnit", gb.c_str());',
        '        GroupBox->Visible=false;                                                         // golden MyVacuumPanel.cpp:159',
        '        edSV=filerw::EL<TEdit>("TfVacuumUnit", sv.c_str());',
        '        edSV->Text=filerw::EL<TEdit>("TfVacuumUnit", "edSV")->Text;                      // golden MyVacuumPanel.cpp:183',
        '        const char* pr[2][2]={{sv.c_str(), gb.c_str()}, {gb.c_str(), parent}};          // golden MyVacuumPanel.cpp:178；VacuumUnit.cpp:69 等',
        '        filerw::ELSetParents("TfVacuumUnit", pr, 2);',
        '    }',
        '};',
    ],
    'overrides': [],
}
