# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_Cleaning.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_Cleaning
#
# Steven 團隊 20260925：golden TfCleaning（AutoClean\uCleaning.cpp，912，2957 行）—— web/page/Setup.Cleaning.html。
#   讀：LoadAutoCleanData（:65）＝ <recipe>\HandlerCondition.Data [Configuration]／[AutoClean]（客戶／CosFunction 條件下
#       另讀 IniData\DefineAutoClean\AutoClean.Data、HotPlate.Data、Tray.Data）→ TestIF_File.*AutoClean* → 元件。
#   寫：SaveAutoCleanData（:836）＝ 元件 → TestIF_File → 同一個 HandlerCondition.Data（bUseDefineAutoCleanOffset 時
#       高度／offset 那段寫 DefineAutoClean\AutoClean.Data）。
# 結構名 TestIF_File_Cleaning：TestIF_File 已被 A 形狀 FileRW/TestIF_File.cpp 佔用（同 TestIF_File_SetUp 的理由）。
#
# 開頁＝golden FormShow（:1465，內含 fCleaning->LoadAutoCleanData() :1584）；
# 存檔＝golden sbCleanSaveClick（:1778：A02 → Tray 格數檢查 → Fix3 有 Bin 檢查 → SaveAutoCleanData → SearchCleanNum →
#       InitialSet → … → LoadAutoCleanData → BackupSetupFile）；
# Exit＝golden sbCleanExitClick（:1886 Close()）→ modal 迴圈跑 FormClose（:2106，SetWorkParameter）→ ShowModal 回傳後
#       TfMain::sbAutoCleanClick 的 DoStructUnitConvert()（main.cpp:29673）。（FileRW/TestIF_File_Cleaning.cpp SaveFlow）
#
# 本檔的自動 replace（_auto）處理 VCL 自動行為與 vclcompat 沒有的東西（每一條都列在產生檔的「取代」註解）：
#   (1) VCL TEdit.OnChange：程式設 Text、值有變就觸發（TControl.SetText 比較後 → CMTextChanged／EN_CHANGE → Change）。
#       XCT1／XCT2（OnChange=XCT1Change）、YCT1／YCT2（YCT1Change）、XPitch2（XPitch2Change）→ CL_EditText。
#       golden LoadAutoCleanData :366-368 就是靠它（XCT2->Text= 之後 XCT1Change 夾值，再讀回 atoi(XCT2->Text)）。
#   (2) VCL TRadioGroup：程式設 ItemIndex、值有變就觸發 OnClick（TCustomRadioGroup.SetItemIndex → TGroupButton.Checked
#       → Click → ButtonClick → Click）。rgCleanKitType（rgCleanKitTypeClick）、rgKitPosition（rgKitPositionClick）→ CL_RadioIndex。
#       rgAutoCleanOnOff 的 OnClick（rgAutoCleanOnOffClick :2292）只做 KYEC 刷條碼（Barcode_Reader 非 KYEC 回 2），客戶專屬 → 不接。
#   (3) golden `udDeviceCT->Min>=edDevicePices->Text`（:397／:407／:910）：TUpDown::Min 是 SmallInt、Text 是 AnsiString；
#       BCB AnsiString 只有成員比較運算子（dstring.h:94-99），實際選到的是 sysvari.h:3113
#       `operator>=(int lhs, const Variant& rhs)`（AnsiString → Variant）＝ **數值**比較。vclcompat 的非成員
#       operator>=(AnsiString, AnsiString)（AnsiString.h:234）會把 int 轉字串、變成字典序比較（"8">="16" 為真）→ CL_VarGE。
#   (4) filerw::ELTrackBar 的 Min／Max／Position 是代理物件：讀值（含 sprintf 可變參數）一律 (int) 轉型。
#   (5) golden 小鍵盤 fQwertyKey->ShowQwertyKey（myQwertyKeyBoard.cpp:169-301）→ CL_QwertyKey：頁面值＝使用者按確定時
#       鍵盤上的內容，這裡只做 golden 關鍵盤後的數值夾限（:283-290）與寫回元件（:294-301，TEdit 寫回照 (1) 觸發 OnChange）。
#       移植樹 fQwertyKey 只在 Public/HTEdit.cpp:311 懶建立，wb_serve 沒保證建立 → 不呼叫它。
#   (6) TControl::Hint（vclcompat 沒有）：只給 RPDefault（DoSetRPDefault／SearchRecipeParameter）用 → 空敘述。
import os
import re

G = r'D:\HT9045_ref\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
F = 'TfCleaning'
_cpp = open(os.path.join(G, 'AutoClean', 'uCleaning.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_h = open(os.path.join(G, 'AutoClean', 'uCleaning.h'), 'rb').read().decode('cp950', errors='replace')
_dfm = open(os.path.join(G, 'AutoClean', 'uCleaning.dfm'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n')

_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_MAPPED = {'TTrackBar': 'filerw::ELTrackBar', 'TUpDown': 'filerw::ELTrackBar', 'TDateTimePicker': 'filerw::ELDateTimePicker',
           'TStringGrid': 'filerw::ELStringGrid'}
_W = {}
for _m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _h[re.search(r'class\s+TfCleaning\b[^;{]*\{', _h).end():], re.M):
    _t = _m.group(1)
    _W[_m.group(2)] = _MAPPED.get(_t) or (_t if _t in _KNOWN else 'TControl')

# golden 小鍵盤處理器（DFM OnClick／OnMouseDown → 本體呼叫 fQwertyKey->ShowQwertyKey）。edPinSingleGf／edPinSingleN／
# edPinsCount 的處理器在鍵盤之後還重算力量（ShowCleanContactForce／ShowTranGfToN／ShowTranNToGf）。
KB = ['edPinSingleGfClick', 'edPinSingleNClick', 'edPinsCountClick', 'edCleanCountClick', 'edtBufferKitLTXClick',
      'edtHotplatePickOffsetClick', 'edACContactCleanHeightClick', 'edtLowYieldLimitClick', 'edtLowYieldCountClick',
      'XST1Click', 'XCT1Click', 'edACContactShiftHeightClick', 'edDropOffset1Click', 'edDevicePicesClick',
      'edAutoCleanAirForce_KgClick', 'edAutoCleanAirForce_NClick', 'edtACSmartClick', 'edTimeCTClick', 'edtIndexVacuumClick',
      'edAdaptiveIntervalMaxMouseDown', 'edAdaptiveIntervalMinMouseDown', 'edAdaptiveIntervalAdjMouseDown',
      'edACSmartCTFMouseDown', 'edtACSmart_ContactTimeMouseDown', 'edtACSmartMouseDown']
ONCHANGE = {'XCT1': 'XCT1Change', 'XCT2': 'XCT1Change', 'YCT1': 'YCT1Change', 'YCT2': 'YCT1Change', 'XPitch2': 'XPitch2Change'}
ONCLICK_RG = {'rgCleanKitType': 'rgCleanKitTypeClick', 'rgKitPosition': 'rgKitPositionClick'}

METHODS = ['TfCleaning', 'SetArmCaption', 'LoadAutoCleanData', 'SaveAutoCleanData', 'LoadImage', 'GetMinCleanPadCount',
           'SetDeviceMaxMin', 'FormShow', 'ShowTranGfToN', 'ShowTranNToGf', 'ShowCleanContactForce', 'sbCleanSaveClick',
           'sbCleanExitClick', 'rgKitPositionClick', 'YCT1Change', 'DrawAutoClean', 'FormClose', 'XCT1Change',
           'rgCleanKitTypeClick', 'XPitch2Change', 'ChangeEditToHPMode', 'cbbCleanPadCountChange'] + KB
# 不轉（頁面停用並註明，見 FileRW/TestIF_File_Cleaning.cpp 檔頭）：udDeviceCTClick／udDeviceCTChangingEx（頁面沒有 TUpDown）、
#   btIncludeClick（要伺服器當下的 TrayForm.Loader）、btnResetCleanCountClick（清潔計數歸零＋SECS 事件）、btnResetIntervalClick
#   （ChangeACSmartInterval 直寫 HandlerCondition.Data）、btnStartAutoCleanClick（啟動 Auto Clean 動作）、sbTrayAssignClick
#   （fMain 主畫面）、cbbSelectTrayChange（Tray CSV，ASE 高雄才顯示）、rgAutoCleanOnOffClick／chkAutoCleanMode6MouseUp（KYEC 條碼）、
#   DoSetRPDefault／DoReplyDefaultToForm／SearchRecipeParameter（RPDefault 走訪表單元件樹，替身沒有樹）、CheckSmartAutoClean／
#   ResetSmartAutoClean／ChangeACSmartInterval／CheckSmartAutoCleanCanStart（生產端，不在頁面）、SetDeviceInTray／SetCleanCellValue／
#   CleanPadCountCanSupport2Arm（移植樹 AutoClean/AutoClean.cpp 已有自由函式版）。


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfCleaning::' + meth + r'\s*\(', l):
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


def _split(line):
    """[(是程式碼?, 文字)]：字串常值不改寫；// 註解丟掉（replace 碼是單行，原文留在 #if 0 裡）。"""
    out, i, n, buf = [], 0, len(line), ''
    while i < n:
        c = line[i]
        if line.startswith('//', i):
            break
        if c in '"\'':
            if buf: out.append((True, buf)); buf = ''
            j = i + 1
            while j < n and line[j] != c:
                j += 2 if line[j] == '\\' else 1
            out.append((False, line[i:j + 1])); i = j + 1; continue
        buf += c; i += 1
    if buf: out.append((True, buf))
    return out


_NAMES = sorted(_W, key=len, reverse=True)
_WRE = re.compile(r'(?<![\w.:])(?<!->)(' + '|'.join(map(re.escape, _NAMES)) + r')\b(?!\s*::)')   # `x>widget`（比較）也要轉；只排除 `->widget`
_MRE = re.compile(r'(?<![\w.>:])(' + '|'.join(METHODS) + r')\s*\(')
_UD = 'EL<filerw::ELTrackBar>("TfCleaning", "udDeviceCT")'
_UDE = re.escape(_UD)


def _conv(code):
    """一行 golden 程式碼 → 產生器同樣的改寫（方法→CL_、widget→EL<>、訊息框、ChangeCompomentEnabled、TComboBox 連動），
    再加本檔的 (1)～(5)。只用在 replace 的取代碼（產生器不改寫 replace 的取代碼）。"""
    parts = []
    for is_code, t in _split(code):
        if is_code:
            t = t.replace('fQwertyKey->ShowQwertyKey(', 'CL_QwertyKey(')                                    # (5)
            t = _MRE.sub(lambda m: 'CL_' + m.group(1) + '(', t)
            t = _WRE.sub(lambda m: 'EL<%s>("TfCleaning", "%s")' % (_W[m.group(1)], m.group(1)), t)
            t = re.sub(r'\bShowMyMessageBox_YES_NO\s*\(', 'filerw::ELAsk(', t)
            t = re.sub(r'\bShowMyMessage\s*\(', 'filerw::ELMessage(', t)
            t = re.sub(r'\bChangeCompomentEnabled\s*\(', 'filerw::ELChangeCompomentEnabled(', t)
        parts.append(t)
    line = ''.join(parts).rstrip()
    # (4) ELTrackBar 代理的讀值 → (int)；寫（=、+=、-=）不動
    line = re.sub(_UDE + r'->(Min|Max|Position)(?!\s*(?:=(?!=)|\+=|-=))', lambda m: '((int)' + _UD + '->' + m.group(1) + ')', line)
    # (3) BCB Variant 數值比較
    line = re.sub(r'\(\(int\)' + _UDE + r'->Min\)\s*>=\s*(EL<\w+>\("TfCleaning", "\w+"\)->Text)',
                  lambda m: 'CL_VarGE(((int)' + _UD + '->Min), ' + m.group(1) + ')', line)
    # (1) TEdit OnChange
    line = re.sub(r'(EL<TEdit>\("TfCleaning", "(?:' + '|'.join(ONCHANGE) + r')"\))->Text\s*=(?!=)\s*([^;]+);',
                  r'CL_EditText(\1, \2);', line)
    # (2) TRadioGroup OnClick
    line = re.sub(r'(EL<TRadioGroup>\("TfCleaning", "(?:' + '|'.join(ONCLICK_RG) + r')"\))->ItemIndex\s*=(?!=)\s*([^;]+);',
                  r'CL_RadioIndex(\1, \2);', line)
    # 產生器的 TComboBox 連動（gen_editlist.py:296-299）
    line = re.sub(r'(EL<TComboBox>\("\w+", "\w+"\))->Text\s*=(?!=)\s*([^;]+);', r'filerw::ELComboText(\1, \2);', line)
    line = re.sub(r'(EL<TComboBox>\("\w+", "\w+"\))->ItemIndex\s*=(?!=)\s*([^;]+);', r'filerw::ELComboIndex(\1, \2);', line)
    return line.strip()


def _code(gl):
    return _cpp[gl - 1].split('//')[0]


_TRIG = re.compile(r'fQwertyKey->ShowQwertyKey\(|(?<![\w.:])(?<!->)udDeviceCT->(Min|Max|Position)'   # 產生器的 widget 規則排除前一字元是 > 的（為了 `a->b`），`x>udDeviceCT->Max` 會漏轉
                   r'|(?<![\w.>:])(' + '|'.join(ONCHANGE) + r')->Text\s*=(?!=)'
                   r'|(?<![\w.>:])(' + '|'.join(ONCLICK_RG) + r')->ItemIndex\s*=(?!=)'
                   r'|->Hint\s*=(?!=)')

MANUAL = []   # 下面填；_auto() 跳過 MANUAL／blocks 已涵蓋的行


def _covered(meth, gl):
    for r in MANUAL + BLOCKS:
        if r[0] == meth and r[1] <= gl <= r[2]:
            return True
    return False


def _auto():
    out = []
    for m in METHODS:
        a, b = SPANS[m]
        for gl in range(a + 1, b):
            code = _code(gl)
            if not _TRIG.search(code) or _covered(m, gl):
                continue
            if re.search(r'->Hint\s*=(?!=)', code):
                out.append((m, gl, gl, '(6) TControl::Hint：vclcompat 沒有；只給 RPDefault 走訪表單用（未移植）', ';'))
                continue
            out.append((m, gl, gl, '自動：(1) TEdit OnChange／(2) TRadioGroup OnClick／(3) Variant 數值比較／(4) ELTrackBar 讀值 (int)'
                        '／(5) 小鍵盤 → CL_QwertyKey（見設定檔檔頭）', _conv(code)))
    return out


# ---------------------------------------------------------------------------------------------------------------
_fs_csv = RANGE('FormShow', 'if(bHasTrayCSV)', '}')
_fs_csv = (_fs_csv[0], L('FormShow', 'cbbCleanPadCount->Visible=(TestIF_File.iTestMode==_12Site2X6);') - 2)   # if(bHasTrayCSV){ … } 到 :1499
_da = RANGE('DrawAutoClean', 'if(rgCleanKitType->ItemIndex==0 || IniConfig.bE43AutoCleanUseHotplate)',
            'SetDeviceInTray(iXItem, iYItem, iDeviceCount, eUcleanUsed);')
_rpd = RANGE('sbCleanSaveClick', 'if(CosFunction.bRecipeParameterDefault)', 'fRPDefault->CompareRPDefaultAndValue(')
_rpd = (_rpd[0], _rpd[1] + 1)
_gig = RANGE('sbCleanSaveClick', 'if(CUSTOMER_CODE==CC_GIGAS)', 'fRPDefault->ShowMonitoredParameter();')
_gig = (_gig[0], _gig[1] + 1)
_kyec = RANGE('rgCleanKitTypeClick', 'if(CUSTOMER_CODE==CC_KYEC_LEE && InitialOK && fFTPClient->bShow==false)', 'return;')
_kyec = (_kyec[0], L('rgCleanKitTypeClick', 'iLastCleanKitType=rgCleanKitType->ItemIndex;') - 1)
_ld_dc_min = L('LoadAutoCleanData', 'cbbCleanPadCount->Text      =udDeviceCT->Min;')
_ld_de_min1 = L('LoadAutoCleanData', 'edDevicePices->Text         =udDeviceCT->Min;')
_ld_de_min2 = L('LoadAutoCleanData', 'edDevicePices->Text=udDeviceCT->Min;')

MANUAL += [
    # ---- FormShow：視窗本身的屬性、焦點
    ('FormShow', L('FormShow', 'Left=100;'), L('FormShow', 'Top =10;'), '表單位置（HTML 端）', ';'),
    ('FormShow', L('FormShow', 'cbbSelectTray->Clear();'), L('FormShow', 'cbbSelectTray->Clear();'),
     'TComboBox::Clear()：VCL CB_RESETCONTENT（清選項、ItemIndex=-1、Text=""）；vclcompat TComboBox 只是欄位',
     '{ TComboBox* cb=EL<TComboBox>("TfCleaning", "cbbSelectTray"); cb->Items->Clear(); cb->ItemIndex=-1; cb->Text=""; }'),
    ('FormShow', L('FormShow', 'S.sprintf("Tray Form'), L('FormShow', 'Caption=S;'),
     '視窗標題（HTML 端；且 golden 把 AnsiString 當 %s 的可變參數，BCB 可行、標準 C++ 不行）', ';'),
    ('FormShow', _fs_csv[0], _fs_csv[1],
     'bHasTrayCSV：fConfiguration->sbtReloadTray／strngrdTray（Tray CSV 清單）在移植樹沒有；只填 cbbSelectTray 的選項'
     '（cbbSelectTray 只有 CC_ASE_KaohSiung/OSE/K3 顯示，客戶專屬）',
     'if(bHasTrayCSV) filerw::ELTodo("golden uCleaning.cpp FormShow :1488-1499 bHasTrayCSV: fConfiguration Tray CSV list (sbtReloadTray/strngrdTray) not ported -- cbbSelectTray has no database items");'),
    ('FormShow', L('FormShow', 'fCleaning->LoadAutoCleanData();'), L('FormShow', 'fCleaning->LoadAutoCleanData();'),
     'fCleaning->：golden 只有一個 TfCleaning 物件，就是本 TU（移植樹門面 fCleaning 沒有 LoadAutoCleanData）', 'CL_LoadAutoCleanData();'),
    # ---- LoadAutoCleanData
    ('LoadAutoCleanData', _ld_dc_min, _ld_dc_min, '(4) ELTrackBar 讀值 (int)＋產生器的 TComboBox 連動',
     'filerw::ELComboText(EL<TComboBox>("TfCleaning", "cbbCleanPadCount"), ((int)' + _UD + '->Min));'),
    ('LoadAutoCleanData', _ld_de_min1, _ld_de_min1, '(4) ELTrackBar 讀值 (int)',
     'EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)' + _UD + '->Min);'),
    ('LoadAutoCleanData', _ld_de_min2, _ld_de_min2, '(4) ELTrackBar 讀值 (int)',
     'EL<TEdit>("TfCleaning", "edDevicePices")->Text=((int)' + _UD + '->Min);'),
    ('LoadAutoCleanData', L('LoadAutoCleanData', 'fMain->bEnableAutoclean();'), L('LoadAutoCleanData', 'fMain->bEnableAutoclean();'),
     'fMain->bEnableAutoclean()（golden main.cpp:31331）：第一行 CUSTOMER_CODE!=CC_ASE_KaohSiung 就 return，其餘只切主畫面 AutoClean 鈕的 '
     'Visible；TfMain 門面沒有（同 GATE G-SU-AcBtn）。客戶專屬 → 只記',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung) filerw::ELTodo("golden main.cpp:31331 fMain->bEnableAutoclean() (CC_ASE_KaohSiung main-screen AutoClean button) -- TfMain facade has none");'),
    # ---- ShowCleanContactForce：golden fContact 是真 TfContact；移植樹 fContact 是 TfContactShim → FileRW/TestIF_File_Cleaning.cpp 轉接
    ('ShowCleanContactForce', L('ShowCleanContactForce', 'fContact->DutCount();'), L('ShowCleanContactForce', 'fContact->DutCount();'),
     '移植樹 fContact 是 TfContactShim（沒有 DutCount）；真表單 fContactForm（forms/fContact.h）→ 轉接（同 TestIF_File_SetUp）',
     'FileRW_Cleaning_ContactDutCount();'),
    ('ShowCleanContactForce', L('ShowCleanContactForce', 'iTotalN   =iDeviceN  *fContact->dDutCount;'),
     L('ShowCleanContactForce', 'iTotalGfAC=iDeviceGf *fContact->dDutCount;'), '同上：fContactForm->dDutCount',
     'iTotalN=iDeviceN*FileRW_Cleaning_ContactDutCountValue(); iTotalGfAC=iDeviceGf*FileRW_Cleaning_ContactDutCountValue();'),
    ('ShowCleanContactForce', L('ShowCleanContactForce', 'fContact->GetMaxIndexForceLimit();'),
     L('ShowCleanContactForce', 'fContact->GetMaxIndexForceLimit();'),
     'TfContact::GetMaxIndexForceLimit（golden cContact.cpp:19184）：移植樹 fContactForm 只有宣告（GATE X-30）→ 轉接到 cContact.h '
     'ComputeMaxIndexForceLimit（逐行同 golden 912），rgKitDiameter 取 C 路 TfContact 替身',
     'double dMaxLimit=FileRW_Cleaning_ContactMaxIndexForceLimit();'),
    # ---- sbCleanSaveClick
    ('sbCleanSaveClick', L('sbCleanSaveClick', 'Close();'), L('sbCleanSaveClick', 'Close();'),
     'Close()：A02 權限不足 golden 關表單 → 記 closed（SaveFlow 照 VCL 順序接 FormClose）', 'filerw::ELMark("closed");'),
    ('sbCleanSaveClick', L('sbCleanSaveClick', 'if(fCleaning->fShow)'), L('sbCleanSaveClick', 'if(fCleaning->fShow)'),
     'fCleaning->fShow：本 TU 的 fShow（golden 只有一個表單物件）', 'if(fShow)'),
    ('sbCleanSaveClick', L('sbCleanSaveClick', 'fMain->bEnableAutoclean();'), L('sbCleanSaveClick', 'fMain->bEnableAutoclean();'),
     '同 LoadAutoCleanData：CC_ASE_KaohSiung 客戶專屬',
     'if(CUSTOMER_CODE==CC_ASE_KaohSiung) filerw::ELTodo("golden main.cpp:31331 fMain->bEnableAutoclean() (CC_ASE_KaohSiung main-screen AutoClean button) -- TfMain facade has none");'),
    # ---- sbCleanExitClick
    ('sbCleanExitClick', L('sbCleanExitClick', 'Close();'), L('sbCleanExitClick', 'Close();'),
     'Close()：golden ShowModal 表單設 ModalResult，處理器回傳後 modal 迴圈跑 FormClose → 記 closed（SaveFlow 接 FormClose）',
     'filerw::ELMark("closed");'),
    # ---- YCT1Change：ptr 是 YCT1 或 YCT2（Sender）
    ('YCT1Change', L('YCT1Change', 'ptr->Text=10;'), L('YCT1Change', 'ptr->Text=10;'), '(1) TEdit OnChange（ptr＝Sender）',
     'CL_EditText(ptr, 10);'),
    # ---- DrawAutoClean：畫 tmyAutoClean（表單上的 TTMyTray 預覽，沒有其他讀者）
    ('DrawAutoClean', _da[0], _da[1],
     'tmyAutoClean（TTMyTray）的預覽圖：純畫面。golden SetDeviceInTray（:1952）用到 V912 的 bCleanKitAxxG3ColUse2Col'
     '（golden AutoClean.cpp:300），移植樹沒有；移植樹 AutoClean.cpp:3409 的自由函式版是 906 版，也不借用',
     ';'),
]

BLOCKS = [
    ('sbCleanSaveClick', _rpd[0], _rpd[1],
     'CosFunction.bRecipeParameterDefault：fRPDefault／fSpeed->SearchRecipeParameter／FTestIF／fYieldMonitoring 走訪表單元件樹'
     '（PCtrl->Controls[]），替身沒有樹；RPDefault 未移植（GATE G-CL-RPD）'),
    ('sbCleanSaveClick', _gig[0], _gig[1], 'CC_GIGAS fRPDefault->ShowMonitoredParameter()：客戶專屬、RPDefault 未移植'),
    ('rgCleanKitTypeClick', _kyec[0], _kyec[1],
     'CC_KYEC_LEE 切換 Kit 型式要刷條碼＋重新登入（fMain->cbUserSelect／fFTPClient）：客戶專屬，先跳過'),
]


# ---------------------------------------------------------------------------------------------------------------
# 小鍵盤表（DFM 的 TEdit/TLabeledEdit → OnClick/OnMouseDown 處理器，處理器在 KB 裡），附 DFM 父元件（可點判斷用：
# ReadOnly 不擋 OnClick，只看自己＋祖先 Enabled/Visible）。FileRW/TestIF_File_Cleaning.cpp BeforeApply 依序重播。
def _dfm_kb():
    rows, stack = [], []
    lines = _dfm.split('\n')
    for i, ln in enumerate(lines):
        t = ln.strip()
        m = re.match(r'(?:object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', t)
        if m:
            stack.append((m.group(1), m.group(2)))
            continue
        if t == 'end' and stack:
            stack.pop()
            continue
        mm = re.match(r'(OnClick|OnMouseDown)\s*=\s*(\w+)$', t)
        if mm and stack and stack[-1][1] in ('TEdit', 'TLabeledEdit') and mm.group(2) in KB:
            parent = stack[-2][0] if len(stack) >= 2 else ''
            rows.append((stack[-1][0], mm.group(2), parent, i + 1))
    return rows


KB_ROWS = _dfm_kb()
_kb_tab = ['const CL_KbEntry kCL_Kb[] = {   // golden uCleaning.dfm：元件 → 小鍵盤處理器（DFM 行）'] + \
          ['    {"%s", &CL_%s, "%s", %d},' % r for r in KB_ROWS] + ['};']

DECLS = [
    'struct CL_KbEntry { const char* name; void (*fn)(TObject*); const char* parent; int dfmLine; };   // 小鍵盤表（members）',
    'int    FileRW_Cleaning_SearchCleanNum();              // FileRW/TestIF_File_Cleaning.cpp（golden AutoClean.cpp:1324，V912）',
    'void   FileRW_Cleaning_ContactDutCount();             // FileRW/TestIF_File_Cleaning.cpp 下半（fContactForm->DutCount）',
    'double FileRW_Cleaning_ContactDutCountValue();        // 同上（fContactForm->dDutCount）',
    'double FileRW_Cleaning_ContactMaxIndexForceLimit();   // 同上（cContact.h ComputeMaxIndexForceLimit）',
    'extern int  iAutoCleanCnt;                            // golden ainarm2.h:89（移植樹定義在 ainarm2.cpp:96，沒有 header 宣告）',
    'extern bool bAuthCriticalPara[26];                    // cAuthority.h:64（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）',
    'extern int  iCloseSiteModeFor2x6;                     // golden ainarm9045_2x6_8.h:23（aHotPlateSubstrate.h:902 與 HTEditList.h 衝突）',
    'extern int  iCloseSiteModeFor2x8;                     // golden ainarm9045_2x8_8.h:28（同上）',
    'extern int  iAutoCleanUseXPitch;                      // golden ainarm2.h:177（aHotPlateSubstrate.h:802，同上）',
    'int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr=false, AnsiString errPart=" ");   // canary_support.h:66',
] + ['static void CL_%s(TObject *Sender);' % m for m in sorted(set(KB) | {'XCT1Change', 'YCT1Change', 'XPitch2Change'})] + \
    ['static void CL_rgCleanKitTypeClick();', 'static void CL_rgKitPositionClick();']

MEMBERS = [
    'bool fShow=false;             // golden uCleaning.h:346 —— 本 TU 自己的（網頁沒有關表單事件；Exit 才跑 FormClose 設回 false）',
    '#define bResetCleanCount (fCleaning->bResetCleanCount)                // golden uCleaning.h:333 —— 移植樹 AutoClean.cpp:3967/:4035 寫它',
    '#define iPosTemp (fCleaning->iPosTemp)                                // golden uCleaning.h:338',
    '#define b1x2SiteAbClosePutDummy (fCleaning->b1x2SiteAbClosePutDummy)  // golden uCleaning.h:345 —— 移植樹 AutoClean.cpp:2392/:3531/:4185 讀它',
    '#define b12SiteRun2x4 (fCleaning->b12SiteRun2x4)                      // golden uCleaning.h:360 —— 移植樹 forms/fCleaning.cpp XCT1Change 同一份',
    '#define iDeviceCount (fCleaning->iDeviceCount)                        // golden uCleaning.h:361',
    '#define SearchCleanNum FileRW_Cleaning_SearchCleanNum                 // golden AutoClean.cpp:1324（移植樹 AutoClean.cpp:1662 是 static、906 版）',
    # (5) 小鍵盤重播時頁面送來的值（FileRW/TestIF_File_Cleaning.cpp BeforeApply 設、CL_QwertyKey 讀）
    'const AnsiString* CL_KbPending=nullptr;',
    # golden cpublic.h:19 Get0_01MMType(char*)（本體只 atof，不改字串）；vclcompat c_str() 是 const char* → 本 TU 的 const 多載轉接
    'int Get0_01MMType(const char* s) { return Get0_01MMType(const_cast<char*>(s)); }   // cpublic.h:19',
    # (1) VCL TEdit.OnChange
    'void CL_EditText(TCustomEdit* e, const AnsiString& v) { if(e->Text==v) return; e->Text=v; '
    'if(e==filerw::EL<TEdit>("TfCleaning", "XCT1") || e==filerw::EL<TEdit>("TfCleaning", "XCT2")) CL_XCT1Change(e); '
    'else if(e==filerw::EL<TEdit>("TfCleaning", "YCT1") || e==filerw::EL<TEdit>("TfCleaning", "YCT2")) CL_YCT1Change(e); '
    'else if(e==filerw::EL<TEdit>("TfCleaning", "XPitch2")) CL_XPitch2Change(e); }   // (1) golden DFM OnChange：XCT1/XCT2=XCT1Change、YCT1/YCT2=YCT1Change、XPitch2=XPitch2Change',
    # (2) VCL TRadioGroup.SetItemIndex（夾在 -1..Count-1，值有變 → OnClick）
    'void CL_RadioIndex(TRadioGroup* rg, int v) { if(v<-1) v=-1; if(rg->Items && v>=rg->Items->Count) v=rg->Items->Count-1; '
    'if(rg->ItemIndex==v) return; rg->ItemIndex=v; '
    'if(rg==filerw::EL<TRadioGroup>("TfCleaning", "rgCleanKitType")) CL_rgCleanKitTypeClick(); '
    'else if(rg==filerw::EL<TRadioGroup>("TfCleaning", "rgKitPosition")) CL_rgKitPositionClick(); }   // (2) golden DFM OnClick：rgCleanKitType/rgKitPosition',
    # (3) BCB sysvari.h:3113 operator>=(int, const Variant&)：字串轉數值比較（非數字字串 golden 丟 EVariantTypeCastError；StrToFloat 同樣丟例外）
    'bool CL_VarGE(int lhs, const AnsiString& rhs) { return (double)lhs >= StrToFloat(rhs); }   // (3)',
    # (5) golden myQwertyKeyBoard.cpp:169-301 的值那一半
    'void CL_QwertyKey(TControl* Ptr, int iFunction, int iDP=0, bool bCheckRange=false, double min=0, double max=0) { (void)iDP; '
    'TPanel* PanelPtr=dynamic_cast<TPanel*>(Ptr); TCustomEdit* EditPtr=dynamic_cast<TCustomEdit*>(Ptr); '
    'AnsiString txt = CL_KbPending ? *CL_KbPending : (PanelPtr ? PanelPtr->Caption : (EditPtr ? EditPtr->Text : AnsiString(""))); '
    'if(iFunction&N_PORT) { bCheckRange=true; if(min<0 || max<=0) { min=0; max=65535; } } '
    'if(iFunction&N_INTEGER || iFunction&N_DOUBLE) { double d=atof(txt.c_str()); if(bCheckRange) txt=AnsiString(CheckRange(d, min, max)); } '
    'if(PanelPtr!=NULL) PanelPtr->Caption=txt; else if(EditPtr!=NULL) CL_EditText(EditPtr, txt); }   // (5) golden :256-301（TEdit 寫回照 (1) 觸發 OnChange）',
] + [chr(10).join(_kb_tab)]

STRUCT = {
    'struct': 'TestIF_File_Cleaning',
    'prefix': 'CL',
    'class': 'TfCleaning',
    'cpp': 'AutoClean/uCleaning.cpp',   # 斜線：產生器把它寫進字串常值（ELTodo），反斜線加 u 會被當成 UCN
    'h': 'AutoClean/uCleaning.h',
    'files': ['HandlerCondition.Data（[Configuration] iAutoClean_*／dBufferKit*／Hotplatl*、[AutoClean]）',
              'IniData\\DefineAutoClean\\AutoClean.Data（CC_TSMC_TAINAN／CosFunction.bUseDefineAutoCleanOffset）',
              'HandlerCondition.Data／AutoCleanCount.Data 的 iAutoCleanPad_CountTime_*（ReadWriteAutoCleanCount）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['sbCleanSaveClick', 'SaveAutoCleanData', 'sbCleanExitClick', 'FormClose'],
    'params': dict({'TfCleaning': '', 'FormShow': '', 'sbCleanSaveClick': '', 'sbCleanExitClick': '', 'FormClose': '',
                    'rgCleanKitTypeClick': '', 'rgKitPositionClick': '', 'cbbCleanPadCountChange': ''},
                   **{m: 'TObject *Sender' for m in KB if m.endswith('MouseDown')}),
    'rettype': {'GetMinCleanPadCount': 'int', 'SetDeviceMaxMin': 'int'},
    'members': MEMBERS,
    'globals': ['iBackupCleaningDeveicePices', 'iBackupCleaningItemX', 'iBackupCleaningItemY', 'iBackupAutoClean_Function'],
    'replace': MANUAL + _auto(),
    'blocks': BLOCKS,
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'cpublic.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fSetup.h', 'forms/fCleaning.h',
                 'forms/fSpeed.h', 'forms/fShowBinSelect.h', 'forms/fLotInfo.h', 'Motor/mymotor.h', 'AutoClean/AutoClean.h',
                 'cUnitConvert.h', 'cinitial.h', 'ainarm9045.h', 'ainarm9045_2x6_8.h', 'csystem.h', 'cMyDB.h'],
    'decls': DECLS,
    'overrides': [],
}
