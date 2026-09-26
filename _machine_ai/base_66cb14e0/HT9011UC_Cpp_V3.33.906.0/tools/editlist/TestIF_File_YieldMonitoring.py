# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_YieldMonitoring.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_YieldMonitoring
#
# Steven 團隊 20260925：TestIF_File 的 TfYieldMonitoring 半邊（<recipe>\Tester.Data）—— golden TfYieldMonitoring
# （uYieldMonitoring.cpp，912，6136 行）。結構名加後綴，因為 "TestIF_File" 已是 A 形狀 FileRW/TestIF_File.cpp（TFTestIF）。
# 沒有 HTEditList：存檔鈕 btnApplyClick（:3092）＝ DoFormToData（widget → TestIF_File）→ CheckSettingNo → SaveSetupFile
# （TestIF_File ＋少數 widget → Tester.Data）。C 路 PageSave 在跑 golden 之前先擋 mustSend 缺值（400），所以 golden
# DoFormToData 直接寫機台正在用的 TestIF_File 也不會被缺值夾成下限（A 形狀需要的暫存複本在這裡不需要）。
# 開頁 FormShow（:2581）照 golden 呼叫 golden ReadFile（:900，尾端 DoIniDataToForm）；開機讀檔仍是移植樹
# fYieldMonitoring->ReadFile()（uYieldMonitoring.cpp:2676），同 ArmSpeed_File 的做法。
# 動態元件：golden FormCreate（:340）產生 12 組 × TEST_MAX_BIN(256) 個 Category 元件（指標陣列，uYieldMonitoring.h:568-579）
# → 陣列照 golden 名稱放成成員，FormCreate 改成照 golden Name（sprintf "%03d"）建具名替身＋父容器（replace）。
# ASE_KaohSiung 的 TMyYieldPanel（:74）：GUI 格子（TTMyTray256）沒有；它的資料成員＋ReTestLimit 用 decls 的
# YM_TMyYieldPanel 代替，golden 的 ReadFile／DoFormToData／SaveSetupFile ASE 段原樣編（只擋畫格子的行）。
_N = '"TfYieldMonitoring"'
_ARR = ['cbByBinSiteGapCat_FT', 'cbByBinSiteGapCat_RT', 'edByBinSiteGapCat_FT', 'edByBinSiteGapCat_RT',
        'cbByArmSiteGapCat_FT', 'cbByArmSiteGapCat_RT', 'edByArmSiteGapCat_FT', 'edByArmSiteGapCat_RT',
        'cbByBinFailureCat_FT', 'cbByBinFailureCat_RT', 'edByBinFailureCat_FT', 'edByBinFailureCat_RT']
# golden FormCreate 每組的父容器（:350 等 ->Parent=）
_PARENT = {'cbByBinSiteGapCat_FT': 'scrlbxBinAlarm2_FT', 'cbByBinSiteGapCat_RT': 'scrlbxBinAlarm2_RT',
           'edByBinSiteGapCat_FT': 'scrlbxBinAlarm2_FT', 'edByBinSiteGapCat_RT': 'scrlbxBinAlarm2_RT',
           'cbByArmSiteGapCat_FT': 'scrlbxBinAlarm3_FT', 'cbByArmSiteGapCat_RT': 'scrlbxBinAlarm3_RT',
           'edByArmSiteGapCat_FT': 'scrlbxBinAlarm3_FT', 'edByArmSiteGapCat_RT': 'scrlbxBinAlarm3_RT',
           'cbByBinFailureCat_FT': 'scrlbxBinAlarm1_FT', 'cbByBinFailureCat_RT': 'scrlbxBinAlarm1_RT',
           'edByBinFailureCat_FT': 'scrlbxBinAlarm1_FT', 'edByBinFailureCat_RT': 'scrlbxBinAlarm1_RT'}


def _create_loop():
    s = ['AnsiString str, StrC; for(int i=0; i<TEST_MAX_BIN; i++) { str.sprintf("Category %d", i); StrC.sprintf("% *s%%", -37, str.c_str());']
    for a in _ARR:
        t = 'TCheckBox' if a.startswith('cb') else 'TEdit'
        p = _PARENT[a]
        s.append('str.sprintf("%s%%03d", i); %s[i]=EL<%s>(%s, str.c_str()); '
                 '{ const char* pr[1][2]={{str.c_str(), "%s"}}; filerw::ELSetParents(%s, pr, 1); } EL<TControl>(%s, "%s");'
                 % (a, a, t, _N, p, _N, _N, p))
        if t == 'TCheckBox':
            s.append('%s[i]->Caption=StrC;' % a)
    s.append('}')
    return ' '.join(s)


STRUCT = {
    'struct': 'TestIF_File_YieldMonitoring',
    'prefix': 'YM',
    'class': 'TfYieldMonitoring',
    'cpp': 'uYieldMonitoring.cpp',
    'h': 'uYieldMonitoring.h',
    'files': ['Tester.Data'],
    'lists': [],
    'methods': ['TfYieldMonitoring', 'FormCreate', 'FormShow', 'ReadFile', 'DoIniDataToForm', 'SetClosedSiteBin',
                'CheckSettingNo', 'FormClose', 'btnApplyClick', 'DoFormToData', 'SaveSetupFile'],
    'save_methods': ['btnApplyClick', 'DoFormToData', 'CheckSettingNo', 'SaveSetupFile'],
    'params': {'TfYieldMonitoring': '', 'FormCreate': '', 'FormShow': '', 'FormClose': '', 'btnApplyClick': ''},
    'members': ['bool bFirstCount=true;            // golden uYieldMonitoring.h:595',
                'bool bYieldCountCheckTick=false;  // golden uYieldMonitoring.h:539',
                'unsigned long ulYieldFailBase=0;  // golden uYieldMonitoring.h:540',
                'bool bAskYieldPassword=false;     // golden uYieldMonitoring.h:535',
                'bool bYieldAlarmUnlocked=false;   // golden uYieldMonitoring.h:536（伺服器端沒有 Unlock 密碼框 → 永遠 false）',
                'int iMinYield=0, iMaxYield=100, iMinCount=1, iMaxCount=100000;   // golden uYieldMonitoring.h:542-547（建構子 :296-301 設）',
                'double dMinYield=0.0, dMaxYield=100.0;',
                'bool fShow=false;                 // golden uYieldMonitoring.h:596',
                'bool bGetGPIBAutoSiteOff=false;   // golden uYieldMonitoring.h:612',
                'std::vector<YM_TMyYieldPanel*> MyYieldPanel;   // golden uYieldMonitoring.cpp:98（ASE ART 面板；資料半邊）'] +
               ['%s *%s[TEST_MAX_BIN];   // golden uYieldMonitoring.h 動態元件指標陣列（FormCreate 建替身）'
                % ('TCheckBox' if a.startswith('cb') else 'TEdit', a) for a in _ARR],
    'replace': [
        ('TfYieldMonitoring', 314, 331, '->Hint：Default Recipe ChangeLog 的提示字（vclcompat 沒有 Hint，HTML 不用）', ';'),
        ('TfYieldMonitoring', 335, 335, 'TMyYieldPanel：GUI 格子（TTMyTray256）沒有移植 → 只建資料半邊＋ReTestLimit 替身（YM_TMyYieldPanel）',
         'MyYieldPanel.push_back(new YM_TMyYieldPanel("ART"));'),
        ('FormCreate', 342, 494, '動態 Category 元件：照 golden Name（sprintf %03d）建具名替身、父容器照 golden ->Parent；'
         '位置／字型／事件（OnMouseUp 鍵盤、OnChange 亮 Apply）是純畫面', _create_loop()),
        ('FormShow', 2613, 2614, 'MyYieldPanel[0]->mtBinSelectYield（TTMyTray256 格子）寬度／欄數：純畫面，格子沒有移植', ';'),
        ('FormShow', 2618, 2618, 'ChangeData(fYieldMonitoring)：只把 OnClick/OnChange 接到 edContactCountFTChange（亮 Apply 鈕），頁面自己做', ';'),
        ('FormShow', 2621, 2622, 'Top／Left：視窗位置', ';'),
        ('ReadFile', 1033, 1033, 'TestIF_File.iYieldAlarmCheckIntervalByCount：912 欄位（rf360），移植樹 SYSTEM_TEST_IF（cprod.h）沒有',
         'filerw::ELTodo("golden uYieldMonitoring.cpp:1033 TestIF_File.iYieldAlarmCheckIntervalByCount (912 rf360) missing from port SYSTEM_TEST_IF -- [Site Yield Alarm] Check Interval By Count not read/shown");'),
        ('ReadFile', 1337, 1367, 'fTemperFrom 的 function status 面板（SetShowYield／ShowYieldFuntion）：移植樹沒有 fTemperFrom 實體（forms/fTemperFrom.h:46；同 uYieldMonitoring.cpp GATE G-YM-Temper*）',
         'if(IniConfig.bShowFunctionWindow) filerw::ELTodo("golden uYieldMonitoring.cpp:1337-1367 fTemperFrom SetShowYield/ShowYieldFuntion not ported (no fTemperFrom instance)");'),
        ('ReadFile', 1704, 1705, 'fSortCT->pnlYield／pnlYieldART：移植樹 forms/fSortCT.h 沒有這兩個面板（同 GATE G-YM-Tail (b)(c)），純畫面', ';'),
        ('DoIniDataToForm', 574, 574, 'iYieldAlarmCheckIntervalByCount 移植樹沒有（ReadFile :1033 已記 ELTodo）', ';'),
        ('DoIniDataToForm', 738, 739, 'InitDataToEdit(0)／InitmtBinSelectData()：ASE ART 格子（TTMyTray256）上色與編號，純畫面', ';'),
        ('DoFormToData', 1760, 1760, 'iYieldAlarmCheckIntervalByCount 移植樹沒有（SaveSetupFile :2139 記 ELTodo）', ';'),
        ('SaveSetupFile', 2139, 2139, 'iYieldAlarmCheckIntervalByCount 移植樹沒有 → 這個鍵不寫（檔裡原值不動）',
         'filerw::ELTodo("golden uYieldMonitoring.cpp:2139 [Site Yield Alarm] Check Interval By Count NOT written -- TestIF_File.iYieldAlarmCheckIntervalByCount (912) missing from port SYSTEM_TEST_IF");'),
        # golden FormShow :2751-2760 三元式右邊的「:gbYieldAlarm->Enabled」產生器不改寫（regex 的 lookbehind 排除 ':'，為了避開 '::'）
        # → 逐行等價改寫（同一個判斷，兩邊都是具名替身）
        ('FormShow', 2751, 2751, '三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;'),
        ('FormShow', 2752, 2752, '三元式 :gbPiggyBack 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBack")->Enabled;'),
        ('FormShow', 2753, 2753, '三元式 :gbAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:EL<TGroupBox>("TfYieldMonitoring", "gbAlarm")->Enabled;'),
        ('FormShow', 2754, 2754, '三元式 :gbYieldAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarmRT")->Enabled;'),
        ('FormShow', 2755, 2755, '三元式 :gbPiggyBackRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:EL<TGroupBox>("TfYieldMonitoring", "gbPiggyBackRT")->Enabled;'),
        ('FormShow', 2756, 2756, '三元式 :gbAlarmRT 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:EL<TGroupBox>("TfYieldMonitoring", "gbAlarmRT")->Enabled;'),
        ('FormShow', 2759, 2759, '三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_FT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;'),
        ('FormShow', 2760, 2760, '三元式 :gbYieldAlarm 產生器沒改寫（lookbehind 排除 :）→ 等價改寫', 'EL<TPageControl>("TfYieldMonitoring", "pgcBySiteByBinPercentCompare_RT")->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:EL<TGroupBox>("TfYieldMonitoring", "gbYieldAlarm")->Enabled;'),
        ('SaveSetupFile', 2365, 2379, 'Auto Clean 低良率基準重置：迴圈上限 TestSocket.iShtRow／iShtCol —— TestSocket 在 mykitsuck.h／aHotPlateSubstrate.h（與 HTEditList.h 衝突），'
         '本 TU 看不到，FileRW/_KitSuck.cpp 目前沒有轉接函式（需要 FileRW_TestSocketShtRow/Col，見交付報告）',
         'if(TestIF_File.bFailAlarmLowYield_AutoClean==true) filerw::ELTodo("golden uYieldMonitoring.cpp:2365-2379 bStandardYield reset (loop over TestSocket.iShtRow/iShtCol) not run -- TestSocket not visible in this TU, FileRW/_KitSuck.cpp adapter missing");'),
        ('btnApplyClick', 3098, 3098, 'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");'),
        ('btnApplyClick', 3125, 3128, 'SaveSetupFileToConfig(AuthPath,"config.ini")：config.ini 由 FileRW/IniConfig 擁有（C 路唯一寫者），這裡不寫',
         'if(CosFunction.bPiggybackFunctionByHandler==true) filerw::ELTodo("golden uYieldMonitoring.cpp:3127 SaveSetupFileToConfig(AuthPath, config.ini) [Alarm] Count Action/Continuous * NOT written -- config.ini is owned by FileRW/IniConfig");'),
        ('btnApplyClick', 3130, 3149, 'Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／FTestIF／SearchRecipeParameter 未移植）',
         'if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden uYieldMonitoring.cpp:3130-3149 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h', 'csystem.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h；cAuthority.h 與 HTEditList.h 衝突）',
              '// golden uYieldMonitoring.cpp:74 TMyYieldPanel 的資料半邊（ASE_KaohSiung ART）。GUI（TTMyTray256 格子、PalYield、laARTLinit）沒有移植；',
              '// ReTestLimit 是具名替身 "ReTestLimit<Name>"（golden :249 Name 規則），Items 1..10、Text "1"（golden :250-270）。',
              'struct YM_TMyYieldPanel {',
              '    bool bEnablePassYieldART=false; double fPassYieldART=0; bool bEnableOpenShortART=false; double fOpenShortYieldART=0;',
              '    bool bEnableRecoverART=false; double fRecoverYieldART=0; bool bPass[TEST_MAX_BIN]={}; bool bOpenShort[TEST_MAX_BIN]={};',
              '    TComboBox* ReTestLimit;',
              '    explicit YM_TMyYieldPanel(const char* Name) {',
              '        AnsiString str; str.sprintf("ReTestLimit%s", Name);',
              '        ReTestLimit=filerw::EL<TComboBox>("TfYieldMonitoring", str.c_str());',
              '        ReTestLimit->Text="1";',
              '        for(int i=1; i<=10; i++) ReTestLimit->Items->Add(IntToStr(i));',
              '    }',
              '};'],
    'blocks': [],
    'overrides': [],
}
