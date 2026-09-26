# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_TesterIF.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_TesterIF
#
# Steven 團隊 20260925：TestIF_File 的 TFTestIF 半邊（<recipe>\Tester.Data；RS232 模式另寫
# D:\RS232Standard\System\Setup.ini）—— golden V912 TFTestIF（cTesterIF.cpp，1588 行，cp950）。
# 一律照 V912（Steven 20260925，RULINGS_20260925.md 第 37 條）；客戶專屬條件（第 25 條）：編得過就照 golden 留著，
# 需要額外移植（缺表單／缺欄位）的才擋掉並註記（每一條見 replace 的原因）。
#
# 結構名 TestIF_File_TesterIF：tag "TestIF_File" 已被 A 形狀 FileRW/TestIF_File.cpp（kBridge_TFTestIF／TfSetup／
#   TfYieldMonitoring）佔用。A 形狀的 TFTestIF 部分因為讀檔端（ReadTestIFFile）沒有而 sourceGap；本檔把 golden
#   ReadTestIFFile 一起轉出來（C 形狀的 static 方法），所以開頁、存檔、存檔後重讀、開機讀檔都是同一份 golden 碼。
#
# 開頁（editlist.get）＝golden FormShow（:109）：InitcbDIOType → ReadTestIFFile（尾端 DoIniDataToForm）→ 顯示／權限。
# 存檔（editlist.save）＝golden spbSaveClick（:1320）：A02 守衛 → SaveSetupFile → SECS → ReadTestIFFile → 備份 →
#   SetWorkParameter。FileRW/TestIF_File_TesterIF.cpp 在前面補「數字欄位預檢」（見那支檔頭）。
# 開機／換配方：golden TfMain::DoReadLastData main.cpp:9327 FTestIF->ReadTestIFFile()（＋:9387 DoIniDataToForm）
#   → FileRW_TesterIF_ReadTestIFFile()（FileRW/TestIF_File_TesterIF.cpp）。
#
# 表單成員：
#   fShow —— 本 TU 自己的（static）。不寫 FTestIF->fShow：golden Command.cpp:7355／:9955（移植樹 :10247／:15081，
#     目前 #if 0）拿它當「有表單開著」的判斷；網頁沒有關表單事件，寫進去會永遠是 true（同 TestIF_File_SetUp 的 fShow）。
#   bIsResetRs232Standard —— #define 到 FTestIF 那一份（forms/fTesterIF.h:438）：golden main.cpp:18520 的計時器讀
#     FTestIF->bIsResetRs232Standard 來重啟 RS232 小程式（golden 只有一個表單物件）。
import os
import re

G = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(G, 'cTesterIF.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
F = '"TFTestIF"'

METHODS = ['TFTestIF', 'InitcbDIOType', 'FormShow', 'SaveSetupFile', 'CheckRs232StandardIni', 'ReadTestIFFile',
           'DoIniDataToForm', 'rgInterfaceTypeClick', 'ShowPageControl2', 'ShowTTLState', 'cbDIOTypeChange',
           'AntiSignalCBoxClick', 'cbRs232TypeChange', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTFTestIF::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('span %s' % meth)


SPANS = {m: _span(m) for m in METHODS}


def L(meth, text, nth=1):
    """golden 行號（1 起）：meth 本體裡第 nth 個含 text 的行。行號漂了就中止（不蓋錯地方）。"""
    a, b = SPANS[meth]
    k = 0
    for gl in range(a, b + 1):
        if text in _cpp[gl - 1]:
            k += 1
            if k == nth:
                return gl
    raise SystemExit('L(%s, %r) not found' % (meth, text))


def RANGE(meth, t1, t2):
    a = L(meth, t1)
    for gl in range(a, SPANS[meth][1] + 1):
        if t2 in _cpp[gl - 1]:
            return a, gl
    raise SystemExit('RANGE(%s,%r,%r)' % (meth, t1, t2))


# golden SaveSetupFile 用 ->Text.ToDouble() 的元件（vclcompat 遇到非數字丟例外 → 寫到一半）：
# FileRW/TestIF_File_TesterIF.cpp 存檔前先檢查這些（kTIF_ToDoubleTexts），一個不過就整筆不寫。
_sv = SPANS['SaveSetupFile']
_TODOUBLE = []
for _gl in range(_sv[0], _sv[1] + 1):
    for _m in re.finditer(r'\b(\w+)->Text\.ToDouble\(\)', _cpp[_gl - 1].split('//')[0]):
        if _m.group(1) not in _TODOUBLE:
            _TODOUBLE.append(_m.group(1))

_amr = RANGE('ReadTestIFFile', '//==> Ifor 20260807', '//<== Ifor 20260807')
_rpd = RANGE('spbSaveClick', 'if(CosFunction.bRecipeParameterDefault)', 'fRPDefault->CompareRPDefaultAndValue')
_rpd = (_rpd[0], _rpd[1] + 1)          # 含收尾的 }
_hint = RANGE('TFTestIF', 'edInitialMaxTest->Hint', 'edStartDelayTime->Hint')
_tcp_off = RANGE('ReadTestIFFile', 'fTesterTCP->TimerTCPIPConnect->Enabled=false;', 'fLotInfo->labTCPIPSimulate->Visible=true;')
_tcp_on = RANGE('ReadTestIFFile', 'fTesterTCP->TimerTCPIPConnect->Enabled=true;', 'fTesterTCP->TimerProcessTCPData->Enabled=true;')
# 第二個 "TimerTCPIPConnect->Enabled=false"（else 分支，:614-617）
_tcp_els = (L('ReadTestIFFile', 'fTesterTCP->TimerTCPIPConnect->Enabled=false;', 2), L('ReadTestIFFile', 'fLotInfo->labTCPIPSimulate->Visible=false;'))
_dio_name = RANGE('ReadTestIFFile', 'TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode];', 'WriteIniData(szDir, "DIO",       "TypeName"')
_cbdio = RANGE('cbDIOTypeChange', 'AnsiString S;', 'fDIOFrom->LoadData(S);')

CB_DIO = 'EL<TComboBox>(%s, "cbDIOType")' % F

REPLACE = [
    # ---- 建構子（golden :40，CreateForm HT9045.cpp:189）
    ('TFTestIF', L('TFTestIF', 'cbGPIBType->Clear();'), L('TFTestIF', 'cbGPIBType->Clear();'),
     'CC_ASE_KaohSiung 的 GPIB 型態改名（客戶專屬，編得過照留）：cbGPIBType->Clear() 照 VCL 語意（Items 清空＋ItemIndex=-1＋Text=""）',
     'TIF_CbClear(EL<TComboBox>(%s, "cbGPIBType"));' % F),
    ('TFTestIF', L('TFTestIF', 'cbGPIBType->Refresh();'), L('TFTestIF', 'cbGPIBType->Refresh();'),
     'cbGPIBType->Refresh()：重畫（vclcompat TControl 沒有 Refresh；HTML 端自己畫）', ';'),
    ('TFTestIF', _hint[0], _hint[1],
     'CC_Greatek 的 ->Hint（Default Recipe ChangeLog 用的鍵名；vclcompat 沒有 Hint，RPDefault 未移植）', ';'),
    # ---- InitcbDIOType（golden :70）
    ('InitcbDIOType', L('InitcbDIOType', 'cbDIOType->Clear();'), L('InitcbDIOType', 'cbDIOType->Clear();'),
     'cbDIOType->Clear()：VCL 語意（Items 清空＋ItemIndex=-1＋Text=""；vclcompat Clear 只清 Items）；同時重置 bDioListLost',
     'TIF_CbClear(%s); bDioListLost=false;' % CB_DIO),
    ('InitcbDIOType', L('InitcbDIOType', 'Application->MessageBox('), L('InitcbDIOType', 'Application->Terminate();'),
     'DIO 檔一個都沒有＋bAlarm==false：golden MessageBox＋Application->Terminate()（結束程式）。網頁版改 ELMessage、'
     '記 bDioListLost → 這次之後的存檔一律拒絕（FileRW/TestIF_File_TesterIF.cpp SaveFlow），不結束 wb_serve（同 FileRW/TTLCfg.cpp）',
     'filerw::ELMessage("DIO data has been lossed, please check!!", "DIO Data loss"); bDioListLost=true; '
     'std::printf("FileRW TesterIF: golden InitcbDIOType -- no *.ini in DIOCFGPath (%s); golden terminates here, saves are refused\\n", DIOCFGPath.c_str());'),
    # ---- FormShow（golden :109）
    ('FormShow', L('FormShow', 'Caption=S;'), L('FormShow', 'Caption=S;'), 'Caption：視窗標題（HTML 自己有）', ';'),
    ('FormShow', L('FormShow', 'Left=100;'), L('FormShow', 'Top=10;'), 'Left／Top：視窗位置', ';'),
    ('FormShow', L('FormShow', 'cbRs232TypeChange(this);'), L('FormShow', 'cbRs232TypeChange(this);'),
     '事件以 (this) 呼叫（參數已拿掉）', 'TIF_cbRs232TypeChange();'),
    # ---- ReadTestIFFile（golden :563）
    ('ReadTestIFFile', L('ReadTestIFFile', 'if(TestIF_File.iTestType>rgInterfaceType->Items->Count-1)'),
     L('ReadTestIFFile', 'if(TestIF_File.iTestType>rgInterfaceType->Items->Count-1)'),
     '產生器的元件改寫不接在 ">" 後面（為了避開 "->"），這一行的 rgInterfaceType 沒改到 → 等價改寫（同一個判斷，DFM 4 項）',
     'if(TestIF_File.iTestType>EL<TRadioGroup>(%s, "rgInterfaceType")->Items->Count-1)' % F),
    ('ReadTestIFFile', _tcp_off[0], _tcp_off[1],
     'TCP/IP 模式＋OFF_LINE：fTesterTCP 的兩個 TTimer 與 fLotInfo->labTCPIPSimulate —— 移植樹門面沒有（forms/fTesterTCP.h 無 TTimer，'
     'TfLotInfo 無 labTCPIPSimulate）',
     'filerw::ELTodo("golden cTesterIF.cpp:585-587 TCP/IP tester OFF_LINE: fTesterTCP->TimerTCPIPConnect/TimerProcessTCPData->Enabled=false, '
     'fLotInfo->labTCPIPSimulate->Visible=true -- not in the port facades (no TTimer / labTCPIPSimulate)");'),
    ('ReadTestIFFile', _tcp_on[0], _tcp_on[1],
     'TCP/IP 模式：golden 啟動 fTesterTCP 的連線／收資料計時器（TCP/IP 測試機連線）—— 移植樹門面沒有 TTimer（forms/fTesterTCP.h:298-306）',
     'filerw::ELTodo("golden cTesterIF.cpp:591-592 TCP/IP tester: fTesterTCP->TimerTCPIPConnect/TimerProcessTCPData->Enabled=true -- '
     'not ported (forms/fTesterTCP.h has no TTimer): the TCP/IP tester link is NOT started by this recipe");'),
    ('ReadTestIFFile', _tcp_els[0], _tcp_els[1],
     '非 TCP/IP 模式：golden 停掉 TCP/IP 計時器、關 ClientSocket_TCPIP、藏 labTCPIPSimulate。移植樹門面沒有這三樣，'
     '也就沒有被這條路徑打開過的 TCP/IP 連線可停 → 空敘述（不記 todo：每個非 TCP/IP 配方每次讀檔都會走到）', ';'),
    ('ReadTestIFFile', _amr[0], _amr[1],
     '[AMR] 四鍵（Ifor 20260807／20260814，golden 912）：欄位 iAMRAutoStartDelay／iAMRLoaderDetectDelay／iAMRWaitSupplyDelay／'
     'bAMRICQtyAutoCleanOut 在 golden cprod.h:2596-2599，移植樹 SYSTEM_TEST_IF 沒有（第 35 條只補了另外三欄）→ 不讀（ReadIniData 純讀，'
     '不讀不改檔）。要讀：Jimmy／JerryYang 先把四欄補進 cprod.h，再拿掉這條 replace',
     'filerw::ELTodo("golden cTesterIF.cpp:626-646 Tester.Data [AMR] Auto Start Delay/Loader Detect Delay/Wait Supply Delay/IC Qty Auto Clean Out '
     'NOT read -- port SYSTEM_TEST_IF has no iAMRAutoStartDelay/iAMRLoaderDetectDelay/iAMRWaitSupplyDelay/bAMRICQtyAutoCleanOut (golden cprod.h:2596-2599)");'),
    ('ReadTestIFFile', _dio_name[0], _dio_name[1],
     'golden :846 MyList->Strings[iDioMode] 沒檢查範圍：BCB TStringList 丟 EStringListError（:847 不會寫）；vclcompat TStringList 回 ""'
     '（TStringList.cpp:76）→ 照抄會把 TypeName="" 寫進 Tester.Data。範圍內照 golden 取值＋寫 TypeName；範圍外不取、不寫、回報'
     '（同 FileRW/TTLCfg.cpp FileRW_TTLCfg_ReadDIOSection 的決斷）',
     'if(TestIF_File.iDioMode<MyList->Count) { TestIF_File.sDioName=MyList->Strings[TestIF_File.iDioMode]; '
     'WriteIniData(szDir, "DIO",       "TypeName",     TestIF_File.sDioName); } '
     'else { AnsiString e; e.sprintf("golden cTesterIF.cpp:846 MyList->Strings[%d] out of range (%d DIO files in DIOCFGPath) -- golden throws '
     'EStringListError here and :847 never writes; TypeName NOT written, TestIF_File.sDioName left as \\"%s\\"", TestIF_File.iDioMode, MyList->Count, '
     'TestIF_File.sDioName); std::printf("FileRW TesterIF: %s\\n", e.c_str()); filerw::ELTodo(e.c_str()); }'),
    ('ReadTestIFFile', L('ReadTestIFFile', 'FrmAOI->bCheckAOIFailBinUse()'), L('ReadTestIFFile', 'FrmAOI->bCheckAOIFailBinUse()'),
     'FrmAOI->bCheckAOIFailBinUse()（CC_KYEC_LEE 的 AOI fail bin，Eastsun 20260710）：移植樹 TFrmAOI（forms/fAOI.h）沒有這支；'
     'golden 本體 fAOI.cpp:9876 是純查詢（無副作用）→ 客戶碼先判（&& 左右交換不改結果），KYEC_LEE 才記 todo、當 false',
     '(CUSTOMER_CODE==CC_KYEC_LEE && TIF_AOIFailBinUse())))    //Eastsun 20260710 Merge'),
    ('ReadTestIFFile', L('ReadTestIFFile', 'ATKRecipeInfo->SaveFile();'), L('ReadTestIFFile', 'ATKRecipeInfo->SaveFile();'),
     'ATKRecipeInfo：移植樹沒有建立（database.cpp:149 註解掉）→ 空指標。golden ATK_RECIPE_INFO::SaveFile（移植樹 cprod.cpp:497）'
     '非 CC_AMKOR_Korea／China 一進去就 return → 指標在就照呼叫；不在時 AMKOR 才記 todo（其他客戶 golden 本來就什麼都不做）',
     'if(ATKRecipeInfo) ATKRecipeInfo->SaveFile(); else if(CUSTOMER_CODE==CC_AMKOR_Korea || CUSTOMER_CODE==CC_AMKOR_China) '
     'filerw::ELTodo("golden cTesterIF.cpp:991 ATKRecipeInfo->SaveFile() (AMKOR Information.txt / D:\\\\eRMS) not run -- ATKRecipeInfo is not created in the port (database.cpp:149)");'),
    # ---- CheckRs232StandardIni（golden :497）
    ('CheckRs232StandardIni', L('CheckRs232StandardIni', 'fMain->CloseGpibProgram(__FUNC__);'), L('CheckRs232StandardIni', 'fMain->CloseGpibProgram(__FUNC__);'),
     'fMain->CloseGpibProgram：TfMain 門面沒有（golden main.cpp:29132：WM_COPYDATA MSG_CMD_CloseGpib 叫 GPIB／RS232 小程式關掉重讀 Setup.ini）。'
     'bIsResetRs232Standard 仍照 golden 設在 FTestIF 上（golden main.cpp:18520 的計時器另外靠它重啟小程式）',
     'filerw::ELTodo("golden cTesterIF.cpp:557 fMain->CloseGpibProgram(__FUNC__) not in the TfMain facade -- D:\\\\RS232Standard\\\\System\\\\Setup.ini was '
     'rewritten but the RS232 helper program is NOT told to close/reload (FTestIF->bIsResetRs232Standard=true is set as golden)");'),
    # ---- DoIniDataToForm（golden :997）
    ('DoIniDataToForm', L('DoIniDataToForm', 'rgInterfaceTypeClick(this);'), L('DoIniDataToForm', 'rgInterfaceTypeClick(this);'),
     '事件以 (this) 呼叫（參數已拿掉）', 'TIF_rgInterfaceTypeClick();'),
    # ---- ShowPageControl2（golden :1192）
    ('ShowPageControl2', L('ShowPageControl2', 'tsTemp[Index]->TabVisible=true;'), L('ShowPageControl2', 'tsTemp[Index]->TabVisible=true;'),
     'tsTemp[Index] 沒檢查範圍（rgInterfaceType->ItemIndex＝TestIF_File.iTestType，檔案是負數時 golden 讀陣列外 → AV）→ 範圍外記 todo 並 return',
     'if(Index<0 || Index>3) { filerw::ELTodo("golden cTesterIF.cpp:1197 ShowPageControl2: Index out of 0..3 (golden indexes tsTemp[] unchecked)"); return; } '
     'tsTemp[Index]->TabVisible=true;'),
    ('ShowPageControl2', L('ShowPageControl2', 'cbRs232TypeChange(this);'), L('ShowPageControl2', 'cbRs232TypeChange(this);'),
     '事件以 (this) 呼叫（參數已拿掉）', 'TIF_cbRs232TypeChange();'),
    # ---- cbDIOTypeChange（golden :1299）
    ('cbDIOTypeChange', _cbdio[0], _cbdio[1],
     'fDIOFrom->GetDIOFileName()＋LoadData(S)：golden TfDIOFrom 的 C 形狀在 FileRW/TTLCfg.cpp（FileRW_TTLCfg_TFTestIF_cbDIOTypeChange，'
     '逐行同 golden :1301-1303，讀 cbDIOType 替身＝同一個 EL<TComboBox>("TFTestIF","cbDIOType")）',
     'FileRW_TTLCfg_TFTestIF_cbDIOTypeChange();'),
    # ---- spbSaveClick（golden :1320）
    ('spbSaveClick', L('spbSaveClick', 'Close();'), L('spbSaveClick', 'Close();'),
     'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（_EditPage 讓下次存檔前重新開頁），頁面自己關；'
     '沒寫檔 → PageDesc::reload（golden FormClose 的 ReadTestIFFile）還原替身',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', _rpd[0], _rpd[1],
     'Recipe Parameter Default ChangeLog（fRPDefault／fSpeed／fCleaning／fYieldMonitoring->SearchRecipeParameter、本表單 SearchRecipeParameter '
     '的 TWinControl 走訪）移植樹沒有（同 A 形狀 tools/formbridge/TFTestIF.py 的 block）',
     'if(CosFunction.bRecipeParameterDefault) filerw::ELTodo("golden cTesterIF.cpp:1348-1371 Recipe Parameter Default ChangeLog (fRPDefault / SearchRecipeParameter) not ported");'),
]

STRUCT = {
    'struct': 'TestIF_File_TesterIF',
    'prefix': 'TIF',
    'class': 'TFTestIF',
    'cpp': 'cTesterIF.cpp',
    'h': 'cTesterIF.h',
    'files': ['Tester.Data', 'D:\\RS232Standard\\System\\Setup.ini（RS232 模式，CheckRs232StandardIni）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TFTestIF': '', 'FormShow': '', 'rgInterfaceTypeClick': '', 'cbDIOTypeChange': '',
               'AntiSignalCBoxClick': '', 'cbRs232TypeChange': '', 'spbSaveClick': ''},
    'members': [
        'bool fShow=false;                 // golden cTesterIF.h:302 —— 本 TU 自己的（見檔頭：不寫 FTestIF->fShow）',
        '#define bIsResetRs232Standard (FTestIF->bIsResetRs232Standard)   // golden cTesterIF.h:303 —— golden main.cpp:18520 計時器讀 FTestIF 這一份',
        'int ioldTestType=-1;              // golden cTesterIF.h:304（建構子 :44 設 -1；FormShow :331 記）',
        'int ioldDIOType=-1;               // golden cTesterIF.h:305',
        'bool bflag=false;                 // golden cTesterIF.cpp:38（file-scope static；rgInterfaceTypeClick 用，golden FormClose :1219 清）',
        'bool bDioListLost=false;          // FileRW：golden InitcbDIOType 找不到 DIO 檔時 Application->Terminate() → 網頁版拒存（見 replace）',
        # VCL TCustomComboBox::Clear（CB_RESETCONTENT）：Items 清空、沒有選取、編輯框清空（vclcompat Clear 只清 Items）
        'void TIF_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }',
        # golden fAOI.cpp:9876 TFrmAOI::bCheckAOIFailBinUse（純查詢）—— 移植樹沒有（見 replace）
        'bool TIF_AOIFailBinUse() { filerw::ELTodo("golden cTesterIF.cpp:937 FrmAOI->bCheckAOIFailBinUse() (CC_KYEC_LEE AOI fail bin -> 33 bins) not ported '
        '(port TFrmAOI has none) -- treated as false"); return false; }',
        # golden ShowTTLState（:1244-1277）讀 fDIOFrom 六個元件的設計期 Items：golden TfDIOFrom 的替身在 FileRW/TTLCfg（DFM Items 由
        # FileRW_TTLCfg_Boot → DI_DfmItems 放）→ 同一組具名替身（鍵＝("TfDIOFrom", 名稱)）。golden 文字不改，fDIOFrom 換成這個檢視。
        'TIF_DIOFromView* TIF_DIOFrom() { static TIF_DIOFromView v; '
        'v.rgStartLogic=filerw::EL<TRadioGroup>("TfDIOFrom", "rgStartLogic"); v.rgStartChannel=filerw::EL<TRadioGroup>("TfDIOFrom", "rgStartChannel"); '
        'v.cbSignalType=filerw::EL<TComboBox>("TfDIOFrom", "cbSignalType"); v.rgBinLogic=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinLogic"); '
        'v.rgBinBitLength=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinBitLength"); v.rgBinDataType=filerw::EL<TRadioGroup>("TfDIOFrom", "rgBinDataType"); '
        'return &v; }',
        '#define fDIOFrom TIF_DIOFrom()   // golden ShowTTLState :1244-1277（FileRW/TestIF_File_TesterIF.cpp 用完 #undef）',
        # 存檔預檢（FileRW/TestIF_File_TesterIF.cpp）：golden SaveSetupFile 以 ->Text.ToDouble() 轉的元件
        'const char* const kTIF_ToDoubleTexts[] = {' + ', '.join('"%s"' % n for n in _TODOUBLE) + '};   // golden SaveSetupFile ->Text.ToDouble()',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'MessageDef.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h',
                 'forms/fSCKART.h', 'forms/fTesterTCP.h', 'forms/fShowBinSelect.h', 'forms/fTesterIF.h', 'csystem.h',
                 'cinitial.h', 'BarcodeReader.h', 'myswitch.h', 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'decls': ['#include <windows.h>   // FindFirstFile（golden InitcbDIOType :76、ReadTestIFFile :825）',
              '#include <cstdio>      // sprintf（golden ShowTTLState）',
              '#include <cstring>     // strcmp',
              'extern bool bAuthCriticalPara[26];            // cAuthority.h:64（golden cAuthority.h；cAuthority.h 帶 language.h，與 HTEditList.h 衝突）',
              'void FileRW_TTLCfg_TFTestIF_cbDIOTypeChange(); // FileRW/TTLCfg.cpp（golden cTesterIF.cpp:1301-1303 GetDIOFileName＋LoadData）',
              '// golden ShowTTLState 讀的 fDIOFrom 六個元件（見 members TIF_DIOFrom）',
              'struct TIF_DIOFromView { TRadioGroup *rgStartLogic, *rgStartChannel; TComboBox *cbSignalType; '
              'TRadioGroup *rgBinLogic, *rgBinBitLength, *rgBinDataType; };'],
    'overrides': [],
}
