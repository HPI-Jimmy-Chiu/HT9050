# -*- coding: utf-8 -*-
# tools/editlist/BinSelect.py -- gen_editlist.py 的結構設定（C 形狀，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only BinSelect
#
# Steven 20260925：結構 BinSelect（BinSelect[eBinTypeTotal] ← <recipe>\Binasgn*.Data，7 種 bin 類型）——
# golden TfBinSel（cBinSel.cpp，6754 行）。取代 A 形狀 tools/formbridge/TfBinSel.py（整合者退役）。
#
# Steven 指定的做法（20260925）：TfBinSel 的資料不在表單元件上，而在**結構**裡 ——
#   * vector<TMyBinPanel*> MyBinPanel（golden cBinSel.cpp:974-980 建 7 個）的整數欄位；
#   * 每種 bin 類型一組 TStringList sXxx[tag]（golden ctor :986-1078 new 並填初值）。
# 讀 golden ReadFile(:1121) → ReadFunctionData(tag)(:4855) → mtTrayNameSetColor(tag) → InitDataToEdit(tag)(:4190)；
# 存 golden spbSaveClick(:2252) → SaveOther(:2378) → SaveFunctionData(tag)(:5716)。
# 所以頁面不模擬 TMyBinPanel 的 GUI edit：JSON 直接交換「7 組 × 各 TStringList ＋ MyBinPanel[tag] 的資料欄位」，
# 由手寫入口 FileRW/BinSelect_C.cpp（FileRW_BinSelect_Page／FileRW_BinSelect_Save）處理；這裡只轉 golden 表單方法。
#
# 物件共用（Steven：優先共用 fBinSel 的清單與 MyBinPanel，不要另建一份）：
#   * sXxx[tag]／MyBinPanel[tag] → 移植樹 fBinSel 的那一份（#define，下面 members）。開機讀檔（wb_serve main()
#     的 fBinSel->ReadFile）、移植樹其他程式（Command.cpp、SECS、fShowBinSelect…）讀的都是這一份。
#   * 表單層的真元件（cbTestMode、CancelErrorBin、rg_FixBinBox、ed_FixBinBoxAlarmCount、chkShow0Xbin、cbUseMRTMode、
#     cbbAutoSiteMap、cbbASMPassBin、cbOutSht…、cbIndexDrop…、PageControl1、labWarning、Label1、spbSave、palSpecificBin）
#     在 BinSelect_C.cpp 開機時「收養」移植樹 fBinSel 的同名同型別物件（ELKeep）——產生器的 'adopt' 認不得
#     forms/fBinSel.h 的 `new TLabel()`（它找 `new vclcompat::T…`），所以收養寫在入口 cpp。
#   * golden 表單方法裡呼叫、但這裡不轉的 golden 方法（ReadFile、ReadParam、CheckFix2Tray、CheckOSBin、ARTBinCheck、
#     TransferBinTrayStrToName）→ 移植樹 fBinSel 的同名方法（#define；讀檔的寫入在移植樹是 GATE，見報告）。
#
# FormSysTools：golden TFormSysTools（systools.cpp:17-199）移植樹沒有 → decls 裡 TU 私用的同名類別，多載集合照
# golden systools.h（bool／int／unsigned int／double／AnsiString／TDateTime），轉給帶檔名的全域 WriteIniData（common.cpp）。
# golden TIniFile 是逐鍵直寫、不快取（OpenFormData＝new TIniFile、CloseFormData＝delete），所以語意相同。
# 多載照 golden：`WriteIniData(G, "Tray", "NotUse")` 的 const char* → bool（標準轉換）勝過 AnsiString（使用者定義轉換），
# golden 寫的是 WriteBool → "1"（照翻，不修）。
STRUCT = {
    'struct': 'BinSelect',
    'prefix': 'BS',
    'class': 'TfBinSel',
    'cpp': 'cBinSel.cpp',
    'h': 'cBinSel.h',
    'files': ['Binasgn.Data', 'BinasgnOff.Data', 'BinasgnOff-Line.Data', 'Binasgn_ART.Data', 'BinasgnOff_ART.Data',
              'Binasgn_MRT.Data', 'Binasgn_MRT_RT.Data', 'HandlerCondition.Data'],
    'lists': [],
    'methods': ['TfBinSel', 'FormShow', 'ShowChangeBinMessage', 'cbTestModeChange', 'ReadPrimeDara', 'SetPrimeButton',
                'spbSaveClick', 'SaveOther', 'SaveFunctionData', 'ReadWriteMRTMode', 'ReadWriteSpecialFunction',
                'btnSettingSpecificBinClick',
                # AI(W906-FRW-S99) 20260926: RULINGS_20260926 S99 —— Normal／Prime 兩顆鈕（golden :2800／:2822）與它們呼叫的
                #   WritePrimeDara（:6107，寫 <配方>\Binasgn.Data [BinModel] bPrime）。入口 FileRW/BinSelect.cpp FileRW_BinSelect_PrimeClick
                #   （editlist.save tag=BinSelect 帶 "op"）。不列進 save_methods：它們不是存檔鈕的流程，列進去會把它們讀的替身加進
                #   mustSend（kBS_SaveReads），改變原本存檔鈕的必送清單。
                'spbNormalClick', 'spbPrimeClick', 'WritePrimeDara'],
    'save_methods': ['spbSaveClick', 'SaveOther', 'SaveFunctionData', 'ReadWriteMRTMode', 'ReadWriteSpecialFunction'],
    'params': {'TfBinSel': '', 'FormShow': '', 'cbTestModeChange': '', 'spbSaveClick': '',
               'btnSettingSpecificBinClick': '',
               'spbNormalClick': '', 'spbPrimeClick': ''},   # AI(W906-FRW-S99) 20260926: 事件處理器的 TObject *Sender 拿掉
    'members': [
        # golden cBinSel.h 的非元件成員（本 TU 私用）。bShow／bSaveBin 不接移植樹 fBinSel：網頁開頁不是表單開著
        # （golden FormClose 才設回 false），接過去會讓移植樹以為 Bin 表單一直開著。
        'bool bShow=false;             // golden cBinSel.h:231 TfBinSel::bShow',
        'bool bSaveBin=false;          // golden cBinSel.h:180',
        'bool bCheckOffLineLevel=false; // golden cBinSel.h:181',
        'int  iMode=0;                 // golden cBinSel.h:182（只有 FormShow 用；ReadWriteMRTMode 的參數同名、遮蔽它，與 golden 相同）',
        'bool bCountSetPassed=false;   // golden cBinSel.h:207',
        # golden MyBinPanel[tag]->Panel->Enabled（每個 tag 的 Bin 面板 TPanel，ctor 動態建的，沒有頁面 id）。
        # 移植樹 TMyBinPanelData 沒有 Panel → 本 TU 記一份；FileRW_BinSelect_Save 用它判斷這個 tag 的 bin 資料能不能改。
        'bool bBinPanelEnabled[eBinTypeTotal]={};   // golden MyBinPanel[tag]->Panel->Enabled（FormShow :1726-1779）',
        '#define MyBinPanel (fBinSel->MyBinPanel)                                   // golden cBinSel.h（vector<TMyBinPanel*>）→ 移植樹 TMyBinPanelData*[7]',
        '#define sBinDoubleContact (fBinSel->sBinDoubleContact)                     // golden cBinSel.h:235',
        '#define sBinConsFail (fBinSel->sBinConsFail)',
        '#define sBinEnableFail (fBinSel->sBinEnableFail)',
        '#define sBinFailPercent (fBinSel->sBinFailPercent)',
        '#define sBinFailIgnore (fBinSel->sBinFailIgnore)',
        '#define sBinCountEnable (fBinSel->sBinCountEnable)',
        '#define sBinCountIgnore (fBinSel->sBinCountIgnore)',
        '#define sBinCountNumber (fBinSel->sBinCountNumber)',
        '#define sSpecialBinByArm (fBinSel->sSpecialBinByArm)',
        '#define sSpecialBinCountByArm (fBinSel->sSpecialBinCountByArm)',
        '#define sSpecialBinBySocket (fBinSel->sSpecialBinBySocket)',
        '#define sSpecialBinCountBySocket (fBinSel->sSpecialBinCountBySocket)',
        '#define sLowYield (fBinSel->sLowYield)',
        '#define sArmYield (fBinSel->sArmYield)',
        '#define sSiteYield (fBinSel->sSiteYield)',
        '#define sBinTraySetT3Pos (fBinSel->sBinTraySetT3Pos)',
        '#define sBinTraySetT3PosName (fBinSel->sBinTraySetT3PosName)',
        '#define sBinType (fBinSel->sBinType)',
        '#define sBySiteClean (fBinSel->sBySiteClean)',
        '#define sByBinClean (fBinSel->sByBinClean)',
        '#define sT3TrayType (fBinSel->sT3TrayType)',
        '#define sT6Retest (fBinSel->sT6Retest)',
        '#define sT3CateR (fBinSel->sT3CateR)',
        '#define sSpecBinBySiteCompareEnable (fBinSel->sSpecBinBySiteCompareEnable)',
        '#define sSpecBinBySiteCompareIgnore (fBinSel->sSpecBinBySiteCompareIgnore)',
        '#define sSpecBinBySiteComparePercent (fBinSel->sSpecBinBySiteComparePercent)',
        '#define sSpecBinByArmPerSiteCompareEnable (fBinSel->sSpecBinByArmPerSiteCompareEnable)',
        '#define sSpecBinByArmPerSiteCompareIgnore (fBinSel->sSpecBinByArmPerSiteCompareIgnore)',
        '#define sSpecBinByArmPerSiteComparePercent (fBinSel->sSpecBinByArmPerSiteComparePercent)',
        '#define sAOIBinTraySetting (fBinSel->sAOIBinTraySetting)',
        '#define sBinTrayLinked (fBinSel->sBinTrayLinked)',
        '#define sBinLinked (fBinSel->sBinLinked)',
        '#define sMagazineSetup (fBinSel->sMagazineSetup)',
        # 不轉的 golden 方法 → 移植樹 fBinSel（同一份資料）
        '#define ReadFile(a, b, c) fBinSel->ReadFile(a, b, c)     // golden :1121 → 移植樹 cBinSel.cpp:1411（live；寫入在移植樹是 GATE）',
        '#define ReadParam() fBinSel->ReadParam()                 // golden :2128',
        '#define CheckFix2Tray() fBinSel->CheckFix2Tray()         // golden :4833',
        '#define CheckOSBin() fBinSel->CheckOSBin()               // golden :6302',
        '#define ARTBinCheck(t) fBinSel->ARTBinCheck(t)           // golden :6429',
        '#define TransferBinTrayStrToName(t) fBinSel->TransferBinTrayStrToName(t)   // golden :6452',
    ],
    'replace': [
        # ================= ctor（golden :971）：只留 cbTestMode 的選項（:1080-1098）=================
        ('TfBinSel', 974, 1078,
         '7 個 TMyBinPanel（GUI）與 30＋組 sXxx TStringList 的 new／初值：移植樹 fBinSel 的建構子已建（forms/fBinSel.h，'
         'MyBinPanel＝TMyBinPanelData）——共用那一份，不另建（Steven 20260925）',
         ';'),
        ('TfBinSel', 1100, 1118,
         'MyBinPanel[tag]->mtBinSelect（TTMyTray 格子）寬度／XItem、KYEC 面板底色：純畫面',
         ';'),
        # ================= FormShow（golden :1700）=================
        ('FormShow', 1711, 1718, 'MyBinPanel[tag]->mtBinSelect／sbBinSetScroll 初始化（TTMyTray 格子與捲軸）：純畫面', ';'),
        ('FormShow', 1724, 1725, 'Caption（視窗標題 Contact Parameter ...）：純畫面', ';'),
        ('FormShow', 1732, 1732, 'MyBinPanel[tag]->Panel（每個 tag 的 Bin 面板）沒有頁面 id → bBinPanelEnabled[tag]',
         'bBinPanelEnabled[tag]=fSecurity->Insufficient(20, false);'),
        ('FormShow', 1739, 1739, 'OutArmSuck（aHotPlateSubstrate.h）與 HTEditList.h 同 TU 衝突 → FileRW/_KitSuck.cpp 轉接',
         'if(FileRW_OutArmSuckHasIC()==false && ShuttleHasIC()==false && IndexHasIC()==false)'),
        ('FormShow', 1742, 1742, 'MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]',
         'bBinPanelEnabled[tag]=fSecurity->Insufficient(20, false);'),
        ('FormShow', 1747, 1747, 'MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]', 'bBinPanelEnabled[tag]=false;'),
        ('FormShow', 1753, 1753, 'MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]', 'bBinPanelEnabled[tag]=false;'),
        ('FormShow', 1759, 1759,
         'HSys.BinDisCtrl->ProcessStopStart(false)：Bin 顯示 TFT（NUMBER_PANEL_TYPE 3/4）的硬體動作；移植樹 BinDisCtrl 是不透明 NULL（database.h:303）',
         'filerw::ELTodo("golden cBinSel.cpp:1759 HSys.BinDisCtrl->ProcessStopStart(false) (bin display TFT, NUMBER_PANEL_TYPE 3/4) not done: port BinDisCtrl is opaque/NULL (database.h:303)");'),
        ('FormShow', 1767, 1767, 'MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]', 'bBinPanelEnabled[tag]=false;'),
        ('FormShow', 1777, 1778, 'MyBinPanel[tag]->Panel → bBinPanelEnabled[tag]（乘上 authBinSetting[tag]）',
         'if(bBinPanelEnabled[tag]==true) bBinPanelEnabled[tag]=authBinSetting[tag];'),
        ('FormShow', 1781, 1801, 'MyBinPanel[tag]->mtTrayItem 格子的 tray 名稱與底色（Prod.iTrayType／s6TrayName）：純畫面（頁面有 s6TrayName）', ';'),
        ('FormShow', 1879, 1879, 'fBinSel->Height=920：視窗屬性', ';'),
        ('FormShow', 1882, 1882, 'cbTestModeChange(this)：本 TU 的轉換版沒有 Sender 參數', 'BS_cbTestModeChange();'),
        ('FormShow', 1884, 1885, 'Left／Top：視窗位置', ';'),
        ('FormShow', 1919, 1919, 'sgSpecificBin（TStringGrid）→ filerw::ELStringGrid：ColCount 在它的 grid 成員上',
         'EL<filerw::ELStringGrid>("TfBinSel", "sgSpecificBin")->grid.ColCount =iTestBinCount;'),
        ('FormShow', 1955, 2009,
         'CC_Greatek Control Bin 疊圖：edControlBinCheckPoint 位置／Parent、mtBinSelectBy（TTMyTray）外觀與格子'
         '（fProductionInfo->iControlBin[]）、edControlBinCheckPoint->Text（移植樹 TfProductionInfo 沒有 iControlBinCheckPoint）',
         'filerw::ELTodo("golden cBinSel.cpp:1955-2009 CC_Greatek Control Bin overlay (mtBinSelectBy TTMyTray cells, edControlBinCheckPoint text from fProductionInfo->iControlBinCheckPoint) not ported");'),
        ('FormShow', 2021, 2021, 'FrmAOI->bCheckAOIFailBinUse()：移植樹 forms/fAOI.h 沒有這個方法 → 走 else（隱藏 spbAOIBin）',
         'filerw::ELTodo("golden cBinSel.cpp:2021 FrmAOI->bCheckAOIFailBinUse() has no body in the port (forms/fAOI.h) -- spbAOIBin hidden"); if(false)'),
        # ================= ShowChangeBinMessage（golden :1635）=================
        ('ShowChangeBinMessage', 1646, 1646, 'OutArmSuck → FileRW/_KitSuck.cpp 轉接', 'if(FileRW_OutArmSuckHasIC())'),
        # ================= cbTestModeChange（golden :2147）：ActivePage=tsX → ActivePageIndex（DFM 頁序 cBinSel.dfm:32-468，無 PageIndex）=================
        # 產生器會把 `->ActivePage=` 當純畫面換成空敘述；但 SaveOther 用 PageControl1->ActivePageIndex 選檔，所以要真的設。
        ('cbTestModeChange', 2161, 2161, 'ActivePage=tsRetest（頁序 1）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=1;'),
        ('cbTestModeChange', 2172, 2172, 'ActivePage=tsOffline（頁序 2）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=2;'),
        ('cbTestModeChange', 2183, 2183, 'ActivePage=tsArtRT（頁序 4）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=4;'),
        ('cbTestModeChange', 2194, 2194, 'ActivePage=tsArtFT（頁序 3）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=3;'),
        ('cbTestModeChange', 2205, 2205, 'ActivePage=tsNormal（頁序 0）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=0;'),
        ('cbTestModeChange', 2216, 2216, 'ActivePage=tsMrtRT（頁序 6）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=6;'),
        ('cbTestModeChange', 2227, 2227, 'ActivePage=tsMrtFT（頁序 5）', 'EL<TPageControl>("TfBinSel", "PageControl1")->ActivePageIndex=5;'),
        # ================= SetPrimeButton（golden :6058）=================
        # AI(W906-FRW-S99) 20260926（Steven 團隊，RULINGS S120 派工 C）：:6089 fMain->SetNormalOrPrime() 的取代規則拿掉 —— 照 golden 原樣呼叫。
        #   原因過期：Jimmy 的 OPMODE 波次（7304dcef）已翻好 TfMain::SetNormalOrPrime（forms/fMain.h:562 宣告，本體 forms/fMain_OperateMode.cpp，
        #   在 ht9045_sm）；本檔產生的 FileRW/BinSelect.gen.inc 只編進 wb_serve（FileRW/_editlist_sources.cmake），wb_serve 的連結群組有 ht9045_sm
        #   ⇒ 直接呼叫就連得到，不用 hook。它的本體只動 palPrime／palNormal（主畫面的兩個面板，網頁擁有，Jimmy 那邊 #if 0），所以現在呼叫的
        #   可觀察效果是「沒有」——跟 golden 一樣的呼叫點與順序（「必須在讀完 Bin 別設定之後」），之後主畫面面板接上就跟著生效。
        #   原規則：('SetPrimeButton', 6089, 6089, 'fMain->SetNormalOrPrime()：主畫面 Normal/Prime 顯示，移植樹 forms/fMain.h 沒有',
        #            'filerw::ELTodo("golden cBinSel.cpp:6089 fMain->SetNormalOrPrime() has no body in the port (forms/fMain.h)");'),
        # ================= ReadPrimeDara（golden :6092）=================
        ('ReadPrimeDara', 6097, 6097, 'AnsiString 經 varargs 傳給 sprintf（BCB AnsiString 只有一個指標成員才成立）→ .c_str()',
         'szDir.sprintf("%s%s\\\\Binasgn.Data", DataPath.c_str(), S.c_str());'),
        # ================= WritePrimeDara（golden :6107）=================   AI(W906-FRW-S99) 20260926
        ('WritePrimeDara', 6112, 6112, 'AnsiString 經 varargs 傳給 sprintf（同 ReadPrimeDara :6097）→ .c_str()',
         'szDir.sprintf("%s%s\\\\Binasgn.Data", DataPath.c_str(), S.c_str());'),
        # ================= spbSaveClick（golden :2252）=================
        ('spbSaveClick', 2258, 2258, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");'),
        # ================= SaveOther（golden :2378）=================
        # PageControl1->ActivePageIndex：PageControl1 是收養的移植樹物件；直接寫 fBinSel->PageControl1，
        # 不經 EL<>（經 EL<> 會被產生器當成「存檔讀的替身」→ mustSend，而 TPageControl 沒有可套用的值 → 存檔永遠 400）。
        ('SaveOther', 2386, 2386, 'PageControl1 → fBinSel->PageControl1（收養的同一個物件；見上）',
         'if(fBinSel->PageControl1->ActivePageIndex==1)'),
        ('SaveOther', 2412, 2412, '同上', 'else if(fBinSel->PageControl1->ActivePageIndex==2)'),
        ('SaveOther', 2417, 2417, '同上', 'else if(fBinSel->PageControl1->ActivePageIndex==3)'),
        ('SaveOther', 2422, 2422, '同上', 'else if(fBinSel->PageControl1->ActivePageIndex==4)'),
        ('SaveOther', 2427, 2427, '同上', 'else if(fBinSel->PageControl1->ActivePageIndex==5)'),
        ('SaveOther', 2432, 2432, '同上', 'else if(fBinSel->PageControl1->ActivePageIndex==6)'),
        ('SaveOther', 2504, 2505, 'AnsiString 經 varargs 傳給 sprintf → .c_str()',
         'str1.sprintf("%s Pass Bin Only Can Set One Bin.", s6TrayName[i].c_str()); '
         'str2.sprintf("%s Pass Bin 僅可設定一個Bin", s6TrayName[i].c_str());'),
        # ================= ReadWriteMRTMode（golden :6181）=================
        ('ReadWriteMRTMode', 6186, 6186, 'AnsiString 經 varargs 傳給 sprintf → .c_str()',
         'szDir.sprintf("%s%s\\\\HandlerCondition.Data", DataPath.c_str(), S.c_str());'),
        ('ReadWriteMRTMode', 6212, 6212, 'fMain->SetOpenBin()：主畫面 open-bin 顯示，移植樹 forms/fMain.h 沒有（只在讀的分支）',
         'filerw::ELTodo("golden cBinSel.cpp:6212 fMain->SetOpenBin() has no body in the port (forms/fMain.h)");'),
        # ================= ReadWriteSpecialFunction（golden :6235）=================
        ('ReadWriteSpecialFunction', 6242, 6242, 'AnsiString 經 varargs 傳給 sprintf → .c_str()',
         'szDir.sprintf("%s%s\\\\HandlerCondition.Data", DataPath.c_str(), S.c_str());'),
        # ================= btnSettingSpecificBinClick（golden :6166）=================
        # Parent=PageControl1 不是純畫面：DFM 的 palSpecificBin 在 tsNormal 底下，Re-Test 等分頁時 tsNormal 看不見，
        # 面板裡的 sgSpecificBin／cbOutSht…／cbIndexDrop… 就改不到；golden 把它移到 PageControl1 下才在每個分頁都能用。
        # 替身的權限樹（ELEditable）照 golden 改父元件；golden 之後沒有再移回去（表單存續期間都在 PageControl1 下）。
        ('btnSettingSpecificBinClick', 6171, 6171, 'palSpecificBin->Parent=PageControl1 → 替身權限樹改父元件（ELSetParents）',
         '{ static const char* const kReparent[][2] = {{"palSpecificBin", "PageControl1"}}; filerw::ELSetParents("TfBinSel", kReparent, 1); }'),
        ('btnSettingSpecificBinClick', 6172, 6173, 'palSpecificBin 的 BringToFront／Top：純畫面（Visible 照 golden 切換）', ';'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fBinSel.h',
                 'forms/fShowBinSelect.h', 'forms/fTrayAssignment.h', 'forms/fLotInfo.h',
                 'ProductionInfo/FileInfo.h', 'csystem.h', 'Motor/mymotor.h', 'canary_support.h'],
    # cAuthority.h（language.h → TWinControl 與 Public/HTEdit.h 重複）、aHotPlateSubstrate.h（TList／uPlateInfo 與
    # HTEditList.h 衝突）不能 include → 只宣告要用的
    'decls': [
        'extern bool authBinSetting[8];         // cAuthority.h:63（golden cAuthority.h）',
        'extern bool bAuthCriticalPara[26];     // cAuthority.h:64',
        'void GetBinSettingAuth();              // cAuthority.h:81',
        'bool FileRW_OutArmSuckHasIC();         // FileRW/_KitSuck.cpp（golden OutArmSuck.HasIC()）',
        '// golden cBinSel.cpp:53-86 的 TU 內 enum eBinSettingItems（只用到這三個；eBinNotUse=25，移植樹 cBinSel.cpp:111 同值）',
        'enum { eBinNotUse=25, eBinSetting=26, eBinSetTotal=(26+eTrayCount) };',
        '// golden systools.h TFormSysTools：TIniFile 逐鍵直寫。移植樹沒有 → 同名 TU 私用類別，多載集合照 golden（systools.h:21-35），',
        '// 轉給帶檔名的全域 WriteIniData（common.cpp:1044-1265；golden 的 change log 在移植樹全域 WriteIniData 裡是 GATE）。',
        'namespace {',
        'struct BS_TFormSysTools {',
        '    AnsiString FileName;',
        '    void OpenFormData(AnsiString Filename) { FileName=Filename; }                 // golden systools.cpp:17 new TIniFile',
        '    void CloseFormData() { FileName=""; }                                           // golden systools.cpp:22 delete',
        '    void WriteIniData(AnsiString G, AnsiString N, bool v)         { ::WriteIniData(FileName, G, N, v); }',
        '    void WriteIniData(AnsiString G, AnsiString N, int v)          { ::WriteIniData(FileName, G, N, v); }',
        '    void WriteIniData(AnsiString G, AnsiString N, unsigned int v) { ::WriteIniData(FileName, G, N, (int)v); }   // golden :119 WriteInteger(Value)',
        '    void WriteIniData(AnsiString G, AnsiString N, double v)       { ::WriteIniData(FileName, G, N, v); }',
        '    void WriteIniData(AnsiString G, AnsiString N, AnsiString v)   { ::WriteIniData(FileName, G, N, v); }',
        '    void WriteIniData(AnsiString G, AnsiString N, TDateTime v)    { ::WriteIniData(FileName, G, N, v); }',
        '};',
        'BS_TFormSysTools BS_FormSysToolsObj;',
        'BS_TFormSysTools* const FormSysTools=&BS_FormSysToolsObj;',
        '}  // namespace',
    ],
    'blocks': [],
}
