# -*- coding: utf-8 -*-
# tools/formbridge/TfSetup.py -- gen_formbridge.py 的表單設定（一個 BCB 表單一個檔）。
# 欄位說明見 tools/formbridge/README.md。改完跑：python tools/gen_formbridge.py --only TfSetup
#
# Steven 團隊 20260924（S12 第二型，TestIF_File 擴充：TfSetup 半邊）。
# golden cSetUp.cpp（912，cp950）。顯示＝golden FormShow（:1615，內含 ReadFile→DoIniDataToForm→ScrollBar1Change→DoIniDataToForm）
# 前面再補 golden 建構子 :123-216 對 widget 的部分（建構子不是一般方法，產生器抓不到，寫在 display）。
# 存檔鈕＝golden sbUpdateClick（:3492）→ SaveSetupFile(GetRecipePath())（:3655）。
#
# ⚠ 三件事讓這頁「可顯示、不可存」（sourceGap 非空；form.save 會 409）：
#   1. 讀檔端缺口（見 sourceGap）。
#   2. golden SaveSetupFile 只有一個參數 (AnsiString sPath)，產生器的 Save 包裝寫死 `B_SaveSetupFile(J, szDir, S)`
#      → 'save' 不設（設了編不過），沒有空跑、沒有 saveReads。要產生器加規則。
#   3. golden 存檔流程用到「開表單時」的快照（bNeedPassword／bOCRNeedPassword／iASMSiteMap／iTestSiteCh／iASMTestMode、
#      CompChange 決定的 cbXx Visible），bridge 的 form.save 是全新 FormState，這些都是 0／預設值 ——
#      例：bNeedPassword=0 會讓「關 RTC 要密碼」直接跳過。解鎖存檔前要先設計「存檔前重放開表單狀態」。

# ======================================================================
# 手寫的 blocks／overrides（每條上面一行寫原因）
# ======================================================================
# 手寫的 blocks／overrides（每條附原因）。由 build_cfg.py 併進 tools/formbridge/TfSetup.py。
SCH = 'auto SCH=[](int r, int c){ std::string s("cb"); s+=char(\'A\'+r); s+=char(\'a\'+c); return s; };   /* bridge：golden TestSiteCH[r][c] 是 widget 指標陣列（ctor :130-135 = cbAa..cbDh），改成名字 */'
KEY = 'auto KEY=[](const char* n, int r, int c){ std::string s(n); s+=\'_\'; s+=char(\'0\'+r); s+=char(\'0\'+c); return s; };   /* bridge：golden cSetUp.cpp 檔案範圍陣列（iASMSiteMap／iTestSiteCh）改成表單狀態 J.M("<名>_<r><c>") */'
SEN = 'auto SEN=[](int i){ return AnsiString("rgSensor")+AnsiString(i+1); };   /* bridge：golden MyTempRGBox[i] 是 ctor :203 動態產生的 TRadioGroup，Name="rgSensor"+(i+1) */'
LAB = 'auto LROW=[](int r){ std::string s("labRow"); s+=char(\'A\'+r); return s; }; auto LCOL=[](int c){ std::string s("labCol"); s+=char(\'A\'+c); return s; };   /* bridge：golden TestLabRow[]／TestLabCol[]（ctor :136-139）改成名字 */'


def RET_T(m):
    return '{ J.M("ret_%s")=1; return; }   /* golden: return true; —— 產生器一律產 void，回傳值放 J.M("ret_%s") */' % (m, m)


def RET_F(m):
    return '{ J.M("ret_%s")=0; return; }   /* golden: return false; */' % m


BLOCKS = [
    # golden FormShow :2093-2112 矽格北興：在 C:\Windows 建立／讀取 SitMap.ini（site map 密碼），並寫檔案範圍 static
    # bNeedEnterPassword／sSigPassword（移植樹 cSetUp.cpp:2046-2047 是 static，這支看不到）。GET 不可以寫系統資料夾 → 不做，照實回報。
    ('FormShow', 2093, 2112, 'if(CUSTOMER_CODE==CC_SIGURD_PeiXing)',
     'if(CUSTOMER_CODE==CC_SIGURD_PeiXing)                                        //Alick 20160602 新增矽格Site map Password用ini檔\n'
     '    {\n'
     '        J.Todo("golden FormShow :2093-2112 SIGURD_PeiXing site-map password (C:/Windows/SitMap.ini create/read, bNeedEnterPassword/sSigPassword) not done by the bridge");\n'
     '    }'),
    # golden SaveSetupFile :3757（IniConfig.bRTCbySystem 時）直寫 config.ini [RTC] Enable。config.ini 由 FileRW/IniConfig 擁有
    # （C 路 editlist.save）→ 這裡不寫，照實回報。整行換掉（單行 override 只換片段，會留下 WriteIniData 的前半）。
    ('SaveSetupFile', 3757, 3757, '"RTC", "Enable"',
     'J.Todo("golden SaveSetupFile :3757 (IniConfig.bRTCbySystem) writes config.ini [RTC] Enable -- config.ini is owned by FileRW/IniConfig (C route editlist.save); NOT written here. Note: [RTC] Enable is not an elConfig Add() key, so no route writes it yet");'),
]

OVERRIDES = [
    # ---- 名字 lambda：放在各方法第一行（golden 那一行照留）
    ('FormShow', 'GetSetupAuth();', SCH + '\n' + KEY + '\nGetSetupAuth();'),
    ('DoIniDataToForm', 'ScrollBar1->Position = TestIF_File.iTestMode;',
     SCH + '\n' + SEN + '\nJ.SetItemIndex("ScrollBar1", TestIF_File.iTestMode);   // golden: ScrollBar1.Position= —— FormState 沒有 Position，以 itemIndex 承載'),
    ('ScrollBar1Change', 'static bool bFirstRead=true;',
     SCH + '\nstatic bool bFirstRead=true;   /* golden 函式內 static：bridge 裡跨請求保留（第一次 GET 設 asHandlingMode，與 golden 第一次開表單同義） */'),
    ('CompChange', 'int iTestCHCT=SiteData[iMode].Cnt;', SCH + '\n' + LAB + '\nint iTestCHCT=SiteData[iMode].Cnt;'),
    ('sbUpdateClick', 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&',
     SCH + '\nJ.Todo("bridge: golden sbUpdateClick relies on open-time form state (bNeedPassword/bOCRNeedPassword from DoIniDataToForm, iASMSiteMap/iTestSiteCh/iASMTestMode from FormShow, cbXx Visible from CompChange); a form.save FormState starts empty -- not replayed");\n'
     'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&'),
    ('CHSetError', 'bool bSelOne=false;', SCH + '\nbool bSelOne=false;'),
    ('SaveSetupFile', 'AnsiString str;',
     'J.M("saved")=1;   // bridge：golden 存檔函式有被呼叫（form.save 的 ack.saved）；save 欄位沒設，產生器不會自動插這行\n'
     + SCH + '\n' + KEY + '\n' + SEN + '\nAnsiString str;'),

    # ---- ScrollBar1（TScrollBar）Position：FormState 沒有 Position，以 itemIndex 承載（頁面 ScrollBar1 送/收 itemIndex）
    ('FormShow', 'iTestModeOcr    =ScrollBar1->Position;', 'J.M("iTestModeOcr")=J.GetItemIndex("ScrollBar1");   // golden: iTestModeOcr=ScrollBar1.Position'),
    ('FormShow', 'iASMTestMode=ScrollBar1->Position;', 'J.M("iASMTestMode")=J.GetItemIndex("ScrollBar1");   // golden: iASMTestMode=ScrollBar1.Position'),
    ('ScrollBar1Change', 'ScrollBar1->Position=TestIF_File.iTestMode;', 'J.SetItemIndex("ScrollBar1", TestIF_File.iTestMode);   // golden: ScrollBar1.Position='),
    ('ScrollBar1Change', 'ScrollBar1->Position=iMax;', 'J.SetItemIndex("ScrollBar1", iMax);   // golden: ScrollBar1.Position=iMax'),
    ('ScrollBar1Change', 'ScrollBar1->Position=iPos;', 'J.SetItemIndex("ScrollBar1", iPos);   // golden: ScrollBar1.Position=iPos'),
    ('ScrollBar1Change', 'ScrollBar1->Position', 'J.GetItemIndex("ScrollBar1")'),
    ('chkOffCenterkitClick', 'ScrollBar1->Position', 'J.GetItemIndex("ScrollBar1")'),
    ('CHSetError', 'ScrollBar1->Position', 'J.GetItemIndex("ScrollBar1")'),
    ('CheckShuttlePitch', 'ScrollBar1->Position', 'J.GetItemIndex("ScrollBar1")'),
    ('SaveSetupFile', 'TestIF_File.iTestMode=GetTestMode(TestSiteFileName[0][ScrollBar1->Position]);',
     'TestIF_File.iTestMode=fSetup->GetTestMode(TestSiteFileName[0][J.GetItemIndex("ScrollBar1")]);'),
    ('SaveSetupFile', 'ScrollBar1->Position', 'J.GetItemIndex("ScrollBar1")'),
    # ---- golden 呼叫事件處理器
    ('ScrollBar1Change', 'rgYPitchOffsetMode->OnClick(this);', 'B_rgYPitchOffsetModeClick(J);   // golden: rgYPitchOffsetMode.OnClick(this)'),
    # ---- 純畫面：示意圖、版面寬度、表單寬度
    ('FormShow', 'Image1->Picture->LoadFromFile(BmpPath+"2site.bmp");', '/* golden: Image1.Picture.LoadFromFile(BmpPath+"2site.bmp") —— 示意圖，HTML 自己有 */'),
    ('chkOffCenterkitClick', 'Image1->Picture->LoadFromFile(', '/* golden 示意圖（HTML 自己有）: Image1.Picture.LoadFromFile( */ (void)('),
    ('rgYPitchOffsetModeClick', 'labYOffset->Width=93;', '/* golden（版面寬度，HTML 不用）: labYOffset.Width=93; */'),
    ('rgYPitchOffsetModeClick', 'labYOffset->Width=134;', '/* golden（版面寬度，HTML 不用）: labYOffset.Width=134; */'),
    ('ScrollBar1Change', 'fSetup->Width=818;', '/* golden（表單寬度，HTML 不用）: fSetup.Width=818; */'),
    ('ScrollBar1Change', 'fSetup->Width=1090;', '/* golden（表單寬度，HTML 不用）: fSetup.Width=1090; */'),

    # ---- TestSiteCH[r][c]（cbAa..cbDh）
    ('FormShow', 'TestSiteCH[i][j]->ItemIndex =TestIF_File.iSiteMap[i][j];', 'J.SetItemIndex(SCH(i,j).c_str(), TestIF_File.iSiteMap[i][j]);'),
    ('FormShow', 'iTestSiteCh[i][j]=TestIF_File.iSiteMap[i][j];', 'J.M(KEY("iTestSiteCh",i,j).c_str())=TestIF_File.iSiteMap[i][j];'),
    ('FormShow', 'iASMSiteMap[i][j]=TestSiteCH[i][j]->ItemIndex;', 'J.M(KEY("iASMSiteMap",i,j).c_str())=J.GetItemIndex(SCH(i,j).c_str());'),
    ('DoIniDataToForm', 'TestSiteCH[i][j]->ItemIndex=TestIF_File.iSiteMap[i][j];', 'J.SetItemIndex(SCH(i,j).c_str(), TestIF_File.iSiteMap[i][j]);'),
    ('ScrollBar1Change', 'TestSiteCH[i][j]->ItemIndex=TestIF_File.iSiteMap[i][j];', 'J.SetItemIndex(SCH(i,j).c_str(), TestIF_File.iSiteMap[i][j]);'),
    ('ScrollBar1Change', 'TestSiteCH[i][j]->ItemIndex=TestIF.iSiteMap[i][j];', 'J.SetItemIndex(SCH(i,j).c_str(), TestIF.iSiteMap[i][j]);'),
    ('CompChange', 'TestLabRow[i]->Visible=false;', 'J.SetVisible(LROW(i).c_str(), false);'),
    ('CompChange', 'TestSiteCH[i][j]->Clear();', 'J.ItemsClear(SCH(i,j).c_str());'),
    ('CompChange', 'TestSiteCH[i][j]->Visible=false;', 'J.SetVisible(SCH(i,j).c_str(), false);'),
    ('CompChange', 'TestLabCol[j]->Visible=false;', 'J.SetVisible(LCOL(j).c_str(), false);'),
    ('CompChange', 'TestLabCol[i]->Visible=true;', 'J.SetVisible(LCOL(i).c_str(), true);'),
    ('CompChange', 'TestSiteCH[j][i]->Visible=true;', 'J.SetVisible(SCH(j,i).c_str(), true);'),
    ('CompChange', 'TestSiteCH[j][i]->Enabled=true;', 'J.SetEnabled(SCH(j,i).c_str(), true);'),
    ('CompChange', 'TestSiteCH[j][i]->Items->Add("- - -");', 'J.ItemsAdd(SCH(j,i).c_str(), "- - -");'),
    ('CompChange', 'TestSiteCH[j][i]->Items->Add("CH "+AnsiString (k));', 'J.ItemsAdd(SCH(j,i).c_str(), "CH "+AnsiString (k));'),
    ('CompChange', 'TestLabRow[j]->Visible=true;', 'J.SetVisible(LROW(j).c_str(), true);'),
    ('sbUpdateClick', 'TestSiteCH[i][j]->ItemIndex=0;', 'J.SetItemIndex(SCH(i,j).c_str(), 0);'),
    ('CHSetError', 'if(TestSiteCH[iRow1][iCol1]->ItemIndex==-1 &&', 'if(J.GetItemIndex(SCH(iRow1,iCol1).c_str())==-1 &&'),
    ('CHSetError', 'TestSiteCH[iRow1][iCol1]->Visible)', 'J.GetVisible(SCH(iRow1,iCol1).c_str()))'),
    ('CHSetError', 'if(TestSiteCH[iRow1][iCol1]->Visible &&', 'if(J.GetVisible(SCH(iRow1,iCol1).c_str()) &&'),
    ('CHSetError', 'TestSiteCH[iRow2][iCol2]->Visible)', 'J.GetVisible(SCH(iRow2,iCol2).c_str()))'),
    ('CHSetError', 'if(TestSiteCH[iRow1][iCol1]->ItemIndex==TestSiteCH[iRow2][iCol2]->ItemIndex &&',
     'if(J.GetItemIndex(SCH(iRow1,iCol1).c_str())==J.GetItemIndex(SCH(iRow2,iCol2).c_str()) &&'),
    ('CHSetError', 'TestSiteCH[iRow1][iCol1]->ItemIndex!=0)', 'J.GetItemIndex(SCH(iRow1,iCol1).c_str())!=0)'),
    ('CHSetError', 'if(TestSiteCH[iRow1][iCol1]->ItemIndex>=1 ||', 'if(J.GetItemIndex(SCH(iRow1,iCol1).c_str())>=1 ||'),
    ('CHSetError', 'TestSiteCH[iRow2][iCol2]->ItemIndex>=1)', 'J.GetItemIndex(SCH(iRow2,iCol2).c_str())>=1)'),
    ('CHSetError', 'TestSiteCH[0][0]->ItemIndex=1;', 'J.SetItemIndex(SCH(0,0).c_str(), 1);'),
    ('SaveSetupFile', 'if(TestSiteCH[i][j]->ItemIndex==0)', 'if(J.GetItemIndex(SCH(i,j).c_str())==0)'),
    ('SaveSetupFile', 'TestSiteCH[i][j]->ItemIndex=K;', 'J.SetItemIndex(SCH(i,j).c_str(), K);'),
    ('SaveSetupFile', 'WriteIniData(szDir, "Configuration", str, TestSiteCH[i][j]->ItemIndex);',
     'WriteIniData(szDir, "Configuration", str, J.GetItemIndex(SCH(i,j).c_str()));'),
    ('SaveSetupFile', 'iSite = TestSiteCH[i][j]->ItemIndex;', 'iSite = J.GetItemIndex(SCH(i,j).c_str());'),
    ('SaveSetupFile', 'if(iASMTestMode!=TestIF_File.iTestMode)', 'if(J.M("iASMTestMode")!=TestIF_File.iTestMode)'),
    ('SaveSetupFile', 'if(iASMSiteMap[i][j]!=TestSiteCH[i][j]->ItemIndex)', 'if(J.M(KEY("iASMSiteMap",i,j).c_str())!=J.GetItemIndex(SCH(i,j).c_str()))'),
    ('SaveSetupFile', 'if(iASMSiteMap[i][j]==0)', 'if(J.M(KEY("iASMSiteMap",i,j).c_str())==0)'),
    ('SaveSetupFile', 'if(TestSiteCH[i][j]->ItemIndex!=0)', 'if(J.GetItemIndex(SCH(i,j).c_str())!=0)'),
    ('SaveSetupFile', 'if(iTestSiteCh[i][j]!=TestIF_File.iSiteMap[i][j])', 'if(J.M(KEY("iTestSiteCh",i,j).c_str())!=TestIF_File.iSiteMap[i][j])'),
    ('SaveSetupFile', 'iTestSiteCh[i][j]=TestIF_File.iSiteMap[i][j];', 'J.M(KEY("iTestSiteCh",i,j).c_str())=TestIF_File.iSiteMap[i][j];'),

    # ---- MyTempRGBox[i]（rgSensor1..N）
    ('DoIniDataToForm', 'MyTempRGBox[i]->ItemIndex=TestIF_File.iSensorCheckType[i];', 'J.SetItemIndex(SEN(i).c_str(), TestIF_File.iSensorCheckType[i]);'),
    ('DoIniDataToForm', 'MyTempRGBox[i]->Visible=true;', 'J.SetVisible(SEN(i).c_str(), true);'),
    ('DoIniDataToForm', 'if(MyTempRGBox[i]->ItemIndex==0)', 'if(J.GetItemIndex(SEN(i).c_str())==0)'),
    ('DoIniDataToForm', 'MyTempRGBox[i]->ItemIndex=2;', 'J.SetItemIndex(SEN(i).c_str(), 2);'),
    ('DoIniDataToForm', 'MyTempRGBox[i]->ItemIndex=1;', 'J.SetItemIndex(SEN(i).c_str(), 1);'),
    ('DoIniDataToForm', 'MyTempRGBox[i]->Visible=false;', 'J.SetVisible(SEN(i).c_str(), false);'),
    ('DoIniDataToForm', 'MyTempRGBox[i]->ItemIndex=0;', 'J.SetItemIndex(SEN(i).c_str(), 0);'),
    ('SaveSetupFile', 'WriteIniData(szDir, "Configuration", str, MyTempRGBox[i]->ItemIndex);',
     'WriteIniData(szDir, "Configuration", str, J.GetItemIndex(SEN(i).c_str()));'),

    # ---- golden 912 的欄位，移植樹 SYSTEM_TEST_IF（cprod.h，906 版）沒有 → 顯示 false 並回報
    ('DoIniDataToForm', 'cbbDualSiteUseOneSuck ->Checked      =TestIF_File.bDualSiteUseOneSuck;',
     'J.SetChecked("cbbDualSiteUseOneSuck", false); J.Todo("golden DoIniDataToForm: TestIF_File.bDualSiteUseOneSuck (912) is missing from the port SYSTEM_TEST_IF -- shown false");'),
    ('DoIniDataToForm', 'cbPreventDropfunction->Checked =TestIF_File.bPreventDropfunction;',
     'J.SetChecked("cbPreventDropfunction", false); J.Todo("golden DoIniDataToForm: TestIF_File.bPreventDropfunction (912) is missing from the port SYSTEM_TEST_IF -- shown false");'),

    # ---- 移植樹門面沒有的成員（與移植樹 cSetUp.cpp 的 GATE 同一個原因）
    ('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;',
     'J.Todo("golden DoIniDataToForm (JCET_FOR_EVAN): fMain cbDisableSiteMappingCheck.Visible -- TfMain facade has no such checkbox (port GATE G-SU-SiteMapChk)");'),
    ('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=false;',
     'J.Todo("golden DoIniDataToForm (JCET_FOR_EVAN): fMain cbDisableSiteMappingCheck.Visible=false -- TfMain facade has no such checkbox (port GATE G-SU-SiteMapChk)");'),
    ('FormShow', 'fConfiguration->ReadLockByFile();',
     'J.Todo("golden FormShow: fConfiguration->ReadLockByFile() -- the only fConfiguration in the port is W5SckArtRem_ConfigStub without ReadLockByFile (port GATE G-SU-LockByFile)");'),
    ('ScrollBar1Change', 'shtMode.Clear();', 'fSetup->shtMode.Clear();   // golden 表單成員 shtMode —— 放在移植樹 fSetup 實例（SetShtMode／VertifyShtModeisDiff 同一份）'),
    ('sbUpdateClick', 'fMain->bNeedRestartProgram=true;',
     'J.Todo("golden sbUpdateClick: fMain->bNeedRestartProgram=true -- TfMain facade has no bNeedRestartProgram (port GATE G-SU-Restart); restart request dropped");'),
    ('sbUpdateClick', 'fMain->bEnableAutoclean();',
     'J.Todo("golden sbUpdateClick: fMain->bEnableAutoclean() -- TfMain facade has no bEnableAutoclean (port GATE G-SU-AcBtn)");'),
    ('sbUpdateClick', 'fContact->DutCount();',
     'J.Todo("golden sbUpdateClick: fContact->DutCount() -- port fContact is TfContactShim (atester_shims.h:251) without DutCount");'),
    ('sbUpdateClick', 'fContact->ReadFile();',
     'J.Todo("golden sbUpdateClick: fContact->ReadFile() -- port fContact is TfContactShim without ReadFile; Contact.Data NOT reloaded");'),
    ('SaveSetupFile', 'fContact->ReadFile();',
     'J.Todo("golden SaveSetupFile: fContact->ReadFile() after forcing Test Arm Contact=-50 -- port fContact is TfContactShim without ReadFile");'),
    ('SaveSetupFile', 'fCleaning->LoadAutoCleanData();',
     'J.Todo("golden SaveSetupFile: fCleaning->LoadAutoCleanData() -- TfCleaning facade has no LoadAutoCleanData (port GATE G-SU-Clean)");'),
    # ---- 移植樹 ADAM_WriteVoltage 是空函式（atester_shims.cpp:327 `{}`）：照叫，照實回報（KYEC_LEE 才走到）
    ('sbUpdateClick', 'ADAM_WriteVoltage(DeviceForm.dPress);',
     'ADAM_WriteVoltage(DeviceForm.dPress); J.Todo("ADAM_WriteVoltage() is an empty shim in the port (atester_shims.cpp:327) -- regulator voltage not written");'),
    # ---- DoPassword：golden 會彈密碼表單（fMain->cbUserSelectChange／stOperatorClick）。bridge 不能彈窗 → 一律當「密碼沒過」
    #      （golden 沒過的分支＝把 RTC／OCR 勾回去，不關）。與 IniConfig 的 DoPassword_MBox 同一個處理。
    ('sbUpdateClick', 'if(DoPassword()==false)',
     'if((J.Todo("golden sbUpdateClick: DoPassword() (password dialog) not available in the bridge -- treated as failed, RTC/OCR stay enabled"), false)==false)'),
    # ---- golden bool 方法：產生器產 void → 回傳值經 J.M("ret_<方法>")
    ('sbUpdateClick', 'if(CHSetError())', 'if((B_CHSetError(J), J.M("ret_CHSetError")))'),
    ('sbUpdateClick', 'if(CheckShuttlePitch()==false)', 'if((B_CheckShuttlePitch(J), J.M("ret_CheckShuttlePitch"))==false)'),
    ('CHSetError', 'return true;', RET_T('CHSetError')),
    ('CHSetError', 'return false;', RET_F('CHSetError')),
    ('CheckShuttlePitch', 'return false;', RET_F('CheckShuttlePitch')),
    ('CheckShuttlePitch', 'return true;', RET_T('CheckShuttlePitch')),
    ('CheckShuttlePitch', 'return bShuttleSensorCanMove;',
     '{ J.M("ret_CheckShuttlePitch")=bShuttleSensorCanMove; return; }   /* golden: return bShuttleSensorCanMove; */'),
    # ---- vclcompat 沒有 IncludeTrailingPathDelimiter；IncludeTrailingBackslash 同義（Public/HTEditList.cpp:643 同做法）
    # ---- 移植樹 TfMain::ShowTestHeadComp 是空函式（forms/fMain.cpp:247 `{}`）：照叫，照實回報
    ('SaveSetupFile', 'fMain->ShowTestHeadComp(bdiff);',
     'fMain->ShowTestHeadComp(bdiff); J.Todo("fMain->ShowTestHeadComp() is an empty body in the port (forms/fMain.cpp:247) -- test-head site display not refreshed");'),
    ('SaveSetupFile', 'fMain->ShowTestHeadComp(bSiteMapHasChange);',
     'fMain->ShowTestHeadComp(J.M("bSiteMapHasChange")); J.Todo("fMain->ShowTestHeadComp() is an empty body in the port (forms/fMain.cpp:247) -- test-head site display not refreshed");'),
    ('SaveSetupFile', 'fMain->ShowTestHeadComp(false);',
     'fMain->ShowTestHeadComp(false); J.Todo("fMain->ShowTestHeadComp() is an empty body in the port (forms/fMain.cpp:247) -- test-head site display not refreshed");'),
    # ---- handlerlog.h 與本 TU 已含的 language.h 重複定義 TWinControl／TForm（編不過）；移植樹也沒有任何地方呼叫 Save_SiteStatusLog
    ('SaveSetupFile', 'myLog.Save_SiteStatusLog();',
     'J.Todo("golden SaveSetupFile: myLog.Save_SiteStatusLog() (site on/off log) -- handlerlog.h cannot be included next to language.h in this TU; no port caller either");'),
    ('SaveSetupFile', 'IncludeTrailingPathDelimiter(sPath)', 'IncludeTrailingBackslash(sPath)'),
]

# ---- AUTO(ws-arrow/multiline) BEGIN
# 由 autogen.py（Steven 團隊 scratchpad 工具）產生，不要手改：每條是 golden 那一行（或一段）照產生器同樣的規則改寫，
# 只多做：去掉 -> 前的對齊空白、把跨行右式收成一行、widget 指標陣列 ARR[i]-> 換成名字（PTR_ARRAYS）、
# 區域 widget 指標陣列宣告換成名字陣列。
# 原因：產生器以行為單位 —— golden 右式跨行、區域陣列初始化跨行，逐行改寫會編不過
AUTO_BLOCKS = [
    ('FormShow', 1847, 1849, 'grpUseXCenterPitch->Visible=(C', 'J.SetVisible("grpUseXCenterPitch", (CosFunction.b2x4SupportCenterPitch==true && (TestIF_File.iTestMode==_8Site2X4 || TestIF_File.iTestMode==_16Site4X4)));   //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC //Sam 20190226 : 16Site4X4   /* golden 1847-1849 右式跨行，整條收成一行 */'),
    ('FormShow', 1903, 1906, 'rgInOutArmYPitch->Visible=(USE', 'J.SetVisible("rgInOutArmYPitch", (USE_IN_OUT_ARM_Y_PITCH==iXPitchManual635 || USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360 || USE_OUT_ARM_Y_PITCH==iXPitchManual635 || USE_OUT_ARM_Y_PITCH==iXPitchManual360));   //JerryYang 20251218 : IN/OUT ARM支援不同模組   /* golden 1903-1906 右式跨行，整條收成一行 */'),
    ('FormShow', 2055, 2057, 'paQualSite2X2Shift->Visible=((', 'J.SetVisible("paQualSite2X2Shift", ((TestIF_File.iTestMode==QualSite2X2 || TestIF_File.iTestMode==DualSite) && TestIF_File.bQualSite2X2Shift));   //kevin 20170415 (wei) add DualSite //wei 20160226 TSMC X Shift   /* golden 2055-2057 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 398, 399, 'grpUseXCenterPitch->Visible=(C', 'J.SetVisible("grpUseXCenterPitch", (CosFunction.b2x4SupportCenterPitch==true && iCheckPos==_8Site2X4));   //Steven 20170706 (wei) : 2x4中間的Pitch不同 for SCC   /* golden 398-399 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 449, 453, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 449-453 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 512, 516, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 512-516 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 518, 519, 'cbUseRotateForHT7000HPKit->Vis', 'J.SetVisible("cbUseRotateForHT7000HPKit", (CosFunction.bRotateUseHT7000HPKit && USE_ROTATE_KIT));   //Sam 20210416 : 新增特殊模式 For Rotate Function HT7000 HP Kit   /* golden 518-519 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 538, 542, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 538-542 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 573, 577, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 573-577 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 608, 612, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 608-612 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 683, 687, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 683-687 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 806, 810, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 806-810 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 838, 842, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 838-842 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 970, 974, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 970-974 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 998, 1002, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 998-1002 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 1028, 1032, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 1028-1032 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 1040, 1042, 'cb12Site10DirectHeater->Visibl', 'J.SetVisible("cb12Site10DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N || USE_16_HEATER==eht32HeaterKT4H || USE_16_HEATER==eht32HeaterDTME08));   //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 1040-1042 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 1071, 1075, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N || USE_16_HEATER==eht32HeaterKT4H || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //Steven 20140923 : Index use EJ1N 32 heater //Steven 20150211 : Index use KT4H 32 heater //Ifor 20170814 (Steven) Add HT9046AT Direct Heater //AI(ht9045-heater-control) 20260812 (RogerYang) : 10-Site missed in JimmyChiu 20210923 DTME08 sweep   /* golden 1071-1075 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 1083, 1085, 'cb12Site10DirectHeater->Visibl', 'J.SetVisible("cb12Site10DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N || USE_16_HEATER==eht32HeaterKT4H || USE_16_HEATER==eht32HeaterDTME08));   //Steven 20140923 : Index use EJ1N 32 heater //Steven 20150211 : Index use KT4H 32 heater //AI(ht9045-heater-control) 20260812 (RogerYang) : 10-Site add DTME08 32 heater, align 12-Site   /* golden 1083-1085 右式跨行，整條收成一行 */'),
    ('ScrollBar1Change', 1134, 1138, 'cb16DirectHeater->Visible=(USE', 'J.SetVisible("cb16DirectHeater", (USE_16_HEATER==eht32HeaterEJ1N  || USE_16_HEATER==eht32HeaterKT4H  || (ATC_SYSTEM==eNewATCSystem && iATC_Use_Heat_Count>=16) || USE_16_HEATER==eht32HeaterDTME08));   //kevin 20191202 add 12 以下 direct //Steven 20140923 : Index使用EJ1N版32組加熱器 //Steven 20150211 : Index使用KT4H版32組加熱器 //Ifor 20170814 (Steven) Add HT9046AT Direct Heater開關 //JimmyChiu 20210923 : Index使用DTME08版32組加熱器   /* golden 1134-1138 右式跨行，整條收成一行 */'),
]
# 原因：產生器的 regex 是 \bW->，golden 的 `W   ->Prop` 對齊寫法與 `ARR[i]->Prop` 不會被改寫，也不會被當成殘留抓出來
AUTO_OVERRIDES = [
    ('FormShow', 'ScrollBar1          ->Visible=false;', 'J.SetVisible("ScrollBar1", false);'),
    ('FormShow', 'XPitch              ->Enabled=false;', 'J.SetEnabled("XPitch", false);'),
    ('FormShow', 'YPitch              ->Enabled=false;', 'J.SetEnabled("YPitch", false);'),
    ('FormShow', 'rgSelectSearchLast  ->Enabled=false;', 'J.SetEnabled("rgSelectSearchLast", false);'),
    ('FormShow', 'rgUseSuckMode       ->Enabled=false;', 'J.SetEnabled("rgUseSuckMode", false);'),
    ('FormShow', 'grpSiteMap           ->Enabled=false;', 'J.SetEnabled("grpSiteMap", false);'),
    ('FormShow', 'chkOffCenterkit     ->Enabled=false;', 'J.SetEnabled("chkOffCenterkit", false);'),
    ('FormShow', 'rgInOutArmYPitch    ->Enabled=false;', 'J.SetEnabled("rgInOutArmYPitch", false);'),
    ('FormShow', 'grpIndexOption      ->Enabled=false;', 'J.SetEnabled("grpIndexOption", false);'),
    ('FormShow', 'grpIndexOption    ->Enabled=true;', 'J.SetEnabled("grpIndexOption", true);'),
    ('FormShow', 'edtRTCFileName    ->Enabled=true;', 'J.SetEnabled("edtRTCFileName", true);'),
    ('FormShow', 'rgInOutArmYPitch  ->Enabled=true;', 'J.SetEnabled("rgInOutArmYPitch", true);'),
    ('FormShow', 'ScrollBar1          ->Visible=fSecurity->Insufficient(21,false);', 'J.SetVisible("ScrollBar1", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'XPitch              ->Enabled=fSecurity->Insufficient(21,false);', 'J.SetEnabled("XPitch", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'YPitch              ->Enabled=fSecurity->Insufficient(21,false);', 'J.SetEnabled("YPitch", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'rgSelectSearchLast  ->Enabled=fSecurity->Insufficient(21,false);', 'J.SetEnabled("rgSelectSearchLast", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'rgUseSuckMode       ->Enabled=fSecurity->Insufficient(21,false);', 'J.SetEnabled("rgUseSuckMode", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'chkOffCenterkit ->Enabled=fSecurity->Insufficient(21,false);', 'J.SetEnabled("chkOffCenterkit", fSecurity->Insufficient(21,false));'),
    ('FormShow', 'chkOffCenterkit ->Enabled=false;', 'J.SetEnabled("chkOffCenterkit", false);'),
    ('DoIniDataToForm', 'chkOffCenterkit     ->Checked   = TestIF_File.bNS7000kit;', 'J.SetChecked("chkOffCenterkit", TestIF_File.bNS7000kit);'),
    ('DoIniDataToForm', 'chkNS7000CS         ->Checked   = TestIF_File.bNS7000CS;', 'J.SetChecked("chkNS7000CS", TestIF_File.bNS7000CS);'),
    ('DoIniDataToForm', 'chkRotateShuttle    ->Checked   = TestIF_File.bRotateShuttle;', 'J.SetChecked("chkRotateShuttle", TestIF_File.bRotateShuttle);'),
    ('DoIniDataToForm', 'cbEnableRealTimeCCD ->Checked   = !COM2->bCCDDummyRum;', 'J.SetChecked("cbEnableRealTimeCCD", !COM2->bCCDDummyRum);'),
    ('DoIniDataToForm', 'cb16change8DirectHeater ->Checked=TestIF_File.b16Direct8Shuttle;', 'J.SetChecked("cb16change8DirectHeater", TestIF_File.b16Direct8Shuttle);'),
    ('DoIniDataToForm', 'cbEnableSocketFloat ->Checked   =TestIF_File.bUseSocketFloat;', 'J.SetChecked("cbEnableSocketFloat", TestIF_File.bUseSocketFloat);'),
    ('DoIniDataToForm', 'cbEnableSocketFloat ->Visible   =false;', 'J.SetVisible("cbEnableSocketFloat", false);'),
    ('DoIniDataToForm', 'cbNS8000H       ->Checked   =TestIF_File.bNS8000CS;', 'J.SetChecked("cbNS8000H", TestIF_File.bNS8000CS);'),
    ('DoIniDataToForm', 'cb1CableLayoutKit   ->Checked   =TestIF_File.b1CableLayoutKit;', 'J.SetChecked("cb1CableLayoutKit", TestIF_File.b1CableLayoutKit);'),
    ('DoIniDataToForm', 'cb6CableLayoutKit   ->Checked   = TestIF_File.b6CableLayoutKit;', 'J.SetChecked("cb6CableLayoutKit", TestIF_File.b6CableLayoutKit);'),
    ('DoIniDataToForm', 'cbOcrFunction       ->Checked   =TestIF_File.bOcrFunction;', 'J.SetChecked("cbOcrFunction", TestIF_File.bOcrFunction);'),
    ('DoIniDataToForm', 'chkOctal80          ->Checked   = TestIF_File.bOctal_80Kit;', 'J.SetChecked("chkOctal80", TestIF_File.bOctal_80Kit);'),
    ('DoIniDataToForm', 'cbSocketSensor      ->Checked   =true;', 'J.SetChecked("cbSocketSensor", true);'),
    ('DoIniDataToForm', 'cbSocketSensor      ->Enabled   =false;', 'J.SetEnabled("cbSocketSensor", false);'),
    ('DoIniDataToForm', 'cbSocketSensor      ->Checked   = TestIF_File.bEnSocketSensor;', 'J.SetChecked("cbSocketSensor", TestIF_File.bEnSocketSensor);'),
    ('DoIniDataToForm', 'cbSocketSensor      ->Enabled   = (AccessLevel>=LevelSet.AccessLevel[159]);', 'J.SetEnabled("cbSocketSensor", (AccessLevel>=LevelSet.AccessLevel[159]));'),
    ('DoIniDataToForm', 'CoSocketCombo       ->ItemIndex = TestIF_File.iSocketCount-1;', 'J.SetItemIndex("CoSocketCombo", TestIF_File.iSocketCount-1);'),
    ('DoIniDataToForm', 'cbOctal16Site       ->Checked   =TestIF_File.bOctal_16Kit;', 'J.SetChecked("cbOctal16Site", TestIF_File.bOctal_16Kit);'),
    ('DoIniDataToForm', 'cbSquareOctalLayout ->Checked   = TestIF_File.bSquare_OctalKit;', 'J.SetChecked("cbSquareOctalLayout", TestIF_File.bSquare_OctalKit);'),
    ('DoIniDataToForm', 'chk2x2Use16siteSLK  ->Checked   =TestIF_File.b2x2Use16SiteKit;', 'J.SetChecked("chk2x2Use16siteSLK", TestIF_File.b2x2Use16SiteKit);'),
    ('DoIniDataToForm', 'cb1x2Use1x4siteSLK  ->Checked   = TestIF_File.b1x2Use1x4SiteKit;', 'J.SetChecked("cb1x2Use1x4siteSLK", TestIF_File.b1x2Use1x4SiteKit);'),
    ('DoIniDataToForm', 'cbUse1x3siteSLK     ->Checked   = TestIF_File.bUse1x3SiteKit;', 'J.SetChecked("cbUse1x3siteSLK", TestIF_File.bUse1x3SiteKit);'),
    ('DoIniDataToForm', 'chk12SiteUse2x8SLK  ->Checked   = TestIF_File.b2x6Use2x8SitSLK;', 'J.SetChecked("chk12SiteUse2x8SLK", TestIF_File.b2x6Use2x8SitSLK);'),
    ('DoIniDataToForm', 'cb16DirectHeater    ->Checked   = TestIF_File.bUse32Heater;', 'J.SetChecked("cb16DirectHeater", TestIF_File.bUse32Heater);'),
    ('DoIniDataToForm', 'Arm1PickArm2Test    ->Checked   = TestIF_File.bArm1PickPlaceArm2Test;', 'J.SetChecked("Arm1PickArm2Test", TestIF_File.bArm1PickPlaceArm2Test);'),
    ('DoIniDataToForm', 'cbCheckArm2Vacuum   ->Checked   = TestIF_File.bCheckArm2Vacuum;', 'J.SetChecked("cbCheckArm2Vacuum", TestIF_File.bCheckArm2Vacuum);'),
    ('DoIniDataToForm', 'cbUseSLKClamp       ->Checked   = TestIF_File.bUseSLKClamp;', 'J.SetChecked("cbUseSLKClamp", TestIF_File.bUseSLKClamp);'),
    ('DoIniDataToForm', 'rgseparabilityTest  ->ItemIndex = TestIF_File.iSeparabilityTest;', 'J.SetItemIndex("rgseparabilityTest", TestIF_File.iSeparabilityTest);'),
    ('DoIniDataToForm', 'cbEnablePreciser            ->Checked   = TestIF_File.bEnableUsePreciser;', 'J.SetChecked("cbEnablePreciser", TestIF_File.bEnableUsePreciser);'),
    ('DoIniDataToForm', 'cbEnabledPreciserRT         ->Checked   = TestIF_File.bEnableRTPreciser;', 'J.SetChecked("cbEnabledPreciserRT", TestIF_File.bEnableRTPreciser);'),
    ('DoIniDataToForm', 'cbEnablePreciserHotPlate    ->Checked   = TestIF_File.bEnablePreciserHotPlate;', 'J.SetChecked("cbEnablePreciserHotPlate", TestIF_File.bEnablePreciserHotPlate);'),
    ('DoIniDataToForm', 'cbEnablePreciserHotPlate    ->Visible   =false;', 'J.SetVisible("cbEnablePreciserHotPlate", false);'),
    ('DoIniDataToForm', 'cbSingleUseOtherSuck ->Checked =TestIF_File.bSingleUseOtherSuck;', 'J.SetChecked("cbSingleUseOtherSuck", TestIF_File.bSingleUseOtherSuck);'),
    ('DoIniDataToForm', 'cbSingleInArmUseOtherSuck ->Checked =TestIF_File.bSingleInArmUseOtherSuck;', 'J.SetChecked("cbSingleInArmUseOtherSuck", TestIF_File.bSingleInArmUseOtherSuck);'),
    ('ScrollBar1Change', 'XPitch                  ->Visible=true;', 'J.SetVisible("XPitch", true);'),
    ('ScrollBar1Change', 'lblXPitch               ->Visible=true;', 'J.SetVisible("lblXPitch", true);'),
    ('ScrollBar1Change', 'lblXPMM                 ->Visible=true;', 'J.SetVisible("lblXPMM", true);'),
    ('ScrollBar1Change', 'cbNS8000H               ->Visible=false;', 'J.SetVisible("cbNS8000H", false);'),
    ('ScrollBar1Change', 'chkOctal80              ->Visible=false;', 'J.SetVisible("chkOctal80", false);'),
    ('ScrollBar1Change', 'cb2CableLayoutKit       ->Visible=false;', 'J.SetVisible("cb2CableLayoutKit", false);'),
    ('ScrollBar1Change', 'cb1CableLayoutKit       ->Visible=false;', 'J.SetVisible("cb1CableLayoutKit", false);'),
    ('ScrollBar1Change', 'cb6CableLayoutKit       ->Visible=false;', 'J.SetVisible("cb6CableLayoutKit", false);'),
    ('ScrollBar1Change', 'cbOctal16Site           ->Visible=false;', 'J.SetVisible("cbOctal16Site", false);'),
    ('ScrollBar1Change', 'chk12SiteUse2x8SLK      ->Visible=false;', 'J.SetVisible("chk12SiteUse2x8SLK", false);'),
    ('ScrollBar1Change', 'cbSquareOctalLayout     ->Visible=false;', 'J.SetVisible("cbSquareOctalLayout", false);'),
    ('ScrollBar1Change', 'chk2x2Use16siteSLK      ->Visible=false;', 'J.SetVisible("chk2x2Use16siteSLK", false);'),
    ('ScrollBar1Change', 'cb1x2Use1x4siteSLK      ->Visible=false;', 'J.SetVisible("cb1x2Use1x4siteSLK", false);'),
    ('ScrollBar1Change', 'cbUse1x3siteSLK         ->Visible=false;', 'J.SetVisible("cbUse1x3siteSLK", false);'),
    ('ScrollBar1Change', 'cbOctal12Site           ->Visible=false;', 'J.SetVisible("cbOctal12Site", false);'),
    ('ScrollBar1Change', 'labYOffset              ->Visible=false;', 'J.SetVisible("labYOffset", false);'),
    ('ScrollBar1Change', 'edYOffset               ->Visible=false;', 'J.SetVisible("edYOffset", false);'),
    ('ScrollBar1Change', 'rgYOffset               ->Visible=false;', 'J.SetVisible("rgYOffset", false);'),
    ('ScrollBar1Change', 'cb16DirectHeater        ->Visible=false;', 'J.SetVisible("cb16DirectHeater", false);'),
    ('ScrollBar1Change', 'cb12Site10DirectHeater  ->Visible=false;', 'J.SetVisible("cb12Site10DirectHeater", false);'),
    ('ScrollBar1Change', 'cbHotechLayoutKit2x2    ->Visible=false;', 'J.SetVisible("cbHotechLayoutKit2x2", false);'),
    ('ScrollBar1Change', 'cbQualSite2X2Shift      ->Visible=false;', 'J.SetVisible("cbQualSite2X2Shift", false);'),
    ('ScrollBar1Change', 'paQualSite2X2Shift      ->Visible=false;', 'J.SetVisible("paQualSite2X2Shift", false);'),
    ('ScrollBar1Change', 'gbInUseBackRow          ->Visible=false;', 'J.SetVisible("gbInUseBackRow", false);'),
    ('ScrollBar1Change', 'gbOutUseBackRow         ->Visible=false;', 'J.SetVisible("gbOutUseBackRow", false);'),
    ('ScrollBar1Change', 'cbQualSite2X2Shift      ->Checked=false;', 'J.SetChecked("cbQualSite2X2Shift", false);'),
    ('ScrollBar1Change', 'cb16change8DirectHeater ->Visible=false;', 'J.SetVisible("cb16change8DirectHeater", false);'),
    ('ScrollBar1Change', 'cb16change8DirectHeater ->Checked=false;', 'J.SetChecked("cb16change8DirectHeater", false);'),
    ('ScrollBar1Change', 'rgYPitchOffsetMode      ->Visible=false;', 'J.SetVisible("rgYPitchOffsetMode", false);'),
    ('ScrollBar1Change', 'cbIndSLK                ->Visible=false;', 'J.SetVisible("cbIndSLK", false);'),
    ('ScrollBar1Change', 'cbSingleUseOtherSuck    ->Visible=false;', 'J.SetVisible("cbSingleUseOtherSuck", false);'),
    ('ScrollBar1Change', 'cbbDualSiteUseOneSuck   ->Visible=false;', 'J.SetVisible("cbbDualSiteUseOneSuck", false);'),
    ('ScrollBar1Change', 'chkOffCenterkit         ->Checked=false;', 'J.SetChecked("chkOffCenterkit", false);'),
    ('ScrollBar1Change', 'chkOffCenterkit         ->Visible=false;', 'J.SetVisible("chkOffCenterkit", false);'),
    ('ScrollBar1Change', 'palVisibleIndex         ->Visible=false;', 'J.SetVisible("palVisibleIndex", false);'),
    ('ScrollBar1Change', 'cbPreventDropfunction   ->Visible=false;', 'J.SetVisible("cbPreventDropfunction", false);'),
    ('ScrollBar1Change', 'YPitch ->Visible=false;', 'J.SetVisible("YPitch", false);'),
    ('ScrollBar1Change', 'XPitch ->Visible=false;', 'J.SetVisible("XPitch", false);'),
    ('ScrollBar1Change', 'XPitch ->Text   ="1";', 'J.SetText("XPitch", AnsiString("1"));'),
    ('ScrollBar1Change', 'YPitch ->Text   ="0";', 'J.SetText("YPitch", AnsiString("0"));'),
    ('ScrollBar1Change', 'cbSingleUseOtherSuck    ->Visible=true;', 'J.SetVisible("cbSingleUseOtherSuck", true);'),
    ('ScrollBar1Change', 'cbPreventDropfunction   ->Visible=true;', 'J.SetVisible("cbPreventDropfunction", true);'),
    ('ScrollBar1Change', 'YPitch ->Visible=true;', 'J.SetVisible("YPitch", true);'),
    ('ScrollBar1Change', 'XPitch ->Text   ="0";', 'J.SetText("XPitch", AnsiString("0"));'),
    ('ScrollBar1Change', 'YPitch ->Text   ="";', 'J.SetText("YPitch", AnsiString(""));'),
    ('ScrollBar1Change', 'labYOffset   ->Visible=true;', 'J.SetVisible("labYOffset", true);'),
    ('ScrollBar1Change', 'edYOffset    ->Visible=true;', 'J.SetVisible("edYOffset", true);'),
    ('ScrollBar1Change', 'labYOffset   ->Visible=false;', 'J.SetVisible("labYOffset", false);'),
    ('ScrollBar1Change', 'edYOffset    ->Visible=false;', 'J.SetVisible("edYOffset", false);'),
    ('ScrollBar1Change', 'cb16change8DirectHeater ->Visible=true;', 'J.SetVisible("cb16change8DirectHeater", true);'),
]
# ---- AUTO(ws-arrow/multiline) END

FORM = {
    'class': 'TfSetup',
    'cpp': 'cSetUp.cpp',
    'h': 'cSetUp.h',
    'page': 'Setup.SetUp.html',
    'struct': 'TestIF_File',
    'files': ['HandlerCondition.Data', 'Contact.Data（Test Arm1/Arm2 Contact、Suck Shuttle Device After Tested）',
              'Temperature.Data（[ATC] ATC7CH1..4Enabled）',
              'config.ini [RTC] Enable（IniConfig.bRTCbySystem 時；golden 寫死 D:/HT9045/config/config.ini）'],
    'also': ['Temperature'],
    # CHSetError／CheckShuttlePitch 在 golden 回 bool；產生器一律產 void → 用 J.M("ret_<方法>") 帶回傳值（見 overrides）。
    'methods': ['FormShow', 'DoIniDataToForm', 'ScrollBar1Change', 'CompChange', 'chkOffCenterkitClick',
                'rgYPitchOffsetModeClick', 'sbUpdateClick', 'CHSetError', 'CheckShuttlePitch', 'SaveSetupFile'],
    # 表單成員（cSetUp.h）＋ golden cSetUp.cpp 檔案範圍、只有這個表單用的全域（OrgTestMode :121、iASMTestMode :55）。
    'members': ['fShow', 'bNeedPassword', 'bOCRNeedPassword', 'bSavePressed', 'iTestModeOcr',
                'bSiteMapHasChange', 'iTestMode', 'OrgTestMode', 'iASMTestMode'],
    # golden 建構子 :123-216 的 widget 部分（非 widget 的 iTestSiteCh=-1、tSiteMap、iTestModeOcr=-1 是程式啟動時一次，
    # 移植樹 TfSetup::Init() 已做，GET 不重做）。
    'display': [
        '// ---- golden 建構子 :141-165：列/欄標籤與 32 個 site 下拉先全部隱藏',
        'for(int i=0; i<MAX_SOCKET_TOTAL; i++)\n'
        '    {\n'
        '        int iRow=i/MAX_SOCKET_COL, iCol=i%MAX_SOCKET_COL;\n'
        '        if(i<MAX_SOCKET_ROW) { std::string r("labRow"); r+=char(\'A\'+i); J.SetVisible(r.c_str(), false); }\n'
        '        if(i<MAX_SOCKET_COL) { std::string c("labCol"); c+=char(\'A\'+i); J.SetVisible(c.c_str(), false); }\n'
        '        std::string s("cb"); s+=char(\'A\'+iRow); s+=char(\'a\'+iCol); J.SetVisible(s.c_str(), false);\n'
        '    }',
        'J.M("fShow")=false;   // golden 建構子 :166',
        'J.Todo("golden ctor :168-183 ScrollBar1->Max (per MachineTypeChoice) not transported -- FormState has no Max");',
        '// ---- golden 建構子 :194-199：Y-Pitch 36mm 機構換選項',
        'if(USE_IN_OUT_ARM_Y_PITCH==iXPitchManual360)\n'
        '    {\n'
        '        J.ItemsClear("rgInOutArmYPitch");\n'
        '        J.ItemsAdd("rgInOutArmYPitch", "60.0 mm");\n'
        '        J.ItemsAdd("rgInOutArmYPitch", "36.0 mm");\n'
        '    }',
        '// ---- golden 建構子 :201-215：動態產生 iSnSocketCnt 個 TRadioGroup，Name="rgSensor"+(i+1)（頁面元件 id 同名）',
        'for(int i=0; i<iSnSocketCnt; i++)\n'
        '    {\n'
        '        AnsiString n=AnsiString("rgSensor")+AnsiString(i+1), Str;\n'
        '        Str.sprintf("Sensor %d usage", i+1);\n'
        '        J.SetCaption(n.c_str(), Str);\n'
        '        J.ItemsClear(n.c_str());\n'
        '        J.ItemsAdd(n.c_str(), "No use");\n'
        '        J.ItemsAdd(n.c_str(), "Has IC");\n'
        '        J.ItemsAdd(n.c_str(), "Floating");\n'
        '    }',
        'B_FormShow(J);',
    ],
    # 'save' 不設：golden SaveSetupFile(AnsiString sPath) 一個參數，產生器的 Save 包裝寫死三個參數（見檔頭 2.）。
    'saveFlow': 'B_sbUpdateClick',
    # 讀檔器 TfSetup::ReadFile 在移植樹 live（cSetUp.cpp:416）；GetTestMode／SetShtMode／VertifyShtModeisDiff 是移植樹已翻、
    # 不碰 widget 的方法（forms/fSetup.h:515/:522），shtMode 狀態放在移植樹 fSetup 實例上（golden 也是表單成員）。
    'port_calls': {'ReadFile': 'fSetup->ReadFile', 'GetTestMode': 'fSetup->GetTestMode',
                   'SetShtMode': 'fSetup->SetShtMode', 'VertifyShtModeisDiff': 'fSetup->VertifyShtModeisDiff'},
    'sourceGap': 'partial: TfSetup::ReadFile (cSetUp.cpp:416) is live and fills 78 of the 83 TestIF_File fields this page shows, but '
                 '(1) TestIF_File.bForEgisTecTest (gbShuttleMode visibility) is only filled by TFTestIF::ReadTestIFFile, still GATE F-5; '
                 '(2) bDualSiteUseOneSuck / bPreventDropfunction are golden-912 fields missing from the port SYSTEM_TEST_IF '
                 '(cbbDualSiteUseOneSuck / cbPreventDropfunction show false); '
                 '(3) save path not wired: golden SaveSetupFile(sPath) has 1 param (generator Save wrapper needs 3) and the save flow '
                 'reads open-time form state (bNeedPassword, iASMSiteMap, iTestSiteCh) that a fresh form.save FormState does not have',
    'includes': ['cprod.h', 'Config.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'cAuthority.h', 'forms/fSetup.h', 'forms/fMain.h', 'forms/fSecurity.h',
                 'LastSet.h', 'csystem.h', 'BarcodeReader.h', 'SECSGEM/SecsEventType.h',
                 'SECSGEM/SecsEventReport.h', 'cinitial.h', 'ainarm9045.h', 'aHotPlateSubstrate.h', 'atester_shims.h', 'mysensor.h',
                 'Motor/mymotor.h', 'cMyDB.h', 'ATC/ATCInterface.h', '<string>'],
    'blocks': BLOCKS + AUTO_BLOCKS,
    'overrides': AUTO_OVERRIDES + OVERRIDES,
}
