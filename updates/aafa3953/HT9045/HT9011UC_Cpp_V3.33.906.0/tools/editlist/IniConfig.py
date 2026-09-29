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
                'btD47Click', 'btnRecordJamRateByTimeClearClick', 'btResumeClick', 'cbA09Click'],
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
               'btD47Click': '', 'btnRecordJamRateByTimeClearClick': '', 'btResumeClick': '', 'cbA09Click': ''},
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
               ('tbD60_Index56mm_NS', 'change', 'tbD60_Index56mm_NSChange')],   # dfm:5473
}
