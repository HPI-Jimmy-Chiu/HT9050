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

G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
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
           'CoSocketComboChange', 'btnLUpToRDownNClick', 'rgShtModeNormalClick', 'rgUseSuckModeClick', 'Arm1PickArm2TestClick', 'cbUseSLKClampClick']   # AI(W906-Q41) 20260927 (St02-E): SU-1..4 golden DFM OnClick（912 cSetUp.cpp:1272／:3447／:4497／:4670），BeforeApply 重播
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
    # AI(W906-FRW-S158) 20260927 [W906] DoPassword() → SU_DoPassword()（members 裡 golden cSetUp.cpp:4325 DoPassword 的轉寫）。
    #   偏離前的做法比 golden 嚴：一律換成 filerw::ELPasswordRefused（視同密碼錯），沒裝 RTC 的機台在 SetUp 頁也關不掉 OCR；
    #   照 golden（Steven 沒反對，decisions R75）：沒裝 REAL_TIME_CCD 直接過，有裝才視同密碼錯（網頁還沒有登入框，C-3／Q45）。
    #   AI(W906-Q45-B5) 20260930：Q45 甲接上 —— 有裝且這次存檔帶了重新登入（editlist.save 的 reauth）→ golden DoPassword 照跑
    #   （members 的 SU_DoPassword → WebLogin.cpp 檔尾 W906_ReauthSetupDoPassword）；沒帶才視同密碼錯。_auto() 說明字串裡的
    #   「有裝視同密碼錯」沒跟著改（改了產生檔 98 行只動註解），以 members 那一條的說明為準。
    code = re.sub(r'(?<![\w.>:])DoPassword\(\)', 'SU_DoPassword()', code)
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
                        '、tSiteMap（移植樹 fSetup 那一份，SECS EC 3540 指到它）、事件以 (this) 呼叫、DoPassword 密碼框（→ SU_DoPassword：沒裝 REAL_TIME_CCD 照 golden 直接過，有裝視同密碼錯；R75）'
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


# ---- //AI(W906-EVB10A) 20260929 [W906]：SU-9（事件批次 B10 part a，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
#   Steven 20260928「任何畫面的事件, 都是我們做」、20260929「照 BCB 的邏輯」）—— Exit 鈕 sbtExit（cSetUp.dfm:1319）→ golden TfSetup::sbtExitClick
#   （cSetUp.cpp:3476-3490）：AUTO_SENSOR_INSTALL && bSaveNeedHome（改了 shuttle pitch 還沒重新回原點，sbUpdateClick :3128 設）⇒
#   ShowMyMessage("Auto Shuttle Sensor Need Reset!!") 而且**不關視窗**（return）；否則 OCR lot、bNeedEnterPassword=true、Close()（→ FormClose
#   :3363：ReadFile、fShow=false、Auto Site Map 關掉時清 HotPlate site map＋SetRunStartMode(rsmContinuStart)）、fMain->GetCZSiteMap(false)
#   （bSendGPIB=false：只重算 aSendSiteMapping，GPIB 下次問 site map 時回它；移植樹 Command.cpp:3026）。
#   WS form.event（頁面 D:\HT9045\web\page\ht9045_setup_c_wire.js 攔 sbtExit）；ack.closed＝true 頁面才關視窗（golden return 時視窗留著）。
#   只加在 STRUCT 的 methods 尾端（不進 METHODS：_auto() 與 _conv() 的規則照舊，既有方法的產生碼不動）。
SPANS['sbtExitClick'] = _span('sbtExitClick')


def _expect(gl, text):
    if _cpp[gl - 1].strip() != text:
        raise SystemExit('TestIF_File_SetUp.py: golden cSetUp.cpp:%d is %r, expected %r' % (gl, _cpp[gl - 1].strip(), text))
    return gl


_expect(SPANS['sbtExitClick'][0], 'void __fastcall TfSetup::sbtExitClick(TObject *Sender)')
_expect(SPANS['sbtExitClick'][0] + 2, 'if(AUTO_SENSOR_INSTALL && bSaveNeedHome)')
_EX_OCR = _expect(L('sbtExitClick', 'fOCR->sTesterLotId'), 'fOCR->sTesterLotId=edOcrText->Text;')
_EX_DOWN = _expect(L('sbtExitClick', 'sbtExit->Down'), 'sbtExit->Down=false;')
_EX_CLOSE = _expect(L('sbtExitClick', 'Close();'), 'Close();')
_EXIT_REPLACE = [
    ('sbtExitClick', _EX_OCR, _EX_OCR,
     'fOCR->sTesterLotId（OCR 的 tester lot id）：移植樹 TfOCR 門面（forms/fOCR.h）沒有這個成員（同 forms/fLotInfo.h:532 記的缺口）→ 記 todo；'
     'if(bGetLotIDFormTester==false) 的本體換成它',
     'filerw::ELTodo("golden cSetUp.cpp:%d fOCR->sTesterLotId=edOcrText->Text not done -- the port TfOCR facade (forms/fOCR.h) has no sTesterLotId");' % _EX_OCR),
    ('sbtExitClick', _EX_DOWN, _EX_DOWN, 'sbtExit->Down=false：按鈕放開（純畫面）', ';'),
    ('sbtExitClick', _EX_CLOSE, _EX_CLOSE,
     'Close()：VCL 同步觸發 OnClose＝FormClose（:3363）→ 伺服器端記 closed（ack.closed，頁面關視窗）再照 golden 跑 FormClose',
     'filerw::ELMark("closed"); SU_FormClose();'),
]


# ---- //AI(W906-B8-SU7) 20260930 [W906]：B8 SU-7（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「SU-7」；
#   Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）—— RTC 等 6 個勾選框的點擊檢查與改回。
#   golden V912 cSetUp.cpp:4364-4399 TfSetup::cbEnableRealTimeCCDClick；cSetUp.dfm 有 6 個元件的 OnClick 綁這一支（:1052 cbOutUseBackRow、
#   :1082 cbInUseBackRow、:1121 cbEnableRealTimeCCD、:1559 cbOcrFunction、:1654 cbdisibleinitialcheck、:1669 cbSocketSensor）——
#   不管點哪一個，處理器都只看 cbEnableRealTimeCCD（golden 如此，照翻；例：點 OCR 勾選框也會跳 RTC 的提示字）。
#     (a) :4366-4375 取消 RTC、[D55] 開著且 [M01-09] 沒開 ⇒ 改回勾選、ShowMyMessage、return
#     (b) :4377-4388 RTC 正在跑（!bCCDDummyRum）：IndexStatus!=Z1_Z2_Normal ⇒ 提示字＋改回勾選；否則送 vision rtInspEnd
#     (c) :4390-4397 RTC 關著：fMain->CheckCanChangeRealDummy()==false（機台裡有料）⇒ 改回不勾＋提示字
#   fMain->CheckCanChangeRealDummy() 原樣：forms/fMain.h:1209 的 TfMain 成員，本體 cMainStatus.cpp:323（jimmychiu MSTATE-P2，golden main.cpp
#     :12895-12901 逐行，查 MOT[MMPlate1／2].HasIC()、ShuttleHasIC()、IndexHasIC()、InArmSuck／OutArmSuck.HasIC()，都是 csystem.cpp 的活本體；
#     csystem_predicates.cpp 那一份全 #if 0 退場）。⚠ 不是 Automation/auto9045.cpp:207-216 的「先回 true」替身（那是 W5FA_TfMainExt 的 TU 私有成員，
#     這個 TU 叫不到）。
#   VCL「程式設 Checked 值有變也觸發 OnClick」（B8 P-8、R118）：處理器裡三行 cbEnableRealTimeCCD->Checked=… 改寫成 filerw::ELClickChecked
#     （值有變才再進一次處理器；OnClick 登記與遞迴上限在 FileRW/TestIF_File_SetUp.cpp 檔尾 FileRW_Setup_B8Su7Boot）。只改這一支裡的三行：
#     整個結構開 'vcl_clicks' 會連 FormShow／DoIniDataToForm／sbUpdateClick 的設值一起改（Q41 SU-1～6 的開頁也會多跑事件），不在這一列。
#   :4387 COM2->SendCommToVision(COM2->rtInspEnd,false)：RTC 視覺序列埠通訊沒有移植（COM2 是 TCOM2Shim，atester_shims.h:373，沒有
#     SendCommToVision／rtInspEnd；B8 P-4）⇒ 換成 ELTodo（ack.todo 說明「沒有通知 vision」）；B8 批次表寫明這一半不在這次。
#   存檔時再查一次（網頁沒送 form.event、直接存檔時）：FileRW/TestIF_File_SetUp.cpp BeforeApply → 檔尾 FileRW_Setup_B8Su7BeforeApply。
SPANS['cbEnableRealTimeCCDClick'] = _span('cbEnableRealTimeCCDClick')


def _expect_code(gl, code):
    if _cpp[gl - 1].split('//')[0].strip() != code:
        raise SystemExit('TestIF_File_SetUp.py: golden cSetUp.cpp:%d code is %r, expected %r' % (gl, _cpp[gl - 1].split('//')[0].strip(), code))
    return gl


_S7 = SPANS['cbEnableRealTimeCCDClick']
_expect(_S7[0], 'void __fastcall TfSetup::cbEnableRealTimeCCDClick(TObject *Sender)')
_expect_code(_S7[0] + 2, 'if(cbEnableRealTimeCCD->Checked==false)')
_expect(_S7[0] + 4, 'if(IniConfig.bD55DisableIndexCheck==true &&')
_expect_code(_S7[0] + 5, 'IniConfig.bM0109RTCOffCheckYieldPiggyBack==false)')
_expect_code(_S7[0] + 13, 'if(COM2->bCCDDummyRum==false)')
_expect_code(_S7[0] + 15, 'if(IndexStatus!=Z1_Z2_Normal)')
_expect_code(_S7[0] + 28, 'if(fMain->CheckCanChangeRealDummy()==false)')
_expect(_S7[1], '}')
_S7_D55 = _expect(L('cbEnableRealTimeCCDClick', 'cbEnableRealTimeCCD->Checked=true;', 1), 'cbEnableRealTimeCCD->Checked=true;')   # :4371
_S7_MSG = _expect(_S7_D55 + 1, 'ShowMyMessage("Must cancel \\"[D55]Disable index check\\" first "," ");')                     # :4372（產生器轉 ELMessage）
_S7_Z12 = _expect(L('cbEnableRealTimeCCDClick', 'cbEnableRealTimeCCD->Checked=true;', 2), 'cbEnableRealTimeCCD->Checked=true;')   # :4383
_S7_END = _expect_code(L('cbEnableRealTimeCCDClick', 'SendCommToVision'), 'COM2->SendCommToVision(COM2->rtInspEnd, false);')      # :4387
_S7_OFF = _expect(L('cbEnableRealTimeCCDClick', 'cbEnableRealTimeCCD->Checked=false;'), 'cbEnableRealTimeCCD->Checked=false;')     # :4394
_S7_CLICK = 'VCL：程式設 Checked 值有變 ⇒ 再觸發一次 OnClick＝本處理器（B8 P-8／R118；登記與遞迴上限在 FileRW/TestIF_File_SetUp.cpp 檔尾）'
_SU7_REPLACE = [
    ('cbEnableRealTimeCCDClick', _S7_D55, _S7_D55, _S7_CLICK,
     'filerw::ELClickChecked(EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD"), true);'),
    ('cbEnableRealTimeCCDClick', _S7_Z12, _S7_Z12, _S7_CLICK,
     'filerw::ELClickChecked(EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD"), true);'),
    ('cbEnableRealTimeCCDClick', _S7_END, _S7_END,
     'COM2->SendCommToVision(COM2->rtInspEnd,false)（golden rs232.cpp:3739-3751，InitialOK 才經 Comm4 序列埠送）：移植樹 COM2 是 TCOM2Shim'
     '（atester_shims.h:373），RTC 視覺通訊沒有移植（B8 P-4）⇒ 記 todo，不送',
     'filerw::ELTodo("golden cSetUp.cpp:%d COM2->SendCommToVision(COM2->rtInspEnd, false) NOT sent -- the RTC vision serial link is not ported '
     '(COM2 is TCOM2Shim, atester_shims.h:373; B8 P-4): the vision is not told to end inspection");' % _S7_END),
    ('cbEnableRealTimeCCDClick', _S7_OFF, _S7_OFF, _S7_CLICK,
     'filerw::ELClickChecked(EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD"), false);'),
]
# golden DFM 順序（cSetUp.dfm:1052／:1082／:1121／:1559／:1654／:1669）；FileRW/TestIF_File_SetUp.cpp 檔尾 kSu7Boxes 同一份
_SU7_BOXES = ['cbOutUseBackRow', 'cbInUseBackRow', 'cbEnableRealTimeCCD', 'cbOcrFunction', 'cbdisibleinitialcheck', 'cbSocketSensor']
_SU7_EVENTS = [(n, 'click', 'cbEnableRealTimeCCDClick') for n in _SU7_BOXES]


STRUCT = {
    'struct': 'TestIF_File_SetUp',
    'prefix': 'SU',
    'class': 'TfSetup',
    'cpp': 'cSetUp.cpp',
    'h': 'cSetUp.h',
    'files': ['HandlerCondition.Data', 'Contact.Data（Test Arm1/Arm2 Contact 等）', 'Temperature.Data（[ATC]）'],
    'lists': [],
    'methods': METHODS + ['sbtExitClick'] + ['cbEnableRealTimeCCDClick'],   # //AI(W906-EVB10A) 20260929 [W906]：SU-9 加在最後  # //AI(W906-B8-SU7) 20260930 [W906]：SU-7 golden cbEnableRealTimeCCDClick（:4364）再加在最後
    'save_methods': ['sbUpdateClick', 'CHSetError', 'CheckShuttlePitch', 'SaveSetupFile'],
    'params': {'TfSetup': '', 'FormShow': '', 'ScrollBar1Change': '', 'chkOffCenterkitClick': '', 'rgShtModeNormalClick': '', 'rgUseSuckModeClick': '', 'Arm1PickArm2TestClick': '', 'cbUseSLKClampClick': '',   # AI(W906-Q41) 20260927
               'rgYPitchOffsetModeClick': '', 'FormClose': '', 'sbUpdateClick': '', 'CoSocketComboChange': '',
               'sbtExitClick': '', 'cbEnableRealTimeCCDClick': ''},   # //AI(W906-EVB10A) 20260929 [W906]：SU-9，golden 不讀 Sender（合併 main 的 Q41 四支之後）  # //AI(W906-B8-SU7) 20260930 [W906]：SU-7 golden 也不讀 Sender（6 個勾選框共用、只看 cbEnableRealTimeCCD）
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
        # AI(W906-FRW-S158) 20260927 [W906] golden TfSetup::DoPassword（cSetUp.cpp:4325-4362）的轉寫（R75）。
        # AI(W906-Q45-B5) 20260930：接上 Q45 甲的重新登入（本體 WebLogin.cpp 檔尾；函式在這一行裡宣告，產生檔的 include 不用多一行）。
        'bool SU_DoPassword() { bool W906_ReauthSetupDoPassword(bool, bool, bool, bool, bool*); bool bHandled=false; '
        'const bool bFlag=W906_ReauthSetupDoPassword(bNeedPassword, bOCRNeedPassword, filerw::EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked, '
        'filerw::EL<TCheckBox>("TfSetup", "cbOcrFunction")->Checked, &bHandled); '
        'return bHandled ? bFlag : filerw::ELPasswordRefused("TfSetup::DoPassword (golden password form)"); }   '
        '//AI(W906-Q45-B5) 20260930 golden cSetUp.cpp:4325-4362 DoPassword → WebLogin.cpp 檔尾 W906_ReauthSetupDoPassword（W906_Reauth）：'
        'bFlag=true；bTechComExist=FileExists(pwPath)；iLevel=LevelSet.AccessLevel[37]（SetUp 版沒有「＝0 就不問」的捷徑）；只有 if(REAL_TIME_CCD)'
        '（移植樹 database.cpp:750 讀 Gerneral.ini [System] REAL_TIME_CCD）才重新登入：有密碼本 → cbUserSelectChange 密碼本分支，'
        '沒有 → stOperatorClick（bNeedPassword 時走 SetUp 那一臂 main.cpp:13283-13301）；AccessLevel<iLevel → false；SetUp 版不登出。'
        '答案＝這次 editlist.save 的 reauth（取消＝空白帳密＝錯，Q45-4）；golden 會問、這次卻沒帶答案（bHandled=false）→ 照舊視同密碼錯（R75）。'
        '後兩個勾選值只用來回報 reverted（golden :3578-3581 的改回由下面照翻的 sbUpdateClick 做）。沒裝 REAL_TIME_CCD → 照 golden 直接回 true',
        # 讀檔器轉接（見檔頭）
        'void SU_PortReadFile() { bool f=fSetup->fShow; fSetup->fShow=fShow; '
        'fSetup->ScrollBar1->Position=filerw::EL<filerw::ELTrackBar>("TfSetup", "ScrollBar1")->Position; '
        'fSetup->rgInOutArmYPitch->ItemIndex=filerw::EL<TRadioGroup>("TfSetup", "rgInOutArmYPitch")->ItemIndex; '
        'fSetup->iTestMode=iTestMode; fSetup->ReadFile(); iTestMode=fSetup->iTestMode; fSetup->fShow=f; }   // golden ReadFile() → 移植樹 fSetup->ReadFile()（cSetUp.cpp:416）；AI(W906-FSHOW-B2) 20260929：讀 fSetup->fShow 是存／還原成員，不問頁面表（問了還原時會把網頁答案寫進成員）',
        '#define ReadFile SU_PortReadFile   // FileRW/TestIF_File_SetUp.cpp 在 include forms/fContact.h 之前 #undef',
        # VCL TCustomComboBox（Style=csDropDownList）語意（見 _CB_SET 說明）；程式設 ItemIndex 不觸發 OnChange（同 VCL）
        'void SU_CbClear(TComboBox* cb) { cb->Clear(); cb->ItemIndex=-1; cb->Text=""; }   // VCL Clear()：CB_RESETCONTENT',
        'void SU_CbSetIndex(TComboBox* cb, int i) { if(i>=0 && i<cb->Items->Count) { cb->ItemIndex=i; cb->Text=cb->Items->Strings[i]; } '
        'else { cb->ItemIndex=-1; cb->Text=""; } }   // VCL SetItemIndex：CB_SETCURSEL（超出範圍 → 沒有選取）',
        '#define IncludeTrailingPathDelimiter IncludeTrailingBackslash   // vclcompat 只有後者（同義；Public/HTEditList.cpp:643 同做法）',
    ],
    'replace': MANUAL + _auto() + _EXIT_REPLACE + _SU7_REPLACE,   # //AI(W906-EVB10A) 20260929 [W906]：SU-9 sbtExitClick 三條（見上）  # //AI(W906-B8-SU7) 20260930 [W906]：SU-7 四條（見上）
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
    # //AI(W906-EVB10A) 20260929 [W906]：SU-9 WS form.event（FileRW/TestIF_File_SetUp.cpp 檔尾註冊；頁面 ht9045_setup_c_wire.js 攔 Exit 鈕）
    'events': [('sbtExit', 'click', 'sbtExitClick')] + _SU7_EVENTS,   # //AI(W906-B8-SU7) 20260930 [W906]：SU-7 6 個勾選框（見上 _SU7_EVENTS；頁面 ht9045_setup_c_wire.js (8)）
}

# AI(W906-E030-Q78) 20261003 [W906] (St01)：Steven 1003 05:3x Q78 裁決「Q78 Q79, 可以按照912，但是註解同時提供906的行號位置」，
#   加上 05:4x 慣例「如果是912比較好，就是註記906的行號跟做法　然後增加註記912已修正或更新的行號」⇒ A02 分支
#   （RogerYang 20260305 [A01_2]，906／V912 都有）裡 Close() 後面那句 `return;` 照 V912 留著，記成第 20 條的例外
#   （D:\HT9045\.claude\skills\ht9050-construction\references\decisions-decided.md Q78；human-review C24）。
#   下面這一列是「等價取代」：取代碼＝golden 原文 `return;`，產生的程式不變，只是讓產生檔那一行帶 906／V912 對照註解
#   （V912 原文留在 #if 0 // GATE 裡）。_e030q78_expect 釘住 V912 的 if／Close()／return 三行：V912 變了，產生器停下來。
#   906＝D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003]（cp950，不在 git，唯讀）；行號＝grep -n／iconv 行號，
#   V912＝產生器的 golden 根目錄（tools/gen_editlist.py GOLDEN）。不改既有的列。
import os   # noqa: E402
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cSetUp.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 cSetUp.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('TestIF_File_SetUp.py (E030-Q78): golden V912 cSetUp.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(3494, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(3498, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 cSetUp.cpp:3470-3475：A02 分支只有 Close()（:3474），後面沒有 return，存檔照跑；主選單 main.cpp:27481 用 ShowModal 開（對話框），Close() 只設 ModalResult ⇒ 操作員改的值照樣寫進檔。'
                'V912 cSetUp.cpp:3498-3499：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('sbUpdateClick', _e030q78_expect(3499, 'return;'), 3499, _E030Q78_WHY, 'return;'))
# AI(W906-E032) 20261003 [W906] (St01)：todo E-032（golden 改回 906 0618，Jimmy RULINGS_20261003 第 2 條）——0618 與 0625 不同的
#   函式 TfSetup::FormShow（D:\AI_TempFile\st01e-e032\diff_0618_0625.tsv）只差一行，產生檔照產生器根目錄 V912 出的是 0625／V912 那一版。
#   RULINGS_20261003 第 1 條（Steven 1003 常設規則：較新的版本是修正／明顯比較好就留，註解三段）⇒ 留 V912，這一列是「等價取代」：
#   取代碼＝產生器原本的輸出，產生的程式不變，只讓產生檔那一行帶三段註解（V912 原文進 #if 0 // GATE）。_e032_expect 釘住 V912 原文。
#   E-031（產生器根目錄換 906 0618）時這一列改成釘 0618 的 `==` 那一行、取代碼不變（＝照舊留 V912 的 `=`）。
_E032_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cSetUp.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e032_expect(gl, text):
    """golden V912 cSetUp.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E032_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('TestIF_File_SetUp.py (E032): golden V912 cSetUp.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e032_expect(2125, 'if(IniConfig.bRTC_Enable==false)')
_E032_RTC_WHY = ('AI(W906-E032) 原文照留（取代碼＝產生器原本的輸出，產生的程式不變，只加這段註解）。'
                 '(1) 906 0618 cSetUp.cpp:2118 `cbEnableRealTimeCCD->Checked==IniConfig.bRTC_Active;` 是比較不是指定（code has no effect）'
                 '⇒ [RTC Lock by file] 鎖住（bLockRTCByFile、bRTC_Enable==false）時勾選框留著配方讀到的值，存檔（0618 :3724）把它寫回 config.ini [RTC] Enable。'
                 '(2) 0625 cSetUp.cpp:2118／V912 :2126 改成 `=`（Steven 0625 的 == 誤寫清理，V912 沿用）⇒ 勾選框跟 lock file 的 RTC_Active 一致；留這一版＝修正。'
                 '(3) #20 exception (Steven 1003 standing rule, RULINGS_20261003 #1)')
STRUCT.setdefault('replace', []).append(
    ('FormShow', _e032_expect(2126, 'cbEnableRealTimeCCD->Checked=IniConfig.bRTC_Active;'), 2126, _E032_RTC_WHY,
     'EL<TCheckBox>("TfSetup", "cbEnableRealTimeCCD")->Checked=IniConfig.bRTC_Active;'))
