# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_SetUp.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_SetUp
#
# Steven 團隊 20260925：TestIF_File 的 TfSetup 半邊（<recipe>\HandlerCondition.Data ＋ Contact.Data 兩鍵 ＋
# Temperature.Data [ATC]）—— golden TfSetup（cSetUp.cpp，912，4891 行）。
# 結構名 TestIF_File_SetUp：TestIF_File 已被 A 形狀 FileRW/TestIF_File.cpp（kBridge_TfSetup）佔用。
# 前一版 A 形狀 tools/formbridge/TfSetup.py「可顯示、不可存」（存檔依賴開表單當下的表單狀態）；C 形狀的成員是
# static，開頁 FormShow 設的 bNeedPassword／iASMSiteMap／iTestSiteCh／iASMTestMode 存檔時還在 → 可以存。
#
# 讀檔器：接移植樹 fSetup->ReadFile()（cSetUp.cpp:416，live，開機／cStartCondition／csystem 共用同一支）。
#   理由：golden ReadFile（:2199-3040，840 行）只有三處碰表單：ScrollBar1->Position（RTC/OCR 分支，fShow 時）、
#   rgInOutArmYPitch->ItemIndex（決定 In Out Arm Y Pitch 的預設值）、tSiteMap（SECS EC 3540 的指標）。
#   另轉一份會多一個讀檔器（且要重做移植樹已審過的 ~20 個 GATE）。改成 SU_PortReadFile()：呼叫前把這三樣
#   與 fShow／bSavePressed 同步到移植樹 fSetup，呼叫後 fShow 還原（見 members 最後一條）。
# 表單成員：移植樹 fSetup 有、且移植樹其他程式會讀的（bSavePressed、bFirstTime、iTestMode、iTestModeOcr、
#   bSiteMapHasChange、shtMode）→ #define 到 fSetup 那一份（golden 只有一個表單物件）。
#   fShow 例外：留在本 TU（static）—— 頁面沒有關表單事件，若寫到 fSetup->fShow=true 會永久跳過
#   ckernel.cpp:877 的 START 回原點檢查（安全相關）。只在呼叫讀檔器那一刻借給 fSetup。
import os
import re

G = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(G, 'cSetUp.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_h = open(os.path.join(G, 'cSetUp.h'), 'rb').read().decode('cp950', errors='replace')
_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_W = {}
for _m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _h[re.search(r'class\s+TfSetup\b[^;{]*\{', _h).end():], re.M):
    _W[_m.group(2)] = _m.group(1) if _m.group(1) in _KNOWN else 'TControl'

METHODS = ['TfSetup', 'FormShow', 'DoIniDataToForm', 'ScrollBar1Change', 'CompChange', 'chkOffCenterkitClick',
           'rgYPitchOffsetModeClick', 'FormClose', 'sbUpdateClick', 'CHSetError', 'CheckShuttlePitch', 'SaveSetupFile',
           # Steven 團隊 20260925：存檔前重播的 golden 事件（FileRW/TestIF_File_SetUp.cpp BeforeApply）——
           #   CoSocketComboChange（:4785，CoSocketCombo 改值 → rgSensor 顯示／值）、
           #   btnLUpToRDownNClick（:4200，六顆 Site Map 排序鈕共用，方向＝Sender->Tag → IniConfig.iSiteMapDirection）
           'CoSocketComboChange', 'btnLUpToRDownNClick']
SB = 'filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")'
CSC = 'EL<TComboBox>("TfSetup", "CoSocketCombo")'
# golden TComboBox（Style=csDropDownList，cSetUp.dfm cbAa..cbDh／CoSocketCombo）的 VCL 語意：Clear() 之後 ItemIndex=-1、Text=""；
# 設 ItemIndex 超出 Items 範圍 → -1（CB_SETCURSEL 失敗會清掉選取）；範圍內 → Text=Items[i]。vclcompat TComboBox 的
# ItemIndex／Text 是裸欄位 → golden 對 TestSiteCH[][]／CoSocketCombo 的 ->ItemIndex=／->Clear() 改呼叫 SU_CbSetIndex／SU_CbClear
# （members 最後兩條；同 FileRW/TTLCfg.cpp ComboSetItemIndex／ComboClear）。不改的話：CompChange 清掉 Items 後，
# DoIniDataToForm 設的 ItemIndex 仍是檔案值（golden 是 -1 → SaveSetupFile :3891-3894 寫 0），SaveSetupFile :3733 寫的
# CoSocketCombo->Text 是空字串（golden 是 "8"）。
_CB_SET = re.compile(r'((?:TestSiteCH\[[^\]]*\]\[[^\]]*\])|(?:' + re.escape(CSC) + r'))\s*->\s*ItemIndex\s*=(?!=)\s*([^;]+);')
_CB_CLR = re.compile(r'(TestSiteCH\[[^\]]*\]\[[^\]]*\])\s*->\s*Clear\s*\(\s*\)\s*;')


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfSetup::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('span %s' % meth)


SPANS = {m: _span(m) for m in METHODS}


def _conv(code):
    """一行 golden 程式碼 → 產生器同樣的改寫（widget→EL<>、方法→SU_），再加 ScrollBar1／tSiteMap／(this) 的規則。
    只用在自動 replace 的行（產生器不改寫 replace 的取代碼）。"""
    code = code.replace('ScrollBar1->', SB + '->')
    code = re.sub(r'(?<![\w.>:])tSiteMap->', 'fSetup->tSiteMap->', code)
    code = code.replace('rgYPitchOffsetMode->OnClick(this)', 'SU_rgYPitchOffsetModeClick()')
    code = re.sub(r'(?<![\w.>:"])(' + '|'.join(METHODS) + r')\s*\(\s*this\s*\)', r'SU_\1()', code)
    code = re.sub(r'(?<![\w.>:])DoPassword\(\)', 'filerw::ELPasswordRefused("TfSetup::DoPassword (golden password form)")', code)
    names = sorted(_W, key=len, reverse=True)
    code = re.sub(r'(?<![\w.>:"(])(' + '|'.join(map(re.escape, names)) + r')\b(?!\s*::)(?!")',
                  lambda m: 'EL<%s>("TfSetup", "%s")' % (_W[m.group(1)], m.group(1)), code)
    code = re.sub(r'(?<![\w.>:])(' + '|'.join(METHODS) + r')\s*\(', lambda m: 'SU_' + m.group(1) + '(', code)
    code = _CB_SET.sub(lambda m: 'SU_CbSetIndex(%s, %s);' % (m.group(1), m.group(2).strip()), code)
    code = _CB_CLR.sub(lambda m: 'SU_CbClear(%s);' % m.group(1), code)
    return code


def _auto():
    out = []
    pat = re.compile(r'ScrollBar1->(Position|Max)|(?<![\w.>:])tSiteMap->|OnClick\(this\)|\b(' + '|'.join(METHODS) +
                     r')\s*\(\s*this\s*\)|(?<![\w.>:])DoPassword\(\)'
                     r'|TestSiteCH\[[^\]]*\]\[[^\]]*\]\s*->\s*(ItemIndex\s*=(?!=)|Clear\s*\()|(?<![\w.>:])CoSocketCombo\s*->\s*ItemIndex\s*=(?!=)')
    for m, (a, b) in SPANS.items():
        for gl in range(a + 1, b):
            raw = _cpp[gl - 1]
            code = raw.split('//')[0]
            if '"' in code and 'ScrollBar1' not in code and 'tSiteMap' not in code:
                continue
            if not pat.search(code):
                continue
            if m == 'TfSetup' and gl in (167, 172, 176, 180, 185, 190):
                continue          # 建構子另有手寫 replace（ScrollBar1->Max、tSiteMap）
            if m == 'SaveSetupFile' and _sv_k3[0] <= gl <= _sv_k3[1]:
                continue          # K3 段整段手寫取代
            out.append((m, gl, gl, '自動：ScrollBar1（golden TScrollBar，產生器當 TControl → 改 filerw::ELTrackBar：Position／Max／OnChange 照 VCL）'
                        '、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（一律拒）'
                        '、TestSiteCH／CoSocketCombo 的 ->ItemIndex=／->Clear()（VCL csDropDownList 語意 → SU_CbSetIndex／SU_CbClear）',
                        _conv(code.rstrip())))
    return out


def L(meth, text, nth=1):
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


_ct_sm = RANGE('TfSetup', 'tSiteMap=new TStringList();', 'tSiteMap->Add("0");')
_ct_rg = RANGE('TfSetup', 'MyTempRGBox[i]=new TRadioGroup(this);', 'MyTempRGBox[i]->OnClick=rgSensor1Click;')
_ct_mx = RANGE('TfSetup', 'if(MachineTypeChoice==Type_HT9045)', 'ScrollBar1->Max=_16Site2X8;')
_sv_k3 = RANGE('SaveSetupFile', 'if(CUSTOMER_CODE==CC_ASE_KaohSiung_K3)', 'K++;')
_sv_k3 = (_sv_k3[0] + 1, _sv_k3[1] + 3)   # 保留 if(...)，只換 { … }（後面接 else）
_fs_sig = RANGE('FormShow', 'if(CUSTOMER_CODE==CC_SIGURD_PeiXing)', 'delete sTmp;')

MANUAL = [
    # ---- 建構子
    ('TfSetup', _ct_mx[0], _ct_mx[1] + 1, 'ScrollBar1->Max（依 MachineTypeChoice）：golden TScrollBar → filerw::ELTrackBar（Max 變了會重新夾 Position，同 VCL）',
     'if(MachineTypeChoice==Type_HT9045) ' + SB + '->Max=_8Site2X4; '
     'else if(MachineTypeChoice==Type_HT9046_LS || MachineTypeChoice==Type_HT1032) ' + SB + '->Max=_32Site4X8N; '
     'else if(MachineTypeChoice==Type_HT9045_12Site) ' + SB + '->Max=_12Site2X6; '
     'else ' + SB + '->Max=_16Site2X8;'),
    ('TfSetup', _ct_sm[0], _ct_sm[1] + 2, 'tSiteMap=new TStringList＋補 32 個 "0"：移植樹 fSetup->Init()（cSetUp.cpp:1348，同一段 golden 建構子）已建好；'
     'SECS EC 3540（uHGemHT9045_EC.cpp:761）拿的是 fSetup->tSiteMap 指標 —— 再 new 一份會讓 SECS 指到舊的',
     'if(!fSetup->tSiteMap) filerw::ELTodo("TfSetup ctor: fSetup->tSiteMap is null -- fSetup->Init() must run before FileRW_Setup_Boot()");'),
    ('TfSetup', _ct_rg[0], _ct_rg[1], 'MyTempRGBox[i]=new TRadioGroup(this)，Name="rgSensor"+(i+1)：改成具名替身（名稱＝頁面元件 id）。'
     'Parent／Height／Caption／Columns／Align／OnClick 是畫面（頁面自己有）',
     'MyTempRGBox[i]=EL<TRadioGroup>("TfSetup", (AnsiString("rgSensor")+AnsiString(i+1)).c_str()); '
     'Str.sprintf("Sensor %d usage", i+1); '
     'MyTempRGBox[i]->Items->Add("No use"); MyTempRGBox[i]->Items->Add("Has IC"); MyTempRGBox[i]->Items->Add("Floating");'),
    # ---- FormShow
    ('FormShow', L('FormShow', 'if(InArmSuck.HasIC()==true || OutArmSuck.HasIC()==true ||'), L('FormShow', 'ShuttleHasIC()==true   || IndexHasIC()==true)'),
     'InArmSuck／OutArmSuck：aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義 → FileRW/_KitSuck.cpp 轉接（同一個判斷）',
     'if(FileRW_InOutArmSuckHasIC() || ShuttleHasIC()==true || IndexHasIC()==true)'),
    ('FormShow', L('FormShow', 'Image1->Picture->LoadFromFile'), L('FormShow', 'Image1->Picture->LoadFromFile'), '示意圖（HTML 自己有）', ';'),
    ('FormShow', L('FormShow', 'Caption=S;'), L('FormShow', 'Top=10;'), 'Caption／Left／Top：視窗標題與位置', ';'),
    ('FormShow', L('FormShow', 'if(ShuttleHasIC() || IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC())'),
     L('FormShow', 'if(ShuttleHasIC() || IndexHasIC() || InArmSuck.HasIC() || OutArmSuck.HasIC())'),
     'InArmSuck／OutArmSuck：同上（FileRW/_KitSuck.cpp 轉接）',
     'if(ShuttleHasIC() || IndexHasIC() || FileRW_InOutArmSuckHasIC())'),
    ('FormShow', L('FormShow', 'fConfiguration->ReadLockByFile();'), L('FormShow', 'fConfiguration->ReadLockByFile();'),
     'fConfiguration->ReadLockByFile()：移植樹唯一的 fConfiguration 是 W5SckArtRem_ConfigStub，沒有 ReadLockByFile（同移植樹 GATE G-SU-LockByFile）',
     'filerw::ELTodo("golden cSetUp.cpp FormShow: fConfiguration->ReadLockByFile() not ported (GATE G-SU-LockByFile) -- IniConfig.bRTC_Enable/bRTC_Active as last read");'),
    # ---- DoIniDataToForm：AI(W906-TIF912) 20260925 —— bDualSiteUseOneSuck／bPreventDropfunction（golden 912 cprod.h:2436-2437）
    #   已加進移植樹 SYSTEM_TEST_IF，移植樹讀檔器 cSetUp.cpp ReadFile 也照 golden :2566-2578 讀這兩鍵 → golden :3357／:3360 原句直接用，
    #   不再用「直接讀檔」的暫代 replace。
    # ---- FormClose
    ('FormClose', L('FormClose', 'Action=caNone;'), L('FormClose', 'Action=caNone;'), 'Action=caNone：VCL 關窗參數（參數已拿掉）→ 記 trace', 'filerw::ELMark("closeRefused");'),
    ('FormClose', L('FormClose', 'rbTemp->SetFocus();'), L('FormClose', 'rbTemp->SetFocus();'), '焦點（HTML 端）', ';'),
    # ---- sbUpdateClick
    ('sbUpdateClick', L('sbUpdateClick', 'Close();'), L('sbUpdateClick', 'Close();'), 'Close()：golden 權限不足時關表單 → 伺服器端記 closed，頁面自己關', 'filerw::ELMark("closed");'),
    ('sbUpdateClick', L('sbUpdateClick', 'fMain->bNeedRestartProgram=true;'), L('sbUpdateClick', 'fMain->bNeedRestartProgram=true;'),
     'fMain->bNeedRestartProgram：TfMain 門面沒有（同移植樹 GATE G-SU-Restart）',
     'filerw::ELTodo("golden cSetUp.cpp sbUpdateClick: fMain->bNeedRestartProgram=true -- TfMain facade has none (GATE G-SU-Restart); restart request dropped");'),
    ('sbUpdateClick', L('sbUpdateClick', 'fBuilder->bSaveAsJobFile('), L('sbUpdateClick', 'fBuilder->bSaveAsJobFile('),
     '#ifdef ASE_KaohSiung：移植樹沒有定義這個巨集，原樣留在 #if 0', ';'),
    ('sbUpdateClick', L('sbUpdateClick', 'fMain->bEnableAutoclean();'), L('sbUpdateClick', 'fMain->bEnableAutoclean();'),
     'fMain->bEnableAutoclean()：TfMain 門面沒有（GATE G-SU-AcBtn，只是顯示 AutoClean 鈕）',
     'filerw::ELTodo("golden cSetUp.cpp sbUpdateClick: fMain->bEnableAutoclean() -- TfMain facade has none (GATE G-SU-AcBtn)");'),
    ('sbUpdateClick', L('sbUpdateClick', 'fContact->DutCount();'), L('sbUpdateClick', 'fContact->DutCount();'),
     '移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單是 fContactForm（forms/fContact.h:1634，DutCount 在 :1522）→ FileRW 轉接',
     'FileRW_Setup_ContactDutCount();'),
    ('sbUpdateClick', L('sbUpdateClick', 'fContact->ReadFile();'), L('sbUpdateClick', 'fContact->ReadFile();'),
     '同上：fContactForm->ReadFile()（forms/fContact.h:1495）→ FileRW 轉接',
     'FileRW_Setup_ContactReadFile();'),
    ('sbUpdateClick', L('sbUpdateClick', 'if(CHSetError())'), L('sbUpdateClick', 'if(CHSetError())'), 'golden bool 方法', 'if(SU_CHSetError())'),
    # ---- SaveSetupFile
    ('SaveSetupFile', L('SaveSetupFile', 'fContact->ReadFile();', 1), L('SaveSetupFile', 'fContact->ReadFile();', 1),
     '移植樹 fContact 是 TfContactShim（沒有 ReadFile）→ fContactForm->ReadFile() 轉接（Test Arm Contact 強制 -50 之後重讀）',
     'FileRW_Setup_ContactReadFile();'),
    ('SaveSetupFile', L('SaveSetupFile', 'fContact->ReadFile();', 2), L('SaveSetupFile', 'fContact->ReadFile();', 2),
     '同上', 'FileRW_Setup_ContactReadFile();'),
    # Steven 團隊 20260925（Cleaning C 路）：golden :4043 fCleaning->LoadAutoCleanData() → FileRW/TestIF_File_Cleaning.cpp（golden TfCleaning
    #   uCleaning.cpp:65 的 C 路版；原本是 blocks 的 GATE G-SU-Clean）。上一行 ReadFile()（＝SU_PortReadFile → fSetup->ReadFile()）
    #   裡 golden :2531-2535 的 Load＋Save 由移植樹 cSetUp.cpp 的 hook 接（IniConfig.bEnableAutoCleanFunction 時）。
    ('SaveSetupFile', L('SaveSetupFile', 'fCleaning->LoadAutoCleanData();'), L('SaveSetupFile', 'fCleaning->LoadAutoCleanData();'),
     'golden TfCleaning::LoadAutoCleanData（uCleaning.cpp:65）→ C 路 FileRW/TestIF_File_Cleaning.cpp（替身沒開機就印出來、不跑）',
     'FileRW_Cleaning_LoadAutoCleanData();'),
    ('SaveSetupFile', _sv_k3[0], _sv_k3[1],
     'CC_ASE_KaohSiung_K3 強制 site 順序：迴圈上限 TestSocket.iShtRow／iShtCol（TMyKitSuck，aHotPlateSubstrate.h 與 HTEditList.h 衝突，本 TU 看不到）'
     '—— 需要 FileRW/_KitSuck.cpp 加轉接 FileRW_TestSocketShtRowCol()（整合者）',
     '{ filerw::ELTodo("golden cSetUp.cpp SaveSetupFile CC_ASE_KaohSiung_K3 forced site order (TestSocket.iShtRow/iShtCol, needs FileRW/_KitSuck.cpp adapter) not ported -- Site Aa..Dh keys NOT written for K3"); }'),
    # ---- 移植樹門面沒有的成員／純畫面
    ('ScrollBar1Change', L('ScrollBar1Change', 'fSetup->Width=818;'), L('ScrollBar1Change', 'fSetup->Width=818;'), '表單寬度（HTML 不用）', ';'),
    ('ScrollBar1Change', L('ScrollBar1Change', 'fSetup->Width=1090;'), L('ScrollBar1Change', 'fSetup->Width=1090;'), '表單寬度（HTML 不用）', ';'),
    ('rgYPitchOffsetModeClick', L('rgYPitchOffsetModeClick', 'labYOffset->Width', 1), L('rgYPitchOffsetModeClick', 'labYOffset->Width', 1), '標籤寬度（HTML 不用）', ';'),
    ('rgYPitchOffsetModeClick', L('rgYPitchOffsetModeClick', 'labYOffset->Width', 2), L('rgYPitchOffsetModeClick', 'labYOffset->Width', 2), '標籤寬度（HTML 不用）', ';'),
    ('rgYPitchOffsetModeClick', L('rgYPitchOffsetModeClick', 'labYOffset->Width', 3), L('rgYPitchOffsetModeClick', 'labYOffset->Width', 3), '標籤寬度（HTML 不用）', ';'),
    ('DoIniDataToForm', L('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;'),
     L('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=IniConfig.bI21EnableASM;'),
     'fMain->cbDisableSiteMappingCheck：TfMain 門面沒有（同移植樹 GATE G-SU-SiteMapChk，主畫面勾選框的顯示）',
     'filerw::ELTodo("golden DoIniDataToForm: fMain->cbDisableSiteMappingCheck->Visible -- TfMain facade has none (GATE G-SU-SiteMapChk)");'),
    ('DoIniDataToForm', L('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=false;'),
     L('DoIniDataToForm', 'fMain->cbDisableSiteMappingCheck->Visible=false;'), '同上',
     'filerw::ELTodo("golden DoIniDataToForm: fMain->cbDisableSiteMappingCheck->Visible=false -- TfMain facade has none (GATE G-SU-SiteMapChk)");'),
    ('FormClose', L('FormClose', 'if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false &&'), L('FormClose', 'if(InArmSuck.HasIC()==false && OutArmSuck.HasIC()==false &&'),
     'InArmSuck／OutArmSuck：FileRW/_KitSuck.cpp 轉接（!(A||B) ≡ A==false && B==false）',
     'if(FileRW_InOutArmSuckHasIC()==false &&'),
    # ---- btnLUpToRDownNClick（Steven 團隊 20260925）：G09 Site Map 密碼框（fPassword＋fQwertyKey）在伺服器端不能彈 →
    #      視同密碼錯誤（bNeedEnterPassword 維持 true → 下一行 golden return，不排序）。bNeedEnterPassword 只在
    #      CC_SIGURD_PeiXing 開頁段設 true（FormShow blocks 已擋，客戶專屬先跳過），所以一般客戶碼走不到這裡。
    ('btnLUpToRDownNClick', L('btnLUpToRDownNClick', 'fPassword->edPassword->Text="";'),
     L('btnLUpToRDownNClick', 'bNeedEnterPassword=false;') + 1,
     'G09 Site Map 密碼框（fPassword／fQwertyKey 是表單，伺服器端沒有）→ 視同密碼錯誤（golden 輸錯：bNeedEnterPassword 維持 true）',
     'if(filerw::ELPasswordRefused("TfSetup::btnLUpToRDownNClick G09 site-map password (golden fPassword/fQwertyKey)")) bNeedEnterPassword=false;'),
]


STRUCT = {
    'struct': 'TestIF_File_SetUp',
    'prefix': 'SU',
    'class': 'TfSetup',
    'cpp': 'cSetUp.cpp',
    'h': 'cSetUp.h',
    'files': ['HandlerCondition.Data', 'Contact.Data（Test Arm1/Arm2 Contact 等）', 'Temperature.Data（[ATC]）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['sbUpdateClick', 'CHSetError', 'CheckShuttlePitch', 'SaveSetupFile'],
    'params': {'TfSetup': '', 'FormShow': '', 'ScrollBar1Change': '', 'chkOffCenterkitClick': '',
               'rgYPitchOffsetModeClick': '', 'FormClose': '', 'sbUpdateClick': '', 'CoSocketComboChange': ''},
    # btnLUpToRDownNClick 保留 golden 的 TObject *Sender：方向＝Sender->Tag（DFM Tag 0..5，FileRW_Setup_Boot 設在六顆鈕的替身上）
    'rettype': {'CHSetError': 'bool', 'CheckShuttlePitch': 'bool'},
    'members': [
        'bool fShow=false;             // golden cSetUp.h:261 —— 本 TU 自己的（見檔頭：不寫 fSetup->fShow）',
        'bool bNeedPassword=false;     // golden cSetUp.h:265（開頁 DoIniDataToForm 設，存檔 sbUpdateClick 讀）',
        'bool bOCRNeedPassword=false;  // golden cSetUp.h:253',
        'bool bShuttleMode=false;      // golden cSetUp.h:255',
        '#define bSavePressed (fSetup->bSavePressed)            // golden cSetUp.h:251 —— 移植樹 ReadFile 讀它',
        '#define bFirstTime (fSetup->bFirstTime)                // golden cSetUp.h:264 —— SECS uHGemHT9045.cpp 寫它',
        'int iTestMode=TotalTestMode;  // golden cSetUp.h:290（建構子 :127）—— 名稱與 TestIF_File.iTestMode 相同，不能 #define；讀檔器前後與 fSetup->iTestMode 同步',
        '#define iTestModeOcr (fSetup->iTestModeOcr)            // golden cSetUp.h:252',
        '#define bSiteMapHasChange (fSetup->bSiteMapHasChange)  // golden cSetUp.h:289',
        '#define shtMode (fSetup->shtMode)                      // golden cSetUp.h:285 —— 移植樹 SetShtMode／VertifyShtModeisDiff 同一份',
        '#define SetShtMode fSetup->SetShtMode                  // golden :4741 —— 移植樹 forms/fSetup.cpp（純成員寫入，逐行相同）',
        '#define VertifyShtModeisDiff fSetup->VertifyShtModeisDiff   // golden :4750（同上）',
        '#define GetTestMode fSetup->GetTestMode                // golden :2187（純查表）',
        '#define CheckSTMMode fSetup->CheckSTMMode              // golden :2156（移植樹 cSetUp.cpp:358）',
        '#define ReadUseSuckModeFile fSetup->ReadUseSuckModeFile   // golden :2141（移植樹 cSetUp.cpp:336）',
        # golden cSetUp.cpp 檔案層級（:49-57、:121）—— 本表單私用。移植樹 cSetUp.cpp:1333 另有一份（外部連結，給移植樹 fSetup 的真元件），
        # 這裡是 static，裝具名替身。
        'TComboBox   *TestSiteCH[MAX_SOCKET_ROW][MAX_SOCKET_COL];   // golden cSetUp.cpp:49',
        'TLabel      *TestLabCol[MAX_SOCKET_COL];                   // golden cSetUp.cpp:50',
        'TLabel      *TestLabRow[MAX_SOCKET_ROW];                   // golden cSetUp.cpp:51',
        'int  iTestSiteCh[MAX_SOCKET_ROW][MAX_SOCKET_COL];          // golden cSetUp.cpp:52',
        'int iASMTestMode;                                          // golden cSetUp.cpp:53',
        'int iASMSiteMap[MAX_SOCKET_ROW][MAX_SOCKET_COL];           // golden cSetUp.cpp:54',
        'bool bNeedEnterPassword=false;                             // golden cSetUp.cpp:55（移植樹 cSetUp.cpp:2046 另有 static 一份）',
        'AnsiString sSigPassword;                                   // golden cSetUp.cpp:56',
        'TRadioGroup *MyTempRGBox[iSnSocketCnt];                    // golden cSetUp.cpp:57',
        'int OrgTestMode=0;                                         // golden cSetUp.cpp:121',
        # 讀檔器轉接（見檔頭）
        'void SU_PortReadFile() { bool f=fSetup->fShow; fSetup->fShow=fShow; '
        'fSetup->ScrollBar1->Position=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position; '
        'fSetup->rgInOutArmYPitch->ItemIndex=filerw::EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex; '
        'fSetup->iTestMode=iTestMode; fSetup->ReadFile(); iTestMode=fSetup->iTestMode; fSetup->fShow=f; }   // golden ReadFile() → 移植樹 fSetup->ReadFile()（cSetUp.cpp:416）',
        '#define ReadFile SU_PortReadFile   // FileRW/TestIF_File_SetUp.cpp 在 include forms/fContact.h 之前 #undef',
        # VCL TCustomComboBox（Style=csDropDownList）語意（見 _CB_SET 說明）；程式設 ItemIndex 不觸發 OnChange（同 VCL）
        'void SU_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }   // VCL Clear()：CB_RESETCONTENT',
        'void SU_CbSetIndex(TComboBox* cb, int i) { if(i>=0 && i<cb->Items->Count) { cb->ItemIndex=i; cb->Text=cb->Items->Strings[i]; } '
        'else { cb->ItemIndex=-1; cb->Text=""; } }   // VCL SetItemIndex：CB_SETCURSEL（超出範圍 → 沒有選取）',
        '#define IncludeTrailingPathDelimiter IncludeTrailingBackslash   // vclcompat 只有後者（同義；Public/HTEditList.cpp:643 同做法）',
    ],
    'replace': MANUAL + _auto(),
    'blocks': [
        ('FormShow', _fs_sig[0], _fs_sig[1] + 2,
         'CC_SIGURD_PeiXing：在 C:\\Windows 建立／讀 SitMap.ini（site map 密碼）—— 開頁不可以寫系統資料夾，不做'),
        ('SaveSetupFile', L('SaveSetupFile', '"RTC", "Enable"'), L('SaveSetupFile', '"RTC", "Enable"'),
         'IniConfig.bRTCbySystem：直寫 config.ini [RTC] Enable —— config.ini 由 FileRW/IniConfig 擁有，這裡不寫'),
        ('SaveSetupFile', L('SaveSetupFile', 'myLog.Save_SiteStatusLog();'), L('SaveSetupFile', 'myLog.Save_SiteStatusLog();'),
         'myLog.Save_SiteStatusLog()：移植樹 handlerlog 沒有這支（handlerlog.h:16-20），且 handlerlog.h 與 language.h 衝突'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fSetup.h', 'csystem.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h', 'Motor/mymotor.h', 'mysensor.h', 'atester_shims.h',
                 'BarcodeReader.h', 'cMyDB.h', 'cinitial.h', 'ainarm9045.h', 'ATC/ATCInterface.h'],
    'decls': ['void GetSetupAuth();                      // cAuthority.h:80（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）',
              'extern int  iSetupSiteMapping;           // cAuthority.h:68',
              'extern void SetRunStartMode(eRunStartMode Mode = rsmNull, AnsiString ModeText = "");   // aHotPlateSubstrate.h:943','bool FileRW_InOutArmSuckHasIC();         // FileRW/_KitSuck.cpp（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
              'void FileRW_Setup_ContactDutCount();     // FileRW/TestIF_File_SetUp.cpp 下半（fContactForm->DutCount）',
              'void FileRW_Setup_ContactReadFile();     // FileRW/TestIF_File_SetUp.cpp 下半（fContactForm->ReadFile）',
              'void FileRW_Cleaning_LoadAutoCleanData(); // FileRW/TestIF_File_Cleaning.cpp（golden fCleaning->LoadAutoCleanData，cSetUp.cpp:4043）',
              'extern bool authSetup[8];                // cAuthority.h:66（golden cAuthority.h）',
              'extern bool bAuthCriticalPara[26];       // cAuthority.h:64（golden cAuthority.h）'],
    'overrides': [],
}
