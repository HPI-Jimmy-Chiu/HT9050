# -*- coding: utf-8 -*-
# tools/formbridge/TfYieldMonitoring.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py --only TfYieldMonitoring
#
# Steven 團隊 20260924（S12 第二型，TestIF_File 擴充：TfYieldMonitoring 半邊）。
# golden uYieldMonitoring.cpp（912，cp950）。
#   顯示＝golden FormCreate（:340，動態產生 12×TEST_MAX_BIN 個 Category 元件，寫在 display）＋ FormShow（:2581，
#         內含 ReadFile→(ReadFile 尾 :1703) DoIniDataToForm）。
#   存檔鈕＝golden btnApplyClick（:3092）：DoFormToData（widget → TestIF_File）→ CheckSettingNo → SaveSetupFile（TestIF_File → Tester.Data）。
#
# ⚠ 這支和 TFTestIF／TfHotPlate 不一樣：golden SaveSetupFile 大多寫「結構」不寫 widget，widget → 結構在 DoFormToData。
#   form.save 的空跑只跑 'save'（SaveSetupFile），抓不到 DoFormToData 的缺值 → 缺值會被 atoi/atof＋CheckRange 夾成下限，
#   直接寫進「機台正在用的」TestIF_File。所以 DoFormToData 改成先寫暫存複本、沒有缺值才覆蓋真結構；有缺值 btnApplyClick 直接中止
#   （不呼叫 SaveSetupFile，ack.saved=false）。見 OVERRIDES 的 DoFormToData／btnApplyClick 兩段。

# ======================================================================
# 手寫的 blocks／overrides（每條上面一行寫原因）
# ======================================================================
# golden FormCreate :340-495 動態產生的 widget 指標陣列（uYieldMonitoring.h:568-579）→ 名字運算式（Name=sprintf("<前綴>%03d", i)）。
# autogen.py 用它把 ARR[i]->Prop 改寫成 J.SetProp(<名字>, …)；DoFormToData 的區域陣列（cbByBinFailCountCat_FT[16]={cbBybinLimitCat0_FT,…}）
# 由 autogen.py 自動換成名字陣列。
PTR_ARRAYS = {n: 'AnsiString().sprintf("%s%%03d", {0}).c_str()' % n for n in [
    'cbByBinSiteGapCat_FT', 'cbByBinSiteGapCat_RT', 'edByBinSiteGapCat_FT', 'edByBinSiteGapCat_RT',
    'cbByArmSiteGapCat_FT', 'cbByArmSiteGapCat_RT', 'edByArmSiteGapCat_FT', 'edByArmSiteGapCat_RT',
    'cbByBinFailureCat_FT', 'cbByBinFailureCat_RT', 'edByBinFailureCat_FT', 'edByBinFailureCat_RT']}

BLOCKS = [
    # golden FormShow :2611-2616 ASE_KaohSiung：MyYieldPanel[0]（TMyYieldPanel，建構子 :334 動態產生的 ART 面板）的表格寬度／欄數。
    # 移植樹沒有 TMyYieldPanel（uYieldMonitoring.cpp GATE G-YM-Panel）→ 照實回報；同一段的 tsReTest.TabVisible 照做。
    ('FormShow', 2611, 2616, 'if(CUSTOMER_CODE==CC_ASE_KaohSiung)',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150704 ART  bin 設定\n'
     '    {\n'
     '        J.Todo("golden FormShow :2613-2614 MyYieldPanel[0] mtBinSelectYield Width/XItem (ASE ART panel) -- TMyYieldPanel not ported (G-YM-Panel)");\n'
     '        J.SetTabVisible("tsReTest", false);                                             //kevin 20150717\n'
     '    }'),
    # golden DoIniDataToForm :735-740 ASE_KaohSiung：MyYieldPanel[0] 的 ReTestLimit、InitDataToEdit(0)（整支都在畫 ART 面板表格）、
    # InitmtBinSelectData() —— 同上，TMyYieldPanel 沒有 port。
    ('DoIniDataToForm', 735, 740, 'if(CUSTOMER_CODE==CC_ASE_KaohSiung)',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                         //kevin 20150529 AutoRetest\n'
     '    {\n'
     '        J.Todo("golden DoIniDataToForm :737-739 MyYieldPanel[0] ReTestLimit / InitDataToEdit(0) / InitmtBinSelectData (ASE ART panel) -- TMyYieldPanel not ported (G-YM-Panel)");\n'
     '    }'),
    # golden DoFormToData :2025-2028 是本方法最後一段；後面接 bridge 的「暫存複本 → 真結構」提交（見 OVERRIDES DoFormToData 第一條）。
    ('DoFormToData', 2025, 2028, 'if(CosFunction.bCreateManualEOCAP)',
     'if(CosFunction.bCreateManualEOCAP)                                          //jou 20221104 : VTest CreateManualEOCAP function;\n'
     '    {\n'
     '        TestIF_File.bCreateManualEOCAP=J.GetChecked("chkCreateManualEOCAP");\n'
     '    }\n'
     '    // ---- bridge：頁面沒送齊（golden 讀了、J 沒有）就不提交，真的 TestIF_File 一個欄位都不動；btnApplyClick 看到缺值會中止、不寫檔\n'
     '    if(J.MissingReads().empty())\n'
     '        ::TestIF_File=TestIF_File;   // 暫存複本 → 機台正在用的 TestIF_File\n'
     '    else\n'
     '        J.Todo("bridge: DoFormToData read widgets the page did not send -- TestIF_File NOT updated, nothing saved");'),
    # golden SaveSetupFile :2259-2286 ASE_KaohSiung [AutoRetest]：值來自 MyYieldPanel[0]（ART 面板成員）。沒有 port → 不寫，照實回報。
    ('SaveSetupFile', 2259, 2286, 'if(CUSTOMER_CODE==CC_ASE_KaohSiung)',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung)\n'
     '    {\n'
     '        J.Todo("golden SaveSetupFile :2259-2286 ASE_KaohSiung [AutoRetest] keys (values from MyYieldPanel[0]) NOT written -- TMyYieldPanel not ported (G-YM-Panel)");\n'
     '    }'),
    # golden btnApplyClick :3125-3128 Piggyback by Handler：SaveSetupFileToConfig(AuthPath, "config.ini") 寫 config.ini [Alarm]。
    # config.ini 由 FileRW/IniConfig 擁有（C 路 editlist.save）→ 這裡不寫，照實回報。
    ('btnApplyClick', 3125, 3128, 'if(CosFunction.bPiggybackFunctionByHandler==true)',
     'if(CosFunction.bPiggybackFunctionByHandler==true)                           //Isaac 20170712 :Piggyback function By Handlder(save file to config.ini)\n'
     '    {\n'
     '        J.Todo("golden btnApplyClick :3127 SaveSetupFileToConfig(AuthPath, config.ini) -- config.ini is owned by FileRW/IniConfig (C route editlist.save); [Alarm] Count Action / Continuous * NOT written here");\n'
     '    }'),
    # golden btnApplyClick :3130-3149 Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／FTestIF／自己的 SearchRecipeParameter）
    # —— 移植樹沒有這幾個 SearchRecipeParameter（與 TFTestIF.py 的 block 同一個理由）。
    ('btnApplyClick', 3130, 3149, 'if(CosFunction.bRecipeParameterDefault)',
     'if(CosFunction.bRecipeParameterDefault)                                     //Sam 20201209 : Default Recipe ChangeLog\n'
     '    {\n'
     '        J.Todo("golden btnApplyClick :3130-3149 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");\n'
     '    }'),
]

OVERRIDES = [
    # ---- golden ReadFile 的最後一行是 DoIniDataToForm()（:1703）；移植樹 ReadFile 那一行是 GATE(G-YM-Tail)（移植樹沒有 widget）。
    #      所以接到移植樹讀檔器之後，要在 bridge 這邊補跑 golden 的 DoIniDataToForm。
    ('FormShow', 'ReadFile();', 'fYieldMonitoring->ReadFile(); B_DoIniDataToForm(J);   // golden ReadFile() —— 移植樹讀檔器（uYieldMonitoring.cpp:2676）＋ golden ReadFile 尾 :1703 的 DoIniDataToForm()'),
    ('btnApplyClick', 'ReadFile();', 'fYieldMonitoring->ReadFile(); B_DoIniDataToForm(J);   // golden ReadFile() —— 移植樹讀檔器＋ golden ReadFile 尾 :1703 的 DoIniDataToForm()'),
    # ---- pgcMode.ActivePage=<TTabSheet>：FormState 只有 ActivePageIndex → 用 golden uYieldMonitoring.dfm 的頁序
    #      （pgcMode：0 tsNormal、1 tsReTest、2 tsAutoRetest、3 tsAutoRetest1、4 tsAlarm4、5 tsYield、6 tsAutoSiteOff）
    ('FormShow', 'pgcMode->ActivePage=tsNormal;', 'J.SetActivePageIndex("pgcMode", 0);   // golden: ActivePage=tsNormal（dfm 第 0 頁）'),
    ('FormShow', 'pgcMode->ActivePage=tsReTest;', 'J.SetActivePageIndex("pgcMode", 1);   // golden: ActivePage=tsReTest（dfm 第 1 頁）'),
    ('FormShow', 'pgcMode->ActivePage=tsAutoRetest;', 'J.SetActivePageIndex("pgcMode", 2);   // golden: ActivePage=tsAutoRetest（dfm 第 2 頁）'),
    ('FormShow', 'pgcMode->ActivePage=tsAutoRetest1;', 'J.SetActivePageIndex("pgcMode", 3);   // golden: ActivePage=tsAutoRetest1（dfm 第 3 頁）'),
    # ---- DoFormToData：golden 把 widget 直接寫進機台正在用的 TestIF_File。bridge 改成寫進區域複本（同名參考遮蔽全域），
    #      方法最後沒有缺值才整份覆蓋（見 BLOCKS DoFormToData）。iMinYield..iMaxCount 是 golden 建構子 :296-301 設的常數成員
    #      （全檔沒有別處寫），放成區域常數。
    ('DoFormToData', 'TestIF_File.bFailAlarmLowYield                  =cbLowYield_FT->Checked;',
     'SYSTEM_TEST_IF TestIF_File_bridge=::TestIF_File;   // bridge：暫存複本\n'
     'SYSTEM_TEST_IF& TestIF_File=TestIF_File_bridge;     // bridge：本方法內的 TestIF_File 都寫到複本\n'
     'const int iMinYield=0, iMaxYield=100, iMinCount=1, iMaxCount=100000;   // golden ctor :296-301\n'
     'const double dMinYield=0.0, dMaxYield=100.0;                          // golden ctor :298-299\n'
     'TestIF_File.bFailAlarmLowYield                  =J.GetChecked("cbLowYield_FT");'),
    # ---- btnApplyClick：DoFormToData 有缺值就中止（不跑 SaveSetupFile，ack.saved=false）。golden 是單行 if，換成複合敘述。
    ('btnApplyClick', 'DoFormToData();',
     '{ B_DoFormToData(J); if(!J.MissingReads().empty()) { J.Message("Some values were not sent by the page -- nothing saved", "頁面缺少部分欄位，未存檔"); return; } }'),
    # ---- ChangeData(fYieldMonitoring)：golden 只是把所有 CheckBox／Edit／Combo 的 OnClick/OnChange 接到 edContactCountFTChange（亮 Apply 鈕）
    ('FormShow', 'ChangeData(fYieldMonitoring);', '/* golden: ChangeData(fYieldMonitoring) —— 只接 OnClick/OnChange 事件（亮 Apply 鈕），頁面自己做 */'),
    # ---- golden 912 的欄位，移植樹 SYSTEM_TEST_IF（cprod.h）沒有（AI(rf360-yield-count) 20260814）
    ('DoIniDataToForm', 'edtYieldAlarmCheckIntervalByCount->Text =TestIF_File.iYieldAlarmCheckIntervalByCount;',
     'J.Todo("golden DoIniDataToForm: TestIF_File.iYieldAlarmCheckIntervalByCount (912, rf360) is missing from the port SYSTEM_TEST_IF -- edtYieldAlarmCheckIntervalByCount not shown");'),
    ('DoFormToData', 'TestIF_File.iYieldAlarmCheckIntervalByCount     =CheckRange(atoi(edtYieldAlarmCheckIntervalByCount->Text.c_str()), iMinCount, iMaxCount);',
     'J.Todo("golden DoFormToData: TestIF_File.iYieldAlarmCheckIntervalByCount (912) missing from the port SYSTEM_TEST_IF -- page value dropped");'),
    ('SaveSetupFile', 'WriteIniData(szDir, "Site Yield Alarm","Check Interval By Count",       int(TestIF_File.iYieldAlarmCheckIntervalByCount));',
     'J.Todo("golden SaveSetupFile: [Site Yield Alarm] Check Interval By Count NOT written -- TestIF_File.iYieldAlarmCheckIntervalByCount (912) missing from the port SYSTEM_TEST_IF");'),
    # ---- golden SaveSetupFile :2378 在「寫檔函式」裡改了執行期全域 bStandardYield（Auto Clean low yield 的基準重置）。
    #      form.save 會先空跑 SaveSetupFile（JsonBridge/FormJson.cpp:260，S="dry"）——空跑之後還可能拒寫，全域不可以先被改掉。
    #      golden 單行 if 的本體，換成複合敘述。
    ('SaveSetupFile', 'bStandardYield=false;',
     '{ if(!(S=="dry")) bStandardYield=false; }   // golden: bStandardYield=false; —— form.save 空跑（S=="dry"）時不動全域'),
    # ---- ASE_KaohSiung：iAutoRetestLimit 來自 MyYieldPanel[0]->ReTestLimit（ART 面板），沒有 port
    ('DoFormToData', 'iAutoRetestLimit= MyYieldPanel[0]->ReTestLimit->ItemIndex+1;',
     'J.Todo("golden DoFormToData :1944 iAutoRetestLimit from MyYieldPanel[0] ReTestLimit (ASE ART panel) -- TMyYieldPanel not ported, global unchanged");'),
]

# ---- AUTO(ws-arrow/multiline) BEGIN
# 由 autogen.py（Steven 團隊 scratchpad 工具）產生，不要手改：每條是 golden 那一行（或一段）照產生器同樣的規則改寫，
# 只多做：去掉 -> 前的對齊空白、把跨行右式收成一行、widget 指標陣列 ARR[i]-> 換成名字（PTR_ARRAYS）、
# 區域 widget 指標陣列宣告換成名字陣列。
# 原因：產生器以行為單位 —— golden 右式跨行、區域陣列初始化跨行，逐行改寫會編不過
AUTO_BLOCKS = [
    ('FormShow', 2701, 2702, 'rgAutoSiteOn        ->Visible=', 'J.SetVisible("rgAutoSiteOn", (CosFunction.bLowYieldAutoSiteOff || CosFunction.bAutoCloseSiteWhenRT));   //Steven 20230814 : Initial Start的時候要全開Site   /* golden 2701-2702 右式跨行，整條收成一行 */'),
    ('DoIniDataToForm', 809, 812, 'TCheckBox *cbByBinFailCountCat', 'const char* cbByBinFailCountCat_FT[16]={"cbBybinLimitCat0_FT", "cbBybinLimitCat1_FT", "cbBybinLimitCat2_FT", "cbBybinLimitCat3_FT", "cbBybinLimitCat4_FT", "cbBybinLimitCat5_FT", "cbBybinLimitCat6_FT", "cbBybinLimitCat7_FT", "cbBybinLimitCat8_FT", "cbBybinLimitCat9_FT", "cbBybinLimitCat10_FT", "cbBybinLimitCat11_FT", "cbBybinLimitCat12_FT", "cbBybinLimitCat13_FT", "cbBybinLimitCat14_FT", "cbBybinLimitCat15_FT"};   /* golden 809-812 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoIniDataToForm', 814, 817, 'TCheckBox *cbByBinFailCountCat', 'const char* cbByBinFailCountCat_RT[16]={"cbBybinLimitCat0_RT", "cbBybinLimitCat1_RT", "cbBybinLimitCat2_RT", "cbBybinLimitCat3_RT", "cbBybinLimitCat4_RT", "cbBybinLimitCat5_RT", "cbBybinLimitCat6_RT", "cbBybinLimitCat7_RT", "cbBybinLimitCat8_RT", "cbBybinLimitCat9_RT", "cbBybinLimitCat10_RT", "cbBybinLimitCat11_RT", "cbBybinLimitCat12_RT", "cbBybinLimitCat13_RT", "cbBybinLimitCat14_RT", "cbBybinLimitCat15_RT"};   /* golden 814-817 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoIniDataToForm', 819, 822, 'TEdit *edByBinFailCountCat_FT[', 'const char* edByBinFailCountCat_FT[16]={"edByBinLimitCountCat0_FT", "edByBinLimitCountCat1_FT", "edByBinLimitCountCat2_FT", "edByBinLimitCountCat3_FT", "edByBinLimitCountCat4_FT", "edByBinLimitCountCat5_FT", "edByBinLimitCountCat6_FT", "edByBinLimitCountCat7_FT", "edByBinLimitCountCat8_FT", "edByBinLimitCountCat9_FT", "edByBinLimitCountCat10_FT", "edByBinLimitCountCat11_FT", "edByBinLimitCountCat12_FT", "edByBinLimitCountCat13_FT", "edByBinLimitCountCat14_FT", "edByBinLimitCountCat15_FT"};   /* golden 819-822 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoIniDataToForm', 824, 827, 'TEdit *edByBinFailCountCat_RT[', 'const char* edByBinFailCountCat_RT[16]={"edByBinLimitCountCat0_RT", "edByBinLimitCountCat1_RT", "edByBinLimitCountCat2_RT", "edByBinLimitCountCat3_RT", "edByBinLimitCountCat4_RT", "edByBinLimitCountCat5_RT", "edByBinLimitCountCat6_RT", "edByBinLimitCountCat7_RT", "edByBinLimitCountCat8_RT", "edByBinLimitCountCat9_RT", "edByBinLimitCountCat10_RT", "edByBinLimitCountCat11_RT", "edByBinLimitCountCat12_RT", "edByBinLimitCountCat13_RT", "edByBinLimitCountCat14_RT", "edByBinLimitCountCat15_RT"};   /* golden 824-827 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoFormToData', 1961, 1964, 'TCheckBox *cbByBinFailCountCat', 'const char* cbByBinFailCountCat_FT[16]={"cbBybinLimitCat0_FT", "cbBybinLimitCat1_FT", "cbBybinLimitCat2_FT", "cbBybinLimitCat3_FT", "cbBybinLimitCat4_FT", "cbBybinLimitCat5_FT", "cbBybinLimitCat6_FT", "cbBybinLimitCat7_FT", "cbBybinLimitCat8_FT", "cbBybinLimitCat9_FT", "cbBybinLimitCat10_FT", "cbBybinLimitCat11_FT", "cbBybinLimitCat12_FT", "cbBybinLimitCat13_FT", "cbBybinLimitCat14_FT", "cbBybinLimitCat15_FT"};   /* golden 1961-1964 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoFormToData', 1966, 1969, 'TCheckBox *cbByBinFailCountCat', 'const char* cbByBinFailCountCat_RT[16]={"cbBybinLimitCat0_RT", "cbBybinLimitCat1_RT", "cbBybinLimitCat2_RT", "cbBybinLimitCat3_RT", "cbBybinLimitCat4_RT", "cbBybinLimitCat5_RT", "cbBybinLimitCat6_RT", "cbBybinLimitCat7_RT", "cbBybinLimitCat8_RT", "cbBybinLimitCat9_RT", "cbBybinLimitCat10_RT", "cbBybinLimitCat11_RT", "cbBybinLimitCat12_RT", "cbBybinLimitCat13_RT", "cbBybinLimitCat14_RT", "cbBybinLimitCat15_RT"};   /* golden 1966-1969 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoFormToData', 1971, 1974, 'TEdit *edByBinFailCountCat_FT[', 'const char* edByBinFailCountCat_FT[16]={"edByBinLimitCountCat0_FT", "edByBinLimitCountCat1_FT", "edByBinLimitCountCat2_FT", "edByBinLimitCountCat3_FT", "edByBinLimitCountCat4_FT", "edByBinLimitCountCat5_FT", "edByBinLimitCountCat6_FT", "edByBinLimitCountCat7_FT", "edByBinLimitCountCat8_FT", "edByBinLimitCountCat9_FT", "edByBinLimitCountCat10_FT", "edByBinLimitCountCat11_FT", "edByBinLimitCountCat12_FT", "edByBinLimitCountCat13_FT", "edByBinLimitCountCat14_FT", "edByBinLimitCountCat15_FT"};   /* golden 1971-1974 區域 widget 指標陣列 → 名字陣列 */'),
    ('DoFormToData', 1976, 1979, 'TEdit *edByBinFailCountCat_RT[', 'const char* edByBinFailCountCat_RT[16]={"edByBinLimitCountCat0_RT", "edByBinLimitCountCat1_RT", "edByBinLimitCountCat2_RT", "edByBinLimitCountCat3_RT", "edByBinLimitCountCat4_RT", "edByBinLimitCountCat5_RT", "edByBinLimitCountCat6_RT", "edByBinLimitCountCat7_RT", "edByBinLimitCountCat8_RT", "edByBinLimitCountCat9_RT", "edByBinLimitCountCat10_RT", "edByBinLimitCountCat11_RT", "edByBinLimitCountCat12_RT", "edByBinLimitCountCat13_RT", "edByBinLimitCountCat14_RT", "edByBinLimitCountCat15_RT"};   /* golden 1976-1979 區域 widget 指標陣列 → 名字陣列 */'),
]
# 原因：產生器的 regex 是 \bW->，golden 的 `W   ->Prop` 對齊寫法與 `ARR[i]->Prop` 不會被改寫，也不會被當成殘留抓出來
AUTO_OVERRIDES = [
    ('FormShow', 'gbYieldAlarm    ->Enabled=true;', 'J.SetEnabled("gbYieldAlarm", true);'),
    ('FormShow', 'gbPiggyBack     ->Enabled=true;', 'J.SetEnabled("gbPiggyBack", true);'),
    ('FormShow', 'gbAlarm         ->Enabled=true;', 'J.SetEnabled("gbAlarm", true);'),
    ('FormShow', 'gbYieldAlarmRT  ->Enabled=true;', 'J.SetEnabled("gbYieldAlarmRT", true);'),
    ('FormShow', 'gbPiggyBackRT   ->Enabled=true;', 'J.SetEnabled("gbPiggyBackRT", true);'),
    ('FormShow', 'gbAlarmRT       ->Enabled=true;', 'J.SetEnabled("gbAlarmRT", true);'),
    ('FormShow', 'cbByBinSiteGapCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByBinSiteGapCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'cbByBinSiteGapCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByBinSiteGapCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByBinSiteGapCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByBinSiteGapCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByBinSiteGapCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByBinSiteGapCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'cbByArmSiteGapCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByArmSiteGapCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'cbByArmSiteGapCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByArmSiteGapCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByArmSiteGapCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByArmSiteGapCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByArmSiteGapCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByArmSiteGapCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'cbByBinFailureCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByBinFailureCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'cbByBinFailureCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("cbByBinFailureCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByBinFailureCat_FT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByBinFailureCat_FT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'edByBinFailureCat_RT[i]->Visible=(i<iTestBinCount);', 'J.SetVisible(AnsiString().sprintf("edByBinFailureCat_RT%03d", i).c_str(), (i<iTestBinCount));'),
    ('FormShow', 'palContinuousPass_FT  ->Visible=false;', 'J.SetVisible("palContinuousPass_FT", false);'),
    ('FormShow', 'palContinuousPass_RT  ->Visible=false;', 'J.SetVisible("palContinuousPass_RT", false);'),
    ('FormShow', 'grpSiteYieldCmp_FT  ->Visible=CosFunction.bSiteCmpYield;', 'J.SetVisible("grpSiteYieldCmp_FT", CosFunction.bSiteCmpYield);'),
    ('FormShow', 'grpSiteYieldCmp_RT  ->Visible=CosFunction.bSiteCmpYield;', 'J.SetVisible("grpSiteYieldCmp_RT", CosFunction.bSiteCmpYield);'),
    ('FormShow', 'cbYieldAlarmByBin   ->Visible=CosFunction.bUseLowYieldAlarmByBin;', 'J.SetVisible("cbYieldAlarmByBin", CosFunction.bUseLowYieldAlarmByBin);'),
    ('FormShow', 'chkAutoSiteOff              ->Visible=(CosFunction.bLowYieldAutoSiteOff);', 'J.SetVisible("chkAutoSiteOff", (CosFunction.bLowYieldAutoSiteOff));'),
    ('FormShow', 'chkAutoSiteOffByArmBySite   ->Visible=(CosFunction.bLowYieldAutoSiteOff);', 'J.SetVisible("chkAutoSiteOffByArmBySite", (CosFunction.bLowYieldAutoSiteOff));'),
    ('FormShow', 'chkAutoSiteOffByContiFail   ->Visible=(CosFunction.bLowYieldAutoSiteOff);', 'J.SetVisible("chkAutoSiteOffByContiFail", (CosFunction.bLowYieldAutoSiteOff));'),
    ('FormShow', 'chkchkAutoSiteOffByPicker   ->Visible=(CosFunction.bLowYieldAutoSiteOff);', 'J.SetVisible("chkchkAutoSiteOffByPicker", (CosFunction.bLowYieldAutoSiteOff));'),
    ('FormShow', 'rgPiggyBack_FT     ->Enabled=false;', 'J.SetEnabled("rgPiggyBack_FT", false);'),
    ('FormShow', 'cbContactCountFT   ->Enabled=false;', 'J.SetEnabled("cbContactCountFT", false);'),
    ('FormShow', 'edContactCountFT   ->Enabled=false;', 'J.SetEnabled("edContactCountFT", false);'),
    ('FormShow', 'rgPiggyBack_FT      ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("rgPiggyBack_FT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'edContinuPassSkt_FT ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("edContinuPassSkt_FT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'edContactCountFT    ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("edContactCountFT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'gbYieldAlarm    ->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;', 'J.SetEnabled("gbYieldAlarm", (AccessLevel<LevelSet.AccessLevel[81])?false:J.GetEnabled("gbYieldAlarm"));'),
    ('FormShow', 'gbPiggyBack     ->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:gbPiggyBack   ->Enabled;', 'J.SetEnabled("gbPiggyBack", (AccessLevel<LevelSet.AccessLevel[82])?false:J.GetEnabled("gbPiggyBack"));'),
    ('FormShow', 'gbAlarm         ->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:gbAlarm       ->Enabled;', 'J.SetEnabled("gbAlarm", (AccessLevel<LevelSet.AccessLevel[83])?false:J.GetEnabled("gbAlarm"));'),
    ('FormShow', 'gbYieldAlarmRT  ->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarmRT->Enabled;', 'J.SetEnabled("gbYieldAlarmRT", (AccessLevel<LevelSet.AccessLevel[81])?false:J.GetEnabled("gbYieldAlarmRT"));'),
    ('FormShow', 'gbPiggyBackRT   ->Enabled=(AccessLevel<LevelSet.AccessLevel[82])?false:gbPiggyBackRT ->Enabled;', 'J.SetEnabled("gbPiggyBackRT", (AccessLevel<LevelSet.AccessLevel[82])?false:J.GetEnabled("gbPiggyBackRT"));'),
    ('FormShow', 'gbAlarmRT       ->Enabled=(AccessLevel<LevelSet.AccessLevel[83])?false:gbAlarmRT     ->Enabled;', 'J.SetEnabled("gbAlarmRT", (AccessLevel<LevelSet.AccessLevel[83])?false:J.GetEnabled("gbAlarmRT"));'),
    ('FormShow', 'pgcBySiteByBinPercentCompare_FT->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;', 'J.SetEnabled("pgcBySiteByBinPercentCompare_FT", (AccessLevel<LevelSet.AccessLevel[81])?false:J.GetEnabled("gbYieldAlarm"));'),
    ('FormShow', 'pgcBySiteByBinPercentCompare_RT->Enabled=(AccessLevel<LevelSet.AccessLevel[81])?false:gbYieldAlarm  ->Enabled;', 'J.SetEnabled("pgcBySiteByBinPercentCompare_RT", (AccessLevel<LevelSet.AccessLevel[81])?false:J.GetEnabled("gbYieldAlarm"));'),
    ('FormShow', 'if(gbPiggyBackRT     ->Enabled==false)', 'if(J.GetEnabled("gbPiggyBackRT")==false)'),
    ('FormShow', 'rgPiggyBack_RT     ->Enabled=false;', 'J.SetEnabled("rgPiggyBack_RT", false);'),
    ('FormShow', 'cbContactCountRT   ->Enabled=false;', 'J.SetEnabled("cbContactCountRT", false);'),
    ('FormShow', 'edContactCountRT   ->Enabled=false;', 'J.SetEnabled("edContactCountRT", false);'),
    ('FormShow', 'rgPiggyBack_RT      ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("rgPiggyBack_RT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'edContinuPassSkt_RT ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("edContinuPassSkt_RT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'edContactCountRT    ->Enabled=fSecurity->Insufficient(39, false);', 'J.SetEnabled("edContactCountRT", fSecurity->Insufficient(39, false));'),
    ('FormShow', 'cbLowYield_FT                       ->Enabled=fSecurity->Insufficient(118, false);', 'J.SetEnabled("cbLowYield_FT", fSecurity->Insufficient(118, false));'),
    ('FormShow', 'cbSiteYieldDifferent_FT             ->Enabled=fSecurity->Insufficient(118, false);', 'J.SetEnabled("cbSiteYieldDifferent_FT", fSecurity->Insufficient(118, false));'),
    ('FormShow', 'cbLowYield_RT                       ->Enabled=fSecurity->Insufficient(118, false);', 'J.SetEnabled("cbLowYield_RT", fSecurity->Insufficient(118, false));'),
    ('FormShow', 'cbSiteYieldDifferent_RT             ->Enabled=fSecurity->Insufficient(118, false);', 'J.SetEnabled("cbSiteYieldDifferent_RT", fSecurity->Insufficient(118, false));'),
    ('FormShow', 'edLowYield_FT                       ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edLowYield_FT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edSiteYieldDifferent_FT             ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edSiteYieldDifferent_FT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edLowYieldIg_FT                     ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edLowYieldIg_FT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edSiteYieldDifferentIg_FT           ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edSiteYieldDifferentIg_FT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edSiteYieldDifferent_RT             ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edSiteYieldDifferent_RT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edLowYield_RT                       ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edLowYield_RT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edLowYieldIg_RT                     ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edLowYieldIg_RT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'edSiteYieldDifferentIg_RT           ->Enabled=fSecurity->Insufficient(81, false);', 'J.SetEnabled("edSiteYieldDifferentIg_RT", fSecurity->Insufficient(81, false));'),
    ('FormShow', 'cbContinuousPass_FT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuousPass_FT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContactCountFT                    ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContactCountFT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContinuousLoad_FT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuousLoad_FT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContinuPassSkt_FT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuPassSkt_FT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContinuPassSkt_RT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuPassSkt_RT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContactCountRT                    ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContactCountRT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContinuousPass_RT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuousPass_RT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'cbContinuousLoad_RT                 ->Enabled=fSecurity->Insufficient(119, false);', 'J.SetEnabled("cbContinuousLoad_RT", fSecurity->Insufficient(119, false));'),
    ('FormShow', 'edContinuousPass_FT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuousPass_FT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContinuPassSkt_FT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuPassSkt_FT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContinuousLoad_FT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuousLoad_FT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContactCountFT                    ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContactCountFT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'cobContinuousPass_FT                ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("cobContinuousPass_FT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContinuPassSkt_RT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuPassSkt_RT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContinuousPass_RT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuousPass_RT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContinuousLoad_RT                 ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContinuousLoad_RT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'edContactCountRT                    ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("edContactCountRT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'cobContinuousPass_RT                ->Enabled=fSecurity->Insufficient(82, false);', 'J.SetEnabled("cobContinuousPass_RT", fSecurity->Insufficient(82, false));'),
    ('FormShow', 'rbContsFailBySocket_FTOn            ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailBySocket_FTOn", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailBySocket_FTOff           ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailBySocket_FTOff", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailByHead_FTOn              ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailByHead_FTOn", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailByHead_FTOff             ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailByHead_FTOff", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailBySocket_RTOn            ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailBySocket_RTOn", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailBySocket_RTOff           ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailBySocket_RTOff", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailByHead_RTOn              ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailByHead_RTOn", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'rbContsFailByHead_RTOff             ->Enabled=fSecurity->Insufficient(120, false);', 'J.SetEnabled("rbContsFailByHead_RTOff", fSecurity->Insufficient(120, false));'),
    ('FormShow', 'edContsFailSocketAlarmCT_FT         ->Enabled=fSecurity->Insufficient(83, false);', 'J.SetEnabled("edContsFailSocketAlarmCT_FT", fSecurity->Insufficient(83, false));'),
    ('FormShow', 'edContsFailHeadAlarmCT_FT           ->Enabled=fSecurity->Insufficient(83, false);', 'J.SetEnabled("edContsFailHeadAlarmCT_FT", fSecurity->Insufficient(83, false));'),
    ('FormShow', 'edContsFailSocketAlarmCT_RT         ->Enabled=fSecurity->Insufficient(83, false);', 'J.SetEnabled("edContsFailSocketAlarmCT_RT", fSecurity->Insufficient(83, false));'),
    ('FormShow', 'edContsFailHeadAlarmCT_RT           ->Enabled=fSecurity->Insufficient(83, false);', 'J.SetEnabled("edContsFailHeadAlarmCT_RT", fSecurity->Insufficient(83, false));'),
    ('FormShow', 'cbAlarm4ContinueType                ->Enabled=fSecurity->Insufficient(121, false);', 'J.SetEnabled("cbAlarm4ContinueType", fSecurity->Insufficient(121, false));'),
    ('FormShow', 'cb_Alarm4EnableIntervalYield        ->Enabled=fSecurity->Insufficient(121, false);', 'J.SetEnabled("cb_Alarm4EnableIntervalYield", fSecurity->Insufficient(121, false));'),
    ('FormShow', 'cb_SiteToSiteYieldEnable            ->Enabled=fSecurity->Insufficient(121, false);', 'J.SetEnabled("cb_SiteToSiteYieldEnable", fSecurity->Insufficient(121, false));'),
    ('FormShow', 'cb_HeadToHeadYieldEnable            ->Enabled=fSecurity->Insufficient(121, false);', 'J.SetEnabled("cb_HeadToHeadYieldEnable", fSecurity->Insufficient(121, false));'),
    ('FormShow', 'cb_SiteYieldOverAlert               ->Enabled=fSecurity->Insufficient(121, false);', 'J.SetEnabled("cb_SiteYieldOverAlert", fSecurity->Insufficient(121, false));'),
    ('FormShow', 'edAlarm4IntervalCount               ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("edAlarm4IntervalCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'edAlarm4ContinueCount               ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("edAlarm4ContinueCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_Alarm4IntervalYieldIntervalCount ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_Alarm4IntervalYieldIntervalCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_Alarm4IntervalYieldContinueCount ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_Alarm4IntervalYieldContinueCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_Alarm4IntervalYieldYield         ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_Alarm4IntervalYieldYield", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_SiteToSiteYield                  ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_SiteToSiteYield", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_HeadToHeadYield                  ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_HeadToHeadYield", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_SiteYieldOverAlert               ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_SiteYieldOverAlert", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_SiteToSiteYieldCount             ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_SiteToSiteYieldCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_HeadToHeadYieldCount             ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_HeadToHeadYieldCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'ed_SiteYieldOverAlertCount          ->Enabled=fSecurity->Insufficient(117, false);', 'J.SetEnabled("ed_SiteYieldOverAlertCount", fSecurity->Insufficient(117, false));'),
    ('FormShow', 'cbAlarm5_BySiteLowYieldEnable       ->Enabled=fSecurity->Insufficient(153, false);', 'J.SetEnabled("cbAlarm5_BySiteLowYieldEnable", fSecurity->Insufficient(153, false));'),
    ('FormShow', 'cbAlarm5_BySiteCmpYieldEnable       ->Enabled=fSecurity->Insufficient(153, false);', 'J.SetEnabled("cbAlarm5_BySiteCmpYieldEnable", fSecurity->Insufficient(153, false));'),
    ('FormShow', 'cbAlarm5_BySiteAlarmYieldEnable     ->Enabled=fSecurity->Insufficient(153, false);', 'J.SetEnabled("cbAlarm5_BySiteAlarmYieldEnable", fSecurity->Insufficient(153, false));'),
    ('FormShow', 'cbAlarm5_BySitePreCmpYieldEnable    ->Enabled=fSecurity->Insufficient(153, false);', 'J.SetEnabled("cbAlarm5_BySitePreCmpYieldEnable", fSecurity->Insufficient(153, false));'),
    ('FormShow', 'edAlarm5_OSBin                      ->Enabled=false;', 'J.SetEnabled("edAlarm5_OSBin", false);'),
    ('FormShow', 'edAlarm5_BySiteLowYield             ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteLowYield", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteLowYieldRej          ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteLowYieldRej", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteAlarmYield           ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteAlarmYield", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteAlarmYieldRej        ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteAlarmYieldRej", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteIntervalContactCnt   ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteIntervalContactCnt", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteCmpYield             ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteCmpYield", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySiteCmpYieldRej          ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySiteCmpYieldRej", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySitePreCmpYield          ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySitePreCmpYield", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'edAlarm5_BySitePreCmpYieldRej       ->Enabled=fSecurity->Insufficient(154, false);', 'J.SetEnabled("edAlarm5_BySitePreCmpYieldRej", fSecurity->Insufficient(154, false));'),
    ('FormShow', 'rbContsFailBySocket_FTOn    ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailBySocket_FTOn", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailBySocket_FTOff   ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailBySocket_FTOff", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailByHead_FTOn      ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailByHead_FTOn", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailByHead_FTOff     ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailByHead_FTOff", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailBySocket_RTOn    ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailBySocket_RTOn", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailBySocket_RTOff   ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailBySocket_RTOff", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailByHead_RTOn      ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailByHead_RTOn", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'rbContsFailByHead_RTOff     ->Enabled=fSecurity->Insufficient(125, false);', 'J.SetEnabled("rbContsFailByHead_RTOff", fSecurity->Insufficient(125, false));'),
    ('FormShow', 'edContsFailSocketAlarmCT_FT ->Enabled=fSecurity->Insufficient(126, false);', 'J.SetEnabled("edContsFailSocketAlarmCT_FT", fSecurity->Insufficient(126, false));'),
    ('FormShow', 'edContsFailHeadAlarmCT_FT   ->Enabled=fSecurity->Insufficient(126, false);', 'J.SetEnabled("edContsFailHeadAlarmCT_FT", fSecurity->Insufficient(126, false));'),
    ('FormShow', 'edContsFailSocketAlarmCT_RT ->Enabled=fSecurity->Insufficient(126, false);', 'J.SetEnabled("edContsFailSocketAlarmCT_RT", fSecurity->Insufficient(126, false));'),
    ('FormShow', 'edContsFailHeadAlarmCT_RT   ->Enabled=fSecurity->Insufficient(126, false);', 'J.SetEnabled("edContsFailHeadAlarmCT_RT", fSecurity->Insufficient(126, false));'),
    ('FormShow', 'labLowYield1_FT             ->Caption="%  after contact count";', 'J.SetCaption("labLowYield1_FT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labSiteYieldDifferent1_FT   ->Caption="%  after contact count";', 'J.SetCaption("labSiteYieldDifferent1_FT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labSiteYieldCmp1_FT         ->Caption="%  after contact count";', 'J.SetCaption("labSiteYieldCmp1_FT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labLowYield1_RT             ->Caption="%  after contact count";', 'J.SetCaption("labLowYield1_RT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labSiteYieldDifferent1_RT   ->Caption="%  after contact count";', 'J.SetCaption("labSiteYieldDifferent1_RT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labSiteYieldCmp1_RT         ->Caption="%  after contact count";', 'J.SetCaption("labSiteYieldCmp1_RT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labLowYieldByTotal1_FT      ->Caption="%  after contact count";', 'J.SetCaption("labLowYieldByTotal1_FT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labLowYieldByTotal1_RT      ->Caption="%  after contact count";', 'J.SetCaption("labLowYieldByTotal1_RT", AnsiString("%  after contact count"));'),
    ('FormShow', 'lblLowYieldByPicker_FT      ->Caption="%  after contact count";', 'J.SetCaption("lblLowYieldByPicker_FT", AnsiString("%  after contact count"));'),
    ('FormShow', 'lblLowYieldByPicker_RT      ->Caption="%  after contact count";', 'J.SetCaption("lblLowYieldByPicker_RT", AnsiString("%  after contact count"));'),
    ('FormShow', 'labLowYield1_FT             ->Caption="%  Ignore IC count";', 'J.SetCaption("labLowYield1_FT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labSiteYieldDifferent1_FT   ->Caption="%  Ignore IC count";', 'J.SetCaption("labSiteYieldDifferent1_FT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labSiteYieldCmp1_FT         ->Caption="%  Ignore IC count";', 'J.SetCaption("labSiteYieldCmp1_FT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labLowYield1_RT             ->Caption="%  Ignore IC count";', 'J.SetCaption("labLowYield1_RT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labSiteYieldDifferent1_RT   ->Caption="%  Ignore IC count";', 'J.SetCaption("labSiteYieldDifferent1_RT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labSiteYieldCmp1_RT         ->Caption="%  Ignore IC count";', 'J.SetCaption("labSiteYieldCmp1_RT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labLowYieldByTotal1_FT      ->Caption="%  Ignore IC count";', 'J.SetCaption("labLowYieldByTotal1_FT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labLowYieldByTotal1_RT      ->Caption="%  Ignore IC count";', 'J.SetCaption("labLowYieldByTotal1_RT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'lblLowYieldByPicker_FT      ->Caption="%  Ignore IC count";', 'J.SetCaption("lblLowYieldByPicker_FT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'lblLowYieldByPicker_RT      ->Caption="%  Ignore IC count";', 'J.SetCaption("lblLowYieldByPicker_RT", AnsiString("%  Ignore IC count"));'),
    ('FormShow', 'labSiteYieldDifferent1_FT   ->Caption="%  after per site count";', 'J.SetCaption("labSiteYieldDifferent1_FT", AnsiString("%  after per site count"));'),
    ('FormShow', 'labSiteYieldCmp1_FT         ->Caption="%  after per site count";', 'J.SetCaption("labSiteYieldCmp1_FT", AnsiString("%  after per site count"));'),
    ('FormShow', 'labSiteYieldDifferent1_RT   ->Caption="%  after per site count";', 'J.SetCaption("labSiteYieldDifferent1_RT", AnsiString("%  after per site count"));'),
    ('FormShow', 'labSiteYieldCmp1_RT         ->Caption="%  after per site count";', 'J.SetCaption("labSiteYieldCmp1_RT", AnsiString("%  after per site count"));'),
    ('FormShow', 'lblLowYieldByPicker_FT      ->Caption="%  after per site count";', 'J.SetCaption("lblLowYieldByPicker_FT", AnsiString("%  after per site count"));'),
    ('FormShow', 'lblLowYieldByPicker_RT      ->Caption="%  after per site count";', 'J.SetCaption("lblLowYieldByPicker_RT", AnsiString("%  after per site count"));'),
    ('FormShow', 'labLowYield1_FT             ->Caption="%  after test count";', 'J.SetCaption("labLowYield1_FT", AnsiString("%  after test count"));'),
    ('FormShow', 'labLowYield1_RT             ->Caption="%  after test count";', 'J.SetCaption("labLowYield1_RT", AnsiString("%  after test count"));'),
    ('FormShow', 'grpLowYield_FT              ->Caption="Low Yields%(By Site)";', 'J.SetCaption("grpLowYield_FT", AnsiString("Low Yields%(By Site)"));'),
    ('FormShow', 'grpSiteYieldDifferent_FT    ->Caption="By Arm Per Site Differ Yield%";', 'J.SetCaption("grpSiteYieldDifferent_FT", AnsiString("By Arm Per Site Differ Yield%"));'),
    ('FormShow', 'grpSiteYieldCmp_FT          ->Caption="By Site Compare Yield%";', 'J.SetCaption("grpSiteYieldCmp_FT", AnsiString("By Site Compare Yield%"));'),
    ('FormShow', 'grpLowYield_RT              ->Caption="Low Yields%(By Site)";', 'J.SetCaption("grpLowYield_RT", AnsiString("Low Yields%(By Site)"));'),
    ('FormShow', 'grpSiteYieldDifferent_RT    ->Caption="By Arm Per Site Differ Yield%";', 'J.SetCaption("grpSiteYieldDifferent_RT", AnsiString("By Arm Per Site Differ Yield%"));'),
    ('FormShow', 'grpSiteYieldCmp_RT          ->Caption="By Site Compare Yield%";', 'J.SetCaption("grpSiteYieldCmp_RT", AnsiString("By Site Compare Yield%"));'),
    ('FormShow', 'grpLowYield_FT              ->Caption="Low Yields% (1min)";', 'J.SetCaption("grpLowYield_FT", AnsiString("Low Yields% (1min)"));'),
    ('FormShow', 'grpSiteYieldDifferent_FT    ->Caption="By Arm Per Site Differ Yield% (1min)";', 'J.SetCaption("grpSiteYieldDifferent_FT", AnsiString("By Arm Per Site Differ Yield% (1min)"));'),
    ('FormShow', 'grpSiteYieldCmp_FT          ->Caption="By Site Compare Yield% (1min)";', 'J.SetCaption("grpSiteYieldCmp_FT", AnsiString("By Site Compare Yield% (1min)"));'),
    ('FormShow', 'grpLowYield_RT              ->Caption="Low Yields% (1min)";', 'J.SetCaption("grpLowYield_RT", AnsiString("Low Yields% (1min)"));'),
    ('FormShow', 'grpSiteYieldDifferent_RT    ->Caption="By Arm Per Site Differ Yield% (1min)";', 'J.SetCaption("grpSiteYieldDifferent_RT", AnsiString("By Arm Per Site Differ Yield% (1min)"));'),
    ('FormShow', 'grpSiteYieldCmp_RT          ->Caption="By Site Compare Yield% (1min)";', 'J.SetCaption("grpSiteYieldCmp_RT", AnsiString("By Site Compare Yield% (1min)"));'),
    ('FormShow', 'gbYieldAlarm  ->Enabled=false;', 'J.SetEnabled("gbYieldAlarm", false);'),
    ('FormShow', 'btnUnlockYieldAlarm  ->Visible=(bYieldAlarmUnlocked==false);', 'J.SetVisible("btnUnlockYieldAlarm", (J.M("bYieldAlarmUnlocked")==false));'),
    ('FormShow', 'palConsFailIgnore_FT ->Visible=(CUSTOMER_CODE==CC_KYEC_CHEN);', 'J.SetVisible("palConsFailIgnore_FT", (CUSTOMER_CODE==CC_KYEC_CHEN));'),
    ('FormShow', 'palConsFailIgnore_RT ->Visible=(CUSTOMER_CODE==CC_KYEC_CHEN);', 'J.SetVisible("palConsFailIgnore_RT", (CUSTOMER_CODE==CC_KYEC_CHEN));'),
    ('FormShow', 'cbAllSiteFail       ->Visible=false;', 'J.SetVisible("cbAllSiteFail", false);'),
    ('FormShow', 'edAllSiteFailCount  ->Visible=false;', 'J.SetVisible("edAllSiteFailCount", false);'),
    ('FormShow', 'cbAllSiteFail_RT    ->Visible=false;', 'J.SetVisible("cbAllSiteFail_RT", false);'),
    ('FormShow', 'gbPiggyBack     ->Visible=false;', 'J.SetVisible("gbPiggyBack", false);'),
    ('FormShow', 'gbPiggyBackRT   ->Visible=false;', 'J.SetVisible("gbPiggyBackRT", false);'),
    ('DoIniDataToForm', 'rbContsFailBySocket_FTOn ->Enabled=true;', 'J.SetEnabled("rbContsFailBySocket_FTOn", true);'),
    ('DoIniDataToForm', 'rbContsFailBySocket_FTOn ->Checked=true;', 'J.SetChecked("rbContsFailBySocket_FTOn", true);'),
    ('DoIniDataToForm', 'rbContsFailBySocket_FTOn ->Enabled=false;', 'J.SetEnabled("rbContsFailBySocket_FTOn", false);'),
    ('DoIniDataToForm', 'rbContsFailBySocket_FTOn ->Checked=TestIF_File.bContsFailBySocket;', 'J.SetChecked("rbContsFailBySocket_FTOn", TestIF_File.bContsFailBySocket);'),
    ('DoIniDataToForm', 'rbContsFailBySocket_RTOn ->Checked=TestIF_File.bContsFailBySocket_RT;', 'J.SetChecked("rbContsFailBySocket_RTOn", TestIF_File.bContsFailBySocket_RT);'),
    ('DoIniDataToForm', 'rbContsFailByHead_FTOn ->Enabled=true;', 'J.SetEnabled("rbContsFailByHead_FTOn", true);'),
    ('DoIniDataToForm', 'rbContsFailByHead_FTOn ->Checked=true;', 'J.SetChecked("rbContsFailByHead_FTOn", true);'),
    ('DoIniDataToForm', 'rbContsFailByHead_FTOn ->Enabled=false;', 'J.SetEnabled("rbContsFailByHead_FTOn", false);'),
    ('DoIniDataToForm', 'rbContsFailByHead_FTOn ->Checked=TestIF_File.bContsFailByHead;', 'J.SetChecked("rbContsFailByHead_FTOn", TestIF_File.bContsFailByHead);'),
    ('DoIniDataToForm', 'rbContsFailByHead_RTOn ->Checked    =TestIF_File.bContsFailByHead_RT;', 'J.SetChecked("rbContsFailByHead_RTOn", TestIF_File.bContsFailByHead_RT);'),
    ('DoIniDataToForm', 'cbByBinSiteGapCat_FT[i]->Checked=TestIF_File.bSpecBinBySiteCompareEnable[FT][i];', 'J.SetChecked(AnsiString().sprintf("cbByBinSiteGapCat_FT%03d", i).c_str(), TestIF_File.bSpecBinBySiteCompareEnable[FT][i]);'),
    ('DoIniDataToForm', 'cbByBinSiteGapCat_RT[i]->Checked=TestIF_File.bSpecBinBySiteCompareEnable[RT][i];', 'J.SetChecked(AnsiString().sprintf("cbByBinSiteGapCat_RT%03d", i).c_str(), TestIF_File.bSpecBinBySiteCompareEnable[RT][i]);'),
    ('DoIniDataToForm', 'edByBinSiteGapCat_FT[i]->Text   =TestIF_File.dSpecBinBySiteComparePercent[FT][i];', 'J.SetText(AnsiString().sprintf("edByBinSiteGapCat_FT%03d", i).c_str(), AnsiString(TestIF_File.dSpecBinBySiteComparePercent[FT][i]));'),
    ('DoIniDataToForm', 'edByBinSiteGapCat_RT[i]->Text   =TestIF_File.dSpecBinBySiteComparePercent[RT][i];', 'J.SetText(AnsiString().sprintf("edByBinSiteGapCat_RT%03d", i).c_str(), AnsiString(TestIF_File.dSpecBinBySiteComparePercent[RT][i]));'),
    ('DoIniDataToForm', 'cbByArmSiteGapCat_FT[i]->Checked=TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i];', 'J.SetChecked(AnsiString().sprintf("cbByArmSiteGapCat_FT%03d", i).c_str(), TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]);'),
    ('DoIniDataToForm', 'cbByArmSiteGapCat_RT[i]->Checked=TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i];', 'J.SetChecked(AnsiString().sprintf("cbByArmSiteGapCat_RT%03d", i).c_str(), TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]);'),
    ('DoIniDataToForm', 'edByArmSiteGapCat_FT[i]->Text   =TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i];', 'J.SetText(AnsiString().sprintf("edByArmSiteGapCat_FT%03d", i).c_str(), AnsiString(TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]));'),
    ('DoIniDataToForm', 'edByArmSiteGapCat_RT[i]->Text   =TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i];', 'J.SetText(AnsiString().sprintf("edByArmSiteGapCat_RT%03d", i).c_str(), AnsiString(TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]));'),
    ('DoIniDataToForm', 'cbByBinFailureCat_FT[i]->Checked=TestIF_File.bByBinFailureEnable[FT][i];', 'J.SetChecked(AnsiString().sprintf("cbByBinFailureCat_FT%03d", i).c_str(), TestIF_File.bByBinFailureEnable[FT][i]);'),
    ('DoIniDataToForm', 'cbByBinFailureCat_RT[i]->Checked=TestIF_File.bByBinFailureEnable[RT][i];', 'J.SetChecked(AnsiString().sprintf("cbByBinFailureCat_RT%03d", i).c_str(), TestIF_File.bByBinFailureEnable[RT][i]);'),
    ('DoIniDataToForm', 'edByBinFailureCat_FT[i]->Text   =TestIF_File.dByBinFailurePercent[FT][i];', 'J.SetText(AnsiString().sprintf("edByBinFailureCat_FT%03d", i).c_str(), AnsiString(TestIF_File.dByBinFailurePercent[FT][i]));'),
    ('DoIniDataToForm', 'edByBinFailureCat_RT[i]->Text   =TestIF_File.dByBinFailurePercent[RT][i];', 'J.SetText(AnsiString().sprintf("edByBinFailureCat_RT%03d", i).c_str(), AnsiString(TestIF_File.dByBinFailurePercent[RT][i]));'),
    ('DoIniDataToForm', 'cbByBinFailCountCat_FT[i]->Checked=TestIF_File.bFailCountEnable[FT][i];', 'J.SetChecked(cbByBinFailCountCat_FT[i], TestIF_File.bFailCountEnable[FT][i]);'),
    ('DoIniDataToForm', 'cbByBinFailCountCat_RT[i]->Checked=TestIF_File.bFailCountEnable[RT][i];', 'J.SetChecked(cbByBinFailCountCat_RT[i], TestIF_File.bFailCountEnable[RT][i]);'),
    ('DoIniDataToForm', 'edByBinFailCountCat_FT[i]->Text=TestIF_File.iFailCountLimit[FT][i];', 'J.SetText(edByBinFailCountCat_FT[i], AnsiString(TestIF_File.iFailCountLimit[FT][i]));'),
    ('DoIniDataToForm', 'edByBinFailCountCat_RT[i]->Text=TestIF_File.iFailCountLimit[RT][i];', 'J.SetText(edByBinFailCountCat_RT[i], AnsiString(TestIF_File.iFailCountLimit[RT][i]));'),
    ('DoFormToData', 'TestIF_File.bSpecBinBySiteCompareEnable[FT][i]          =(i<iTestBinCount)?cbByBinSiteGapCat_FT[i]->Checked:false;', 'TestIF_File.bSpecBinBySiteCompareEnable[FT][i]          =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByBinSiteGapCat_FT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.bSpecBinBySiteCompareEnable[RT][i]          =(i<iTestBinCount)?cbByBinSiteGapCat_RT[i]->Checked:false;', 'TestIF_File.bSpecBinBySiteCompareEnable[RT][i]          =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByBinSiteGapCat_RT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.dSpecBinBySiteComparePercent[FT][i]         =CheckRange(atof(edByBinSiteGapCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dSpecBinBySiteComparePercent[FT][i]         =CheckRange(atof(J.GetText(AnsiString().sprintf("edByBinSiteGapCat_FT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.dSpecBinBySiteComparePercent[RT][i]         =CheckRange(atof(edByBinSiteGapCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dSpecBinBySiteComparePercent[RT][i]         =CheckRange(atof(J.GetText(AnsiString().sprintf("edByBinSiteGapCat_RT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]    =(i<iTestBinCount)?cbByArmSiteGapCat_FT[i]->Checked:false;', 'TestIF_File.bSpecBinByArmPerSiteCompareEnable[FT][i]    =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByArmSiteGapCat_FT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]    =(i<iTestBinCount)?cbByArmSiteGapCat_RT[i]->Checked:false;', 'TestIF_File.bSpecBinByArmPerSiteCompareEnable[RT][i]    =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByArmSiteGapCat_RT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]   =CheckRange(atof(edByArmSiteGapCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dSpecBinByArmPerSiteComparePercent[FT][i]   =CheckRange(atof(J.GetText(AnsiString().sprintf("edByArmSiteGapCat_FT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]   =CheckRange(atof(edByArmSiteGapCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dSpecBinByArmPerSiteComparePercent[RT][i]   =CheckRange(atof(J.GetText(AnsiString().sprintf("edByArmSiteGapCat_RT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.bByBinFailureEnable[FT][i]                  =(i<iTestBinCount)?cbByBinFailureCat_FT[i]->Checked:false;', 'TestIF_File.bByBinFailureEnable[FT][i]                  =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByBinFailureCat_FT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.bByBinFailureEnable[RT][i]                  =(i<iTestBinCount)?cbByBinFailureCat_RT[i]->Checked:false;', 'TestIF_File.bByBinFailureEnable[RT][i]                  =(i<iTestBinCount)?J.GetChecked(AnsiString().sprintf("cbByBinFailureCat_RT%03d", i).c_str()):false;'),
    ('DoFormToData', 'TestIF_File.dByBinFailurePercent[FT][i]                 =CheckRange(atof(edByBinFailureCat_FT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dByBinFailurePercent[FT][i]                 =CheckRange(atof(J.GetText(AnsiString().sprintf("edByBinFailureCat_FT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.dByBinFailurePercent[RT][i]                 =CheckRange(atof(edByBinFailureCat_RT[i]->Text.c_str()), dMinYield, dMaxYield);', 'TestIF_File.dByBinFailurePercent[RT][i]                 =CheckRange(atof(J.GetText(AnsiString().sprintf("edByBinFailureCat_RT%03d", i).c_str()).c_str()), dMinYield, dMaxYield);'),
    ('DoFormToData', 'TestIF_File.bFailCountEnable[FT][i]=cbByBinFailCountCat_FT[i]->Checked;', 'TestIF_File.bFailCountEnable[FT][i]=J.GetChecked(cbByBinFailCountCat_FT[i]);'),
    ('DoFormToData', 'TestIF_File.bFailCountEnable[RT][i]=cbByBinFailCountCat_RT[i]->Checked;', 'TestIF_File.bFailCountEnable[RT][i]=J.GetChecked(cbByBinFailCountCat_RT[i]);'),
    ('DoFormToData', 'TestIF_File.iFailCountLimit[FT][i]=CheckRange(atoi(edByBinFailCountCat_FT[i]->Text.c_str()), 0, 100000);', 'TestIF_File.iFailCountLimit[FT][i]=CheckRange(atoi(J.GetText(edByBinFailCountCat_FT[i]).c_str()), 0, 100000);'),
    ('DoFormToData', 'TestIF_File.iFailCountLimit[RT][i]=CheckRange(atoi(edByBinFailCountCat_RT[i]->Text.c_str()), 0, 100000);', 'TestIF_File.iFailCountLimit[RT][i]=CheckRange(atoi(J.GetText(edByBinFailCountCat_RT[i]).c_str()), 0, 100000);'),
]
# ---- AUTO(ws-arrow/multiline) END

FORM = {
    'class': 'TfYieldMonitoring',
    'cpp': 'uYieldMonitoring.cpp',
    'h': 'uYieldMonitoring.h',
    'page': 'Setup.YieldMonitoring.html',
    'struct': 'TestIF_File',
    'files': ['Tester.Data', 'config.ini（CosFunction.bPiggybackFunctionByHandler 時 SaveSetupFileToConfig；本 bridge 不寫，見 blocks）'],
    'methods': ['FormShow', 'DoIniDataToForm', 'btnApplyClick', 'DoFormToData', 'SaveSetupFile'],
    # bYieldAlarmUnlocked：golden 912 表單成員（rf360 Yield 密碼解鎖，btnUnlockYieldAlarmClick 設 true）；bridge 每次請求從 0 開始＝未解鎖。
    'members': ['fShow', 'bYieldAlarmUnlocked'],
    # golden FormCreate :340-495：每個 Category i（0..TEST_MAX_BIN-1）動態 new 12 個元件，Name=sprintf("<前綴>%03d", i)，
    # checkbox 的 Caption=sprintf("% *s%%", -37, "Category i")。版面屬性（Parent/Top/Left/Font/事件）HTML 不用。
    # golden 建構子 :287-338 的 Hint（RP Default ChangeLog 用）、MyYieldPanel（ASE_KaohSiung）不做，見 Todo。
    'display': [
        '// ---- golden FormCreate :340-495：動態 Category 元件的 Caption（頁面元件 id＝golden Name）',
        'for(int i=0; i<TEST_MAX_BIN; i++)\n'
        '    {\n'
        '        AnsiString str, StrC;\n'
        '        str.sprintf("Category %d", i);\n'
        '        StrC.sprintf("% *s%%", -37, str.c_str());\n'
        '        const char* cbs[6]={"cbByBinSiteGapCat_FT%03d", "cbByBinSiteGapCat_RT%03d", "cbByArmSiteGapCat_FT%03d",\n'
        '                            "cbByArmSiteGapCat_RT%03d", "cbByBinFailureCat_FT%03d", "cbByBinFailureCat_RT%03d"};\n'
        '        for(int k=0; k<6; k++)\n'
        '            J.SetCaption(AnsiString().sprintf(cbs[k], i).c_str(), StrC);\n'
        '    }',
        'if(CUSTOMER_CODE==CC_ASE_KaohSiung)\n'
        '        J.Todo("golden ctor :334-337 MyYieldPanel (ASE_KaohSiung ART panel, TMyYieldPanel) not ported -- ART settings not shown");',
        'B_FormShow(J);',
    ],
    'save': 'B_SaveSetupFile',
    'saveFlow': 'B_btnApplyClick',
    # 讀檔器 TfYieldMonitoring::ReadFile 在移植樹 live（uYieldMonitoring.cpp:2676）；CheckSettingNo 只動結構（forms/fYieldMonitoring.h:302）。
    'port_calls': {'CheckSettingNo': 'fYieldMonitoring->CheckSettingNo'},
    'sourceGap': '',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'cAuthority.h', 'forms/fYieldMonitoring.h', 'forms/fMain.h', 'forms/fSecurity.h',
                 'forms/fLotInfo.h', 'LastSet.h', 'csystem.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'aHotPlateSubstrate.h', '<string>', '<stdlib.h>'],
    'blocks': BLOCKS + AUTO_BLOCKS,
    'overrides': AUTO_OVERRIDES + OVERRIDES,
}
