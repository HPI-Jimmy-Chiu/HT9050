# -*- coding: utf-8 -*-
# tools/editlist/IniConfig.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only IniConfig]

# //AI(W906-FRW-S158) 20260927 [W906]：WS form.event（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 盤點
#   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\Q41_INVENTORY_20260927.md 3.19 節 CC-E2／CC-E7）用到的 golden 行用 _expect 釘住原文
#   （golden 那一行變了產生器就停，不會蓋錯行；同 tools/editlist/TrayForm.py）。
_GOLDEN_CPP = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cConfiguration.cpp'
_GL = open(_GOLDEN_CPP, 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _expect(ln, text):
    if text not in _GL[ln - 1]:
        raise SystemExit('IniConfig.py: golden cConfiguration.cpp:%d 不再是 %r（實際 %r）' % (ln, text, _GL[ln - 1].strip()))
    return ln


STRUCT = {
    'struct': 'IniConfig',
    'class': 'TfConfiguration',
    'cpp': 'cConfiguration.cpp',
    'h': 'cConfiguration.h',
    'files': ['config.ini', 'LastSet.ini', 'configByRecipe.ini'],
    'lists': ['elConfig', 'cbLastSet', 'elConfig_byRecipe'],
    # 讀方向：golden TfConfiguration 建構子（cConfiguration.cpp:112-115）呼叫的那幾支 ＋ 它們呼叫的
    'methods': ['ReadLockByFile', 'InitConfigEdtList_ItemA', 'InitConfigEdtList_ItemB', 'InitConfigEdtList_ItemC',
                'InitConfigEdtList_ItemD', 'InitConfigEdtList_ItemE', 'InitConfigEdtList_ItemF',
                'InitConfigEdtList_ItemG', 'InitConfigEdtList_ItemI', 'InitConfigEdtList_ItemL',
                'InitConfigEdtList_ItemM', 'InitConfigEdtList_ItemN', 'InitConfigEdtList_ItemO',
                'InitConfigEdtList_ItemP', 'InitConfigEdtList', 'ChangeCBListProperty',
                # 寫方向：golden 關表單就存檔 —— FormClose（cConfiguration.cpp:5816）→ CheckConfigurationBeforeSave
                # → SaveConfiguration（WriteLastDataFile／SaveLastSetIni／SaveTasterInfo／SetA73）→ LoadConfiguration
                'FormClose', 'CheckConfigurationBeforeSave', 'SaveConfiguration', 'LoadConfiguration', 'SetA73',
                'EnableRMSFunc', 'WriteContactData',
                # 顯示端：golden 開頁 FormShow（:4614）＝「C++ 發 JSON 給 HTML」—— GET /api/editlist 時跑
                'FormShow',
                # 審查第 9 輪 M-2：golden DFM OnChange（EP 保護：<85→85、>115→115，記 aEP*OldData／bEP*DataChange 供 FormClose 寫變更 log）；
                # FormShow :4698-4713 設 Position 就會觸發（filerw::ELTrackBar）
                'tbD25_Index30mmChange', 'tbD25_Index30mm_NSChange', 'tbD25_Index40mmChange', 'tbD25_Index40mm_NSChange', 'tbD25_Index60mmChange', 'tbD25_Index60mm_NSChange', 'tbD60_Index56mmChange', 'tbD60_Index56mm_NSChange',
                # //AI(W906-FRW-Q19) 20260927: Steven 20260927 Q19（RULINGS_20260926 S140）「P26 OCR Tray Lot 可以補」——
                # golden :5522 edOCRTrayLotChange（建構子 :214 動態建的 10 格 OCRTrayLot0..9 的 OnClick：小鍵盤 → LastSet.TrayCount[Tag]
                # → WriteLastDataFile()）。FileRW/IniConfig.cpp 存檔時對頁面改過的格子重播（參數照 golden：TObject *Sender）
                'edOCRTrayLotChange',
                # //AI(W906-FRW-S158) 20260927 [W906]：WS form.event 的 golden 處理器（Q41 盤點 CC-E2／CC-E7，見下面 'events'）——
                #   :6094 udD46Click（D46 上下鍵）、:6125 cbE30Click、:6369 cbE39Click、:6551 cbD36Click、:6025 Timer1Timer（只留顯示段，見 replace）。
                #   FileRW/IniConfig.cpp 檔尾註冊（IniConfig 不是 PageDesc，另掛一個 form.event 專用的頁名 Config.Configuration）。
                'udD46Click', 'cbE30Click', 'cbE39Click', 'cbD36Click', 'Timer1Timer',
                # //AI(W906-EVB4) 20260928 [W906]：Steven 20260928 事件派工 B4（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md 第三節 B4）的 form.event 處理器 ——
                #   :6062 btD47Click（CC-E1）、:6691 btnRecordJamRateByTimeClearClick（CC-E3）、:6257 btResumeClick（CC-E4）、
                #   :6453 cbA09Click（CC-E6）。CC-E8 的 8 支滑桿處理器（tbD25_*／tbD60_* Change）上面已經在列，只在 'events' 加列。
                'btD47Click', 'btnRecordJamRateByTimeClearClick', 'btResumeClick', 'cbA09Click',
                # //AI(W906-EVB10B) 20260929 [W906] CC-L1（Q46：Steven 20260929「依權限檔鎖住是要執行的」＝照 BCB 翻切頁程式）＋X-5：
                #   :6101 PageControl1Change（外層 Soft／Comm／Config／Tray／Hot Plate，dfm:22 OnChange）、:6116 pcConfigChange（A～M 大分頁，
                #   dfm:452 OnChange）—— 依 D:\HT9045\config\Security_new.def（FormShow :4663 GetConfAuth 讀進 authConfig[]／authConf[]）把
                #   切到的那一頁整頁鎖住。加在最後：既有方法的產生碼順序不動。
                'PageControl1Change', 'pcConfigChange'],
    # //AI(W906-EVB10B) 20260929 [W906] X-3（R118＝照 BCB）：VCL 程式設 Checked 也觸發 DFM OnClick —— 產生器在 IC_DfmState 登記 cbE30…／cbE39／
    #   cbD36…／cbA09 群組的 OnClick（處理器有轉的那幾個），FileRW/IniConfig.cpp 開機裝上 HTEditList 的勾選 hook（Public/HTEditList.cpp
    #   HTEditList_SetClickHooks）⇒ golden Add／ReadEditTextFromFile／InitialDataToEdit（HTEditList.cpp:1390 那一類）設勾選時照 VCL 觸發；
    #   本檔轉出方法裡的 `cb->Checked=…` 也改寫（cbA09Click 自己改回 cbA09 那兩行＝VCL 會再進一次 cbA09Click，值相等就 return）。
    #   chkHeater（FormShow :4634／FormClose :5823 設 false，OnClick＝chkHeaterClick 切加熱繼電器）沒轉 ⇒ 不登記、不觸發（網頁沒有 Heater 手動測試，
    #   CC-E11；chkHeater 替身永遠是 false，設 false 不會有變化）。cbC12 群組／cbA27／cbN07_EnableEmployeeCheak／cbM01 群組的處理器（廠商密碼、
    #   DoPassword）沒轉 ⇒ 不觸發（Q45＝C：網頁不問密碼，IC_PasswordGuard 存檔拒）。
    'vcl_clicks': True,
    'rettype': {'EnableRMSFunc': 'bool'},
    # 存檔流程的方法：產生器掃出它們「讀」的替身，列成 kIC_SaveReads —— 不在任何 HTEditList 裡的替身
    # （值來自 golden FormShow，例 tbD25_*->Position、dtO06_LastDate->Date）頁面沒送就拒存。
    'save_methods': ['FormClose', 'CheckConfigurationBeforeSave', 'SaveConfiguration', 'LoadConfiguration',
                     'SetA73', 'WriteContactData'],
    # golden 方法參數換掉（伺服器端沒有 Sender／CloseAction；FormClose 本體沒用到它們）
    'params': {'FormClose': '', 'FormShow': '', 'tbD25_Index30mmChange': '', 'tbD25_Index30mm_NSChange': '', 'tbD25_Index40mmChange': '', 'tbD25_Index40mm_NSChange': '', 'tbD25_Index60mmChange': '', 'tbD25_Index60mm_NSChange': '', 'tbD60_Index56mmChange': '', 'tbD60_Index56mm_NSChange': '',
               # //AI(W906-FRW-S158) 20260927 [W906]：form.event 處理器的本體都沒讀 Sender（udD46Click 也沒讀 TUDBtnType Button：
               #   VCL TUpDown 在 OnClick 之前就已經把 Position 加減好，:6097-6098 只讀 Position）→ ''（產生器的事件表只收 '' 或 TObject *Sender）
               'udD46Click': '', 'cbE30Click': '', 'cbE39Click': '', 'cbD36Click': '', 'Timer1Timer': '',
               # //AI(W906-EVB4) 20260928 [W906]：B4 四支的本體都沒讀 Sender（cbA09Click 綁在 cbA09／chkA09_1／cbA14 三格，但只看、只改 cbA09）
               'btD47Click': '', 'btnRecordJamRateByTimeClearClick': '', 'btResumeClick': '', 'cbA09Click': '',
               'PageControl1Change': '', 'pcConfigChange': ''},   # //AI(W906-EVB10B) 20260929 [W906]：CC-L1 兩支都不讀 Sender
    # golden FormShow :4619-4623 for(i<pcConfig->PageCount) if(Pages[i]!=tsSearchFunction) ChangeCompomentEnabled(…)
    'enable_all_root': 'pcConfig',
    'enable_all_skip': ['tsSearchFunction'],
    # 以指定程式碼取代的 golden 段落（方法, 起, 迄, 原因, 取代碼）
    'replace': [ ('InitConfigEdtList_ItemN', 3444, 3444, 'AI(W906-FRW-W44-1) 20260928 ★W44-1＝A（Steven 0928）：N10-3-1 指定時間接上——golden dtpN10_3_1_SpecifiedTime 不在任何 elConfig->Add（912 dfm:15549、h:2160），IniConfig.dN10_3_1_SpecifiedTime 從不讀存；照 golden HTEditList 的 TDateTimePicker 支以 ECDouble 綁，節＝FTPUpLoad、鍵＝欄位名（ELA ReadScheduleConfig 讀的就是這個）', 'elConfig->Add(EL<TRadioGroup>("TfConfiguration", "rgN10_3_1"), &IniConfig.iN10UploadProductMethod, ECInteger, "FTPUpLoad", "iN10UploadProductMethod", bShow, bEnable, bReadFromFile, 0);  elConfig->Add(EL<filerw::ELDateTimePicker>("TfConfiguration", "dtpN10_3_1_SpecifiedTime"), &IniConfig.dN10_3_1_SpecifiedTime, ECDouble, "FTPUpLoad", "dN10_3_1_SpecifiedTime", bShow, bEnable, bReadFromFile, 0);'),
        ('FormShow', 4954, 4978, 'ATC_InterfaceForm->asATC_SW_Ver：移植樹 ATC 門面沒有版本字串 → 等同 golden 字串為空，走 :4972-4977 else 支（審查 M-A）',
         'if(CUSTOMER_CODE==CC_ASE_KaohSiung) { IniConfig.bL43EnableATCPowerFollow=false; EL<TCheckBox>("TfConfiguration", "cbL43")->Checked=IniConfig.bL43EnableATCPowerFollow; EL<TCheckBox>("TfConfiguration", "cbL43")->Visible=false; }'),
        ('FormShow', 4619, 4623, 'pcConfig->Pages[i] 逐頁 ChangeCompomentEnabled(頁, true, true)：替身沒有頁的清單',
         'filerw::ELEnableNames("TfConfiguration", kIC_EnableAll, (int)(sizeof(kIC_EnableAll) / sizeof(kIC_EnableAll[0])));   // golden：頁與底下的容器／TLabel／TSpeedButton／TButton／TCheckBox／TRadioButton Enabled=true（審查 #2）'),
        # //AI(W906-FRW-Q19) 20260927: P26 OCR Tray Lot（Steven 20260927 Q19，RULINGS_20260926 S140）。以前是 blocks（#if 0）。
        #   golden edOCRTrayLot[10] 是建構子 :206-224 new 出來的陣列（header :2380，產生器的 widget 規則認不到陣列）→ 換成同名具名替身
        #   （golden Name＝"OCRTrayLot"+i；開機由 FileRW/IniConfig.cpp W906_IC_CreateOCRTrayLotProxies 照 :206-224 建、Parent＝gbP26_OCRCheck）。
        ('FormShow', 5290, 5294,
         '//AI(W906-FRW-Q19) 20260927: edOCRTrayLot[i]（golden 建構子 :208 new，Name＝"OCRTrayLot"+i）→ 具名替身 OCRTrayLot<i>；'
         'Text=LastSet.TrayCount[i]（int → AnsiString，同 golden 隱含轉換）、Tag=i',
         'for(int i=0; i<10; i++) { TEdit* p_=EL<TEdit>("TfConfiguration", (AnsiString("OCRTrayLot")+AnsiString(i)).c_str()); '
         'p_->Text=AnsiString(LastSet.TrayCount[i]); p_->Tag=i; }'),
        ('edOCRTrayLotChange', 5526, 5526,
         '//AI(W906-FRW-Q19) 20260927: fQwertyKey->ShowQwertyKey：螢幕小鍵盤在頁面（N_INTEGER 0～20，ht9045_iniconfig_p26_c.js）；'
         '這裡只做 golden 關鍵盤之後那一半（myQwertyKeyBoard.cpp:285-301：CheckRange 夾值後寫回 Text），見 IC_QwertyKey',
         'IC_QwertyKey((TEdit *)Sender, N_INTEGER, 0, true, 0, 20);'),
        # //AI(W906-FRW-S158) 20260927 [W906]：Timer1Timer（:6025，Timer1 DFM 預設 Interval 1000 ms，FormShow :5312 起動、FormClose :5931 停）
        #   只留 :6032-6044 的顯示段（勾 D21／D47／F05 才顯示底下的欄位＝Q41 盤點 CC-E7）。另外兩段不是「勾選連動顯示」：
        ('Timer1Timer', _expect(6030, 'UpdateUT150Comm();'), 6030,
         'UpdateUT150Comm()（:5591）：Heater 分頁的溫控器輪詢／送溫度（TMC401WriteTemp、UT100WordWriteNoSucm，走 COM）＝Q41 盤點 CC-E11「加熱器測試」'
         '（歸 Jimmy）；網頁事件裡不送硬體指令', ';'),
        ('Timer1Timer', _expect(6046, 'lblIPSCClear_Qty->Caption'), _expect(6049, 'lblIPSCOut_Qty->Caption'),
         'IPSC 數量標籤（fProductionInfo 的計數；Q41 盤點 CC-E13 的 IPSC 那一項，不是勾選連動顯示）：本 TU 不接 fProductionInfo', ';'),
        # //AI(W906-EVB4) 20260928 [W906]：CC-E6 cbA09Click（:6453-6480）機台內有料的判斷。InArmSuck 是 TMyKitSuck（aHotPlateSubstrate.h 與 HTEditList.h
        #   重複定義 TList，本 TU 不能 include）→ FileRW/_KitSuck.cpp 的 FileRW_ArmSuckHasIC(0)（＝golden InArmSuck.HasIC()，同 FileRW/MainClose.cpp:421）。
        #   其餘照 golden：InputShuttleHasIC／IndexHasIC（csystem.h）、MOT[MMPlate1/2].HasIC()（Motor/mymotor.h，見 decls）、ShowErrorMessage（canary_support.h）。
        ('cbA09Click', _expect(6460, 'if(InArmSuck.HasIC()'), 6460, 'AI(W906-EVB4) 20260928 InArmSuck.HasIC()：TMyKitSuck 本 TU 看不到 → FileRW/_KitSuck.cpp 轉接',
         'if(FileRW_ArmSuckHasIC(0) ||'),
        ('cbA09Click', _expect(6472, 'InArmSuck.HasIC()'), 6472, 'AI(W906-EVB4) 20260928 InArmSuck.HasIC()：同上',
         'FileRW_ArmSuckHasIC(0) ||'),
        # //AI(W906-EVB10B) 20260929 [W906] CC-L1／X-5：切分頁事件讀／設分頁。vclcompat TPageControl 只有 ActivePageIndex ⇒ `->ActivePage` 走
        #   filerw::ELActivePage（FileRW/_EditList.h；頁序由 FileRW/IniConfig.cpp 開機照 golden cConfiguration.dfm 登記 ELSetPageOrder）。
        ('FormShow', _expect(4645, 'PageControl1->ActivePage=tsConfig;'), 4645,
         'AI(W906-EVB10B) 20260929 PageControl1->ActivePage=tsConfig：產生器原本當純畫面丟掉（伺服器停在第 0 頁、頁面顯示 Config 頁）；'
         '外層的切頁事件 PageControl1Change 要看分頁 ⇒ 照 golden 切（tsConfig＝DFM 頁序第 2 頁）',
         '{ const int k_=filerw::ELPageIndexOf("TfConfiguration", "PageControl1", "tsConfig"); if(k_>=0) EL<TPageControl>("TfConfiguration", "PageControl1")->ActivePageIndex=k_; }'),
        ('PageControl1Change', _expect(6105, 'if(bPassWord(2)==false)'), 6105,
         'AI(W906-EVB10B) 20260929 bPassWord(2)（:6887，fPassword->ShowModal 問 ASE 高雄的 Config 頁密碼）：網頁沒有密碼框 ⇒ 照既有規則當「密碼錯」'
         '（filerw::ELPasswordRefused 回 false、訊息／待辦進 ack）⇒ golden 切回第 0 頁（Soft）；Q45 同一類',
         'if(filerw::ELPasswordRefused("TfConfiguration::bPassWord(2) -- CC_ASE_KaohSiung Config tab password (golden cConfiguration.cpp:6887)")==false)'),
        ('PageControl1Change', _expect(6112, 'if(pcConfig->ActivePage!=tsSearchFunction)'), 6112,
         'AI(W906-EVB10B) 20260929 pcConfig->ActivePage：filerw::ELActivePage（頁序 FileRW/IniConfig.cpp 開機登記）',
         'if(filerw::ELActivePage("TfConfiguration", "pcConfig")!=EL<TTabSheet>("TfConfiguration", "tsSearchFunction"))'),
        ('PageControl1Change', _expect(6113, 'ChangeCompomentEnabled(PageControl1->ActivePage, authConfig[PageControl1->ActivePageIndex]);'), 6113,
         'AI(W906-EVB10B) 20260929 PageControl1->ActivePage：filerw::ELActivePage；ChangeCompomentEnabled 的容器那一半（filerw::ELChangeCompomentEnabled，'
         'bMustEnable=false：只會鎖、不會開，同 golden；子元件由 ELEditable 看祖先）',
         'filerw::ELChangeCompomentEnabled(filerw::ELActivePage("TfConfiguration", "PageControl1"), authConfig[EL<TPageControl>("TfConfiguration", "PageControl1")->ActivePageIndex]);'),
        ('pcConfigChange', _expect(6120, 'if(pcConfig->ActivePage!=tsSearchFunction)'), 6120,
         'AI(W906-EVB10B) 20260929 pcConfig->ActivePage：同 :6112',
         'if(filerw::ELActivePage("TfConfiguration", "pcConfig")!=EL<TTabSheet>("TfConfiguration", "tsSearchFunction"))'),
        ('pcConfigChange', _expect(6121, 'ChangeCompomentEnabled(pcConfig->ActivePage, authConf[pcConfig->ActivePageIndex]);'), 6121,
         'AI(W906-EVB10B) 20260929 pcConfig->ActivePage：同 :6113（authConf[] 照 golden 以 pcConfig 的頁序位置當索引，funConf 13 項對 A～M 13 頁，'
         '第 13 頁 tsSearchFunction 被上一行排除）',
         'filerw::ELChangeCompomentEnabled(filerw::ELActivePage("TfConfiguration", "pcConfig"), authConf[EL<TPageControl>("TfConfiguration", "pcConfig")->ActivePageIndex]);'),
        # //AI(W906-D034) 20261002 (St01)：todo D-034 —— golden CheckConfigurationBeforeSave :7353-7369（[I37_1] FIFO 由關改開）的
        #   MyMessageBox->DoPassword_MBox()（mymessbox.cpp:1228-1286）。以前走產生器的通用規則（gen_editlist.py：一律 ELPasswordRefused＝密碼錯）；
        #   現在照 Q45 甲：答案＝editlist.save 的 reauth point "i37_1"（WebLogin.cpp 檔尾 W906_ReauthConfigI37／W906_DoPasswordMBox），
        #   沒帶答案才照舊 ELPasswordRefused（下面 members 的 IC_DoPasswordMBox）。
        ('CheckConfigurationBeforeSave', _expect(7356, 'if(MyMessageBox->DoPassword_MBox()==false)'), 7356,
         'AI(W906-D034) 20261002 MyMessageBox->DoPassword_MBox()（golden mymessbox.cpp:1228-1286）→ IC_DoPasswordMBox → WebLogin.cpp 檔尾 '
         'W906_ReauthConfigI37（editlist.save 的 reauth point i37_1；Q45 甲）；沒帶答案照舊 filerw::ELPasswordRefused（視同密碼錯，FIFO 不打開）',
         'if(IC_DoPasswordMBox()==false)'),
    ],
    # golden cConfiguration.cpp 檔案層級的全域（本表單私用），照抄成本 TU 的 static
    # TfConfiguration 的非元件成員（golden cConfiguration.h），本 TU 只有一個「表單」→ static
    'members': ['bool fShow=false;   // golden cConfiguration.h: TfConfiguration::fShow',
                # //AI(W906-FRW-Q19) 20260927: golden 小鍵盤 TfQwertyKey::ShowQwertyKey（myQwertyKeyBoard.cpp）ShowModal 之後那一半：
                #   :248-257 N_PORT 補範圍、:285-292 數字類 bCheckRange 時 Text=AnsiString(CheckRange(atof(Text), min, max))（:290 引數順序照抄，
                #   CheckRange 自己處理大小相反，MachineType.h:1653）、:298-299 寫回 TEdit。頁面值＝使用者在鍵盤按 Enter 時的內容。
                #   移植樹 fQwertyKey 只在 Public/HTEdit.cpp 懶建立、wb_serve 沒保證建立 → 不呼叫它（同 tools/editlist/TestIF_File_Cleaning.py 的 CL_QwertyKey）。
                'void IC_QwertyKey(TEdit* EditPtr, int iFunction, int iDP=0, bool bCheckRange=false, double min=0, double max=0) { (void)iDP; '
                'if(EditPtr==NULL) return; if(iFunction&N_PORT) { bCheckRange=true; if(min<0 || max<=0) { min=0; max=65535; } } '
                'if(iFunction&N_INTEGER || iFunction&N_DOUBLE) { double d=atof(EditPtr->Text.c_str()); if(bCheckRange) EditPtr->Text=AnsiString(CheckRange(d, min, max)); } }'
                '   // golden myQwertyKeyBoard.cpp:248-257／:285-301（AI(W906-FRW-Q19) 20260927）',
                # //AI(W906-D034) 20261002 (St01)：golden TMyMessageBox::DoPassword_MBox（mymessbox.cpp:1228-1286）給 CheckConfigurationBeforeSave :7356 用
                #   （上面 replace）。函式在這一行裡宣告（同 TestIF_File_SetUp.py 的 SU_DoPassword），產生檔的 include 不用多一行。
                'bool IC_DoPasswordMBox() { bool W906_ReauthConfigI37(bool*); bool bHandled=false; const bool bFlag=W906_ReauthConfigI37(&bHandled); '
                'return bHandled ? bFlag : filerw::ELPasswordRefused("DoPassword_MBox"); }   '
                '//AI(W906-D034) 20261002 golden mymessbox.cpp:1228-1286 DoPassword_MBox → WebLogin.cpp 檔尾 W906_ReauthConfigI37（W906_DoPasswordMBox）：'
                '權限表第 35 項；有密碼本 → cbUserSelectChange 密碼本分支、沒有 → stOperatorClick；AccessLevel<第 35 項且第 35 項>0 → false；'
                '有密碼本時問完一律登出成 Operator；CC_PTI 出貨組態是廠商密碼（Q45 #1～#3＝C，網頁版不提供）。答案＝這次 editlist.save 的 reauth point i37_1'
                '（取消＝空白帳密＝錯，Q45-4）；沒帶答案（bHandled=false）→ 照舊視同密碼錯'],
    'globals': ['aEP60OldData', 'aEP40OldData', 'aEP30OldData', 'aEP60OldData_NS', 'aEP40OldData_NS',
                'aEP30OldData_NS', 'bEP60DataChange', 'bEP40DataChange', 'bEP30DataChange', 'bEP60DataChange_NS',
                'bEP40DataChange_NS', 'bEP30DataChange_NS', 'old60data', 'old40data', 'old30data', 'old60data_NS',
                'old40data_NS', 'old30data_NS', 'bSave', 'bM01Enter', 'bStopChange'],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'atester_shims.h', 'mycylin.h',
                 'PowerSavingMode.h',
                 # 寫方向（FormClose／CheckConfigurationBeforeSave／SaveConfiguration／SetA73）用到的移植樹門面
                 'forms/fLotInfo.h', 'cinitial.h', 'forms/fSCKART.h', 'forms/fOffSet.h', 'csystem.h',
                 'forms/fSetup.h', 'canary_support.h', 'mymessbox_shim.h', 'forms/fTesterIF.h', 'forms/fPassword.h',
                 'forms/fCleaning.h', 'MessageDef.h', 'forms/fAOI.h', 'forms/fBinSel.h', 'forms/fShowBinSelect.h'],
    # cAuthority.h 會帶進 language.h，它的 TWinControl 與 Public/HTEdit.h 重複定義 → 只前置宣告要用的那一支
    'decls': ['AnsiString CheckFile(AnsiString szDir, AnsiString str);   // cAuthority.h:83（golden cAuthority.h）',
              'extern bool authConfig[5];    // cAuthority.cpp:122（golden cAuthority.h:11）',
              'extern bool authConf[14];     // cAuthority.h:60（golden cAuthority.h:12）',
              'void GetConfAuth();           // cAuthority.cpp:472（golden cAuthority.h:27）',
              'void SetTestRunMode();        // RunStartMode.cpp:938（golden cConfiguration.cpp:89 extern；Jimmy 0dcc9c2c 已翻）  AI(W906-FRW-S66) 20260926',
              'bool FileRW_ArmSuckHasIC(int which);   // FileRW/_KitSuck.cpp（0＝InArmSuck；golden cbA09Click :6460／:6472）  AI(W906-EVB4) 20260928',
              'extern bool bLockByServer;          // golden cConfiguration.cpp:6256（定義 golden ckernel.cpp:52；移植樹 Automation/automation.cpp:55）  AI(W906-EVB4) 20260928',
              # //AI(W906-EVB4) 20260928 [W906]：CC-E6 cbA09Click :6470-6471 MOT[MMPlate1/2].HasIC() 要 TTrayMotor（Motor/mymotor.h，同
              #   tools/editlist/BinSelect.py）。放 decls 不放 includes：這個 header 帶進 Motor/HTMotor.h:143-212 的空殼虛擬函式，-Wextra 會多 43 筆
              #   unused-parameter，只在這一個 include 前後關掉（語法檢查的警告基準維持 5 筆 gen.inc）。
              '#if defined(__GNUC__)',
              '#pragma GCC diagnostic push',
              '#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h 空殼虛擬函式（只包下一行的 include）',
              '#endif',
              '#include "Motor/mymotor.h"   // MOT[]（golden cbA09Click :6470-6471）  AI(W906-EVB4) 20260928',
              '#if defined(__GNUC__)',
              '#pragma GCC diagnostic pop',
              '#endif'],
    'overrides': [],
    # (方法, golden 起行, 迄行, 原因)：伺服器端還接不上的段落，原文包進 #if 0、執行時回報 todo
    'blocks': [
        # //AI(W906-FRW-Q19) 20260927: Steven 20260927 Q19（RULINGS_20260926 S140）「Soft 速度可以不需要使用」→ 不做，維持 #if 0。
        ('FormShow', 4648, 4672, 'edSoftSpeed[]／labSoftSpeed[]：動態產生的軟體速度欄（存檔走 edSoftSpeed0Change，另一條觸發）'
                                 '——Steven 20260927 Q19：不需要（不做）'),
        ('FormShow', 4884, 4892, 'imgI37_3->Picture->LoadFromFile：圖片（HTML 端依 tag 顯示 type%d.bmp）'),
        ('FormShow', 4895, 4899, 'rgI22->Controls[i]->Enabled：單一選項停用（替身沒有子選項）'),

        ('FormShow', 5242, 5242, 'FormHS->GetUploadServerByFTPPath：顯示用路徑（移植樹沒有 FormHS）'),
        # //AI(W906-FRW-Q19) 20260927: ('FormShow', 5290, 5294, …) edOCRTrayLot[] 拿掉——Q19 補上，改成 replace（見上）
        ('FormShow', 5299, 5300, '表單 Left／Top：視窗位置'),
        ('FormShow', 5315, 5315, 'ShowMemo()：說明欄'),
        ('FormClose', 5897, 5918, 'TTL ASE_JP：TestSocket（mykitsuck.h）本 TU 看不到（TList 重複定義），fMain->Send_Command_TTL'),
        ('FormClose', 5933, 5937, '主畫面 UI：fMain->edSetupFileName／sbEngSite 的 Visible（HTML 端處理）'),
        ('FormClose', 5940, 5943, 'fSCKART->Show()/Close()：SCK ART 視窗（移植樹門面沒有 Show/Close）'),
        ('FormClose', 5945, 5946, 'fMain->tPSM.Restart()：省電計時器重啟（移植樹 fMain 沒有 tPSM）'),
        ('FormClose', 5971, 5984, 'I21 Auto Site Map：mykitsuck.h 與 HTEditList.h 的 TList 重複定義，本 TU 看不到 InArmSuck 等'),
        ('FormClose', 5997, 6000, 'dmTrayMotor->bNeedSetVibrateMotSpeed：震動馬達通訊（移植樹沒有 dmTrayMotor）'),
        ('FormClose', 6022, 6022, 'fMain->ToolLoadICO()：工具列圖示（HTML 端處理）'),
        ('CheckConfigurationBeforeSave', 7400, 7400, 'FTestIF->ReadTestIFFile()：移植樹 GATE F-5（porting-gaps 十四）'),
        ('CheckConfigurationBeforeSave', 7637, 7642, 'FSECS->GemInitial：SECS/GEM 初始化（移植樹沒有 FSECS）'),
        ('CheckConfigurationBeforeSave', 7645, 7648, 'fCleaning->LoadAutoCleanData()：移植樹仍是 GATE'),
        ('SaveConfiguration', 7295, 7295, 'fMain->DoShowUserDefFrom()：主畫面 UI'),
        ('SaveConfiguration', 7299, 7299, 'fMain->ChangeMainFormWitdh()：主畫面 UI'),
        ('SaveConfiguration', 7301, 7302, 'FrmAOI->AOIFailCountRefresh()：AOI 未移植'),
        ('SaveConfiguration', 7308, 7308, 'fLotInfo->bVTESTKitCheckPending：移植樹 TfLotInfo 沒有這個欄位'),
        # AI(W906-FRW-S66) 20260926: ('SetA73', 7922, 7922, …) 拿掉——SetTestRunMode 已由 Jimmy 0dcc9c2c 翻好（RunStartMode.cpp:938），照 golden 呼叫（Steven S66）
    ],
    # //AI(W906-FRW-S158) 20260927 [W906]：WS form.event 事件表（Steven ★ Q40＝A，RULINGS_20260926 S157；Q41 盤點 CC-E2／CC-E7）。
    #   控制項↔處理器照 golden cConfiguration.dfm 的 OnClick（行號見各列），產生器在 .gen.inc 檔尾產生 kIC_Events；
    #   FileRW/IniConfig.cpp 檔尾照抄一份再註冊（包一層：開頁檢查、udD46 分上／下兩鍵、每個事件後補 Timer1 一拍）。golden 怪處照翻：
    #   * cbD36Click 綁在 5 格上，但只看 cbD36：D36 勾著時點 D37／D38／D36_1／D36_2 也會把 D33 取消、D35 勾上；
    #   * cbE30Click 綁在 8 格上，每次都依 cbE30／31／32 重算三個面板（點 cbE31_1 等只是重算一次）；
    #   * cbE39Click 勾上時 cbE39_1 取「記憶體裡的 IniConfig 值」（不是畫面上剛才的勾選）；
    #   * cbD21／cbD47／cbF05 golden 沒有 OnClick —— 點完之後 Timer1 下一拍（≤1 秒）才更新底下欄位的顯示 ⇒ 這三格掛 Timer1Timer。
    'events': [('udD46', 'click', 'udD46Click'),                            # dfm:4964（TUpDown Min 5 Max 15 Wrap=False；IniConfig.cpp 拆成 btNext／btPrev）
               ('cbE30', 'click', 'cbE30Click'),                            # dfm:5880
               ('cbE31', 'click', 'cbE30Click'),                            # dfm:6417
               ('cbE31_1', 'click', 'cbE30Click'),                          # dfm:7075
               ('cbE31_2', 'click', 'cbE30Click'),                          # dfm:7733
               ('cbE32', 'click', 'cbE30Click'),                            # dfm:8611
               ('cbE32_1', 'click', 'cbE30Click'),                          # dfm:8631
               ('cbE32_2', 'click', 'cbE30Click'),                          # dfm:8869
               ('cbE33', 'click', 'cbE30Click'),                            # dfm:9134
               ('cbE39', 'click', 'cbE39Click'),                            # dfm:9189
               ('cbD36', 'click', 'cbD36Click'),                            # dfm:4824
               ('cbD37', 'click', 'cbD36Click'),                            # dfm:4835
               ('cbD38', 'click', 'cbD36Click'),                            # dfm:4846
               ('cbD36_1', 'click', 'cbD36Click'),                          # dfm:4857
               ('cbD36_2', 'click', 'cbD36Click'),                          # dfm:4868
               ('cbD21', 'click', 'Timer1Timer'),                           # dfm:4320（沒有 OnClick；Timer1 dfm:22667）
               ('cbD47', 'click', 'Timer1Timer'),                           # dfm:5083（同上）
               ('cbF05', 'click', 'Timer1Timer'),                           # dfm:9788（同上）
               # //AI(W906-EVB4) 20260928 [W906]：B4（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md）。golden 怪處照翻：
               #   * cbA09Click 綁在 cbA09／chkA09_1／cbA14 三格（dfm:579／:781／:936），但只看、只改 cbA09（點 cbA14 也可能把 cbA09 改回）；
               #   * btnRecordJamRateByTimeClear 的 DFM Caption 是 'Set All'（dfm:13903），處理器是清 Jam 率計數；
               #   * 滑桿 'change' 要帶 position（7e1785dc，FileRW/_FormEvent.h 檔頭）：RunPageEvent 先照 VCL 設 Position 再跑 tb*Change。
               ('btD47', 'click', 'btD47Click'),                            # dfm:5120（CC-E1）
               ('btnRecordJamRateByTimeClear', 'click', 'btnRecordJamRateByTimeClearClick'),   # dfm:13905（CC-E3）
               ('btResume', 'click', 'btResumeClick'),                      # dfm:19989（CC-E4）
               ('cbA09', 'click', 'cbA09Click'),                            # dfm:579（CC-E6）
               ('chkA09_1', 'click', 'cbA09Click'),                         # dfm:781（同上，golden 怪處）
               ('cbA14', 'click', 'cbA09Click'),                            # dfm:936（同上，golden 怪處）
               ('tbD25_Index60mm', 'change', 'tbD25_Index60mmChange'),      # dfm:4472（CC-E8）
               ('tbD25_Index30mm', 'change', 'tbD25_Index30mmChange'),      # dfm:4489
               ('tbD25_Index40mm', 'change', 'tbD25_Index40mmChange'),      # dfm:4506
               ('tbD25_Index60mm_NS', 'change', 'tbD25_Index60mm_NSChange'),   # dfm:4547
               ('tbD25_Index40mm_NS', 'change', 'tbD25_Index40mm_NSChange'),   # dfm:4564
               ('tbD25_Index30mm_NS', 'change', 'tbD25_Index30mm_NSChange'),   # dfm:4581
               ('tbD60_Index56mm', 'change', 'tbD60_Index56mmChange'),      # dfm:5448
               ('tbD60_Index56mm_NS', 'change', 'tbD60_Index56mm_NSChange'),   # dfm:5473
               # //AI(W906-EVB10B) 20260929 [W906] CC-L1／X-5：頁面點頁籤送 {"control":"pcConfig","event":"change","activePageIndex":n}
               #   （FileRW/_FormEvent.h X-2；RunPageEvent 先照 VCL 換頁再跑處理器）。程式切頁不送（VCL：程式設 ActivePage 不觸發 OnChange）。
               ('PageControl1', 'change', 'PageControl1Change'),            # dfm:22（OnChange＝PageControl1Change）
               ('pcConfig', 'change', 'pcConfigChange')],                   # dfm:452（OnChange＝pcConfigChange）
}
# AI(W906-E032) 20261003 [W906] (St01)：todo E-032（golden 改回 906 0618，Jimmy RULINGS_20261003 第 2 條）——0618 與 0625 不同的
#   函式 TfConfiguration::InitConfigEdtList_ItemN：0625／V912 多了 [N07-3] 「When SECS GEM disconnect will alarm」（chkN07_3_2 →
#   IniConfig.bN07_Alarm，Steven 20260603），產生檔照產生器根目錄 V912 有這兩列。RULINGS_20261003 第 3 條收了 N07 斷線警報（St02 MR
#   !137／!138，跟 INBOX 86 同一批）、第 1 條（Steven 1003 常設規則）⇒ 留 V912；下面兩列是「等價取代」（取代碼＝產生器原本的輸出，產生的
#   程式不變，只加三段註解；V912 原文進 #if 0 // GATE）。移植樹目前沒有人讀 bN07_Alarm（斷線警報本體在 St02 的 MR）。
#   E-031（產生器根目錄換 906 0618）時 0618 沒有這兩行，要改成「插入」列（或照第 3 條另議），不能只換釘子。
def _e032_row(gl, show):
    """V912 cConfiguration.cpp 第 gl 行＝chkN07_3_2 那列（_expect 釘住），回傳產生器會出的同一句（widget 換成 EL<TCheckBox>）。"""
    _expect(gl, 'elConfig->Add(chkN07_3_2,')
    _expect(gl, show)
    code = _GL[gl - 1].split('//')[0].strip()
    return code.replace('elConfig->Add(chkN07_3_2,', 'elConfig->Add(EL<TCheckBox>("TfConfiguration", "chkN07_3_2"),', 1)


_E032_N07_WHY = ('AI(W906-E032) 原文照留（取代碼＝產生器原本的輸出，產生的程式不變，只加這段註解）。'
                 '(1) 906 0618 cConfiguration.cpp:3226／:3250 之後沒有這一列（0618 沒有 [N07-3] 斷線警報設定，Config.h 也沒有 bN07_Alarm）。'
                 '(2) 0625 cConfiguration.cpp:%d／V912 :%d 有（Steven 20260603 Secs_Gem disconnect alarm）。'
                 '(3) #20 exception (Steven 1003 standing rule, RULINGS_20261003 #1; the N07 alarm itself = RULINGS_20261003 #3)')
STRUCT.setdefault('replace', []).append(
    ('InitConfigEdtList_ItemN', 3325, 3325, _E032_N07_WHY % (3227, 3325), _e032_row(3325, 'bShow, bEnable, bReadFromFile, 0);')))
STRUCT.setdefault('replace', []).append(
    ('InitConfigEdtList_ItemN', 3351, 3351, _E032_N07_WHY % (3253, 3351), _e032_row(3351, 'bNoShow, bDisable, bFixedValue, 0);')))

# AI(W906-W195) 20261009 (St02-E) H6 (RULINGS_20261009 #14: St01 away, St02-E runs St01's generator): golden 913
#   cConfiguration.cpp:992-999 (RogerYang 20260916, HANA RMS) -- 908.x kept the RMS interlock switch under [A76], 912 moved it to [A77];
#   on an upgraded machine the old value is copied once, so the interlock does not silently turn off.  The generator root (912) has
#   no such block: 912 :992 (the cbA77 Add of the CC_HANA_MICRON branch) is kept as it is generated today, the 913 block in front of it.
_H6_WHY = ('AI(W906-W195) 20261009 (St02-E) H6: golden 913 cConfiguration.cpp:992-999 [A76] -> [A77] migration (RogerYang 20260916) '
           'in front of 912 :992 (not in the 912 generator root); RULINGS_20261009 #14')
_expect(992, 'elConfig->Add(cbA77, &IniConfig.bA77_EnableHanaRMSInterlock')
STRUCT.setdefault('replace', []).append(
    ('InitConfigEdtList_ItemA', 992, 992, _H6_WHY,
     'AnsiString sRMSCfg=AuthPath+"config.ini"; '
     'if(CheckKeyExist(sRMSCfg, "A77", "A77_EnableHanaRMSInterlock")==false && CheckKeyExist(sRMSCfg, "A76", "A76_EnableHanaRMSInterlock")==true) '
     '{ WriteIniData(sRMSCfg, "A77", "A77_EnableHanaRMSInterlock", ReadIniData(sRMSCfg, "A76", "A76_EnableHanaRMSInterlock", false)); } '
     'elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbA77"), &IniConfig.bA77_EnableHanaRMSInterlock, ECBool, "A77", "A77_EnableHanaRMSInterlock", bShow, bEnable, bReadFromFile, 0);'))

# AI(W906-W195) 20261009 (St02-E) L10 settings row (RULINGS_20261009 #14: St01 away, St02-E runs St01's generator): golden 913
#   cConfiguration.cpp:1318-1322 [C25] Shuttle Floodgate close by ATC enable only (JerryYang 20260910) after the [C24] else; the C++
#   side (IniConfig.bC25FloodGateCloseByATCEnable, DoFloodGateClose) is L10 !402.  The generator root (912) has no [C25]: 912 :1309 (the
#   `}` that closes the [C24] else) is kept, the 913 rows follow it.  cbC25 is not in the 912 dfm, so no parent tab / enable-list entry
#   and no web element yet -- the row only reads / fixes the value (shown only when SHUTTLE_FLOODGATE==1, as golden).
_C25_WHY = ('AI(W906-W195) 20261009 (St02-E) L10: golden 913 cConfiguration.cpp:1318-1322 [C25] Shuttle Floodgate close by ATC enable '
            'only (JerryYang 20260910) after 912 :1309 (not in the 912 generator root); RULINGS_20261009 #14')
_expect(1308, 'bC24InitialStartCheckCylinder')
_expect(1309, '}')
STRUCT.setdefault('replace', []).append(
    ('InitConfigEdtList_ItemC', 1309, 1309, _C25_WHY,
     '} if(SHUTTLE_FLOODGATE==1) elConfig->Add(EL<TCheckBox>("TfConfiguration", "cbC25"), &IniConfig.bC25FloodGateCloseByATCEnable, '
     'ECBool, "Function", "bC25FloodGateCloseByATCEnable", bShow, bEnable, bReadFromFile, 0); else elConfig->Add(EL<TCheckBox>('
     '"TfConfiguration", "cbC25"), &IniConfig.bC25FloodGateCloseByATCEnable, ECBool, "Function", "bC25FloodGateCloseByATCEnable", '
     'bNoShow, bEnable, bFixedValue, 0);'))
