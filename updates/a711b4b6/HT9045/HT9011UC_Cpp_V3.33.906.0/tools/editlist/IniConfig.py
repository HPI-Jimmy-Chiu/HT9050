# -*- coding: utf-8 -*-
# tools/editlist/IniConfig.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only IniConfig]
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
                'edOCRTrayLotChange'],
    'rettype': {'EnableRMSFunc': 'bool'},
    # 存檔流程的方法：產生器掃出它們「讀」的替身，列成 kIC_SaveReads —— 不在任何 HTEditList 裡的替身
    # （值來自 golden FormShow，例 tbD25_*->Position、dtO06_LastDate->Date）頁面沒送就拒存。
    'save_methods': ['FormClose', 'CheckConfigurationBeforeSave', 'SaveConfiguration', 'LoadConfiguration',
                     'SetA73', 'WriteContactData'],
    # golden 方法參數換掉（伺服器端沒有 Sender／CloseAction；FormClose 本體沒用到它們）
    'params': {'FormClose': '', 'FormShow': '', 'tbD25_Index30mmChange': '', 'tbD25_Index30mm_NSChange': '', 'tbD25_Index40mmChange': '', 'tbD25_Index40mm_NSChange': '', 'tbD25_Index60mmChange': '', 'tbD25_Index60mm_NSChange': '', 'tbD60_Index56mmChange': '', 'tbD60_Index56mm_NSChange': ''},
    # golden FormShow :4619-4623 for(i<pcConfig->PageCount) if(Pages[i]!=tsSearchFunction) ChangeCompomentEnabled(…)
    'enable_all_root': 'pcConfig',
    'enable_all_skip': ['tsSearchFunction'],
    # 以指定程式碼取代的 golden 段落（方法, 起, 迄, 原因, 取代碼）
    'replace': [
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
                '   // golden myQwertyKeyBoard.cpp:248-257／:285-301（AI(W906-FRW-Q19) 20260927）'],
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
              'void SetTestRunMode();        // RunStartMode.cpp:938（golden cConfiguration.cpp:89 extern；Jimmy 0dcc9c2c 已翻）  AI(W906-FRW-S66) 20260926'],
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
}
