# -*- coding: utf-8 -*-
# tools/editlist/Offset_File.py -- gen_editlist.py 的結構設定（C 形狀：具名替身）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only Offset_File
#
# Steven 團隊 20260925：Offset_File 族（<offset 資料夾>\Position Offset.Data ＋ Hot／cool 變體；
# 結構 Offset_File、InArmOffSet_File[]、OutArmOffSet_File[]、SortArmOffSet_File[]）—— golden TfOffSet
# （cOffSet.cpp，912，4182 行）。頁面 web/page/Setup.OffSet.html。
#
# ⚠ TfOffSet 是「選取式編輯器」：同一組 edit 依按鈕顯示不同部位。Steven 20260925 定案：
#   開頁時對每一個選取各跑一次 golden 的按鈕事件（SpBotSelClick :2476／IndexOffSetBT2Click :2594，內含
#   ShowOneByOneOffSet＋DoIniDataToForm），整包用 JSON 傳給頁面；頁面點按鈕只切換顯示哪一組。
#   存檔時伺服器對每一組：先跑同一個按鈕事件（非頁面欄位＝golden 值）→ 套頁面值 → 跑 golden spbSaveClick（:2807，
#   只存目前選取）。JSON 形狀與 _EditPage 的 PageJson 不同 → 手寫入口 FileRW/Offset_File_C.cpp
#   （FileRW_Offset_Page／FileRW_Offset_Save）。A 形狀 FileRW/Offset_File.cpp 佔用了 Offset_File.cpp 這個檔名。
#
# 讀檔器：接移植樹 fOffSet->ReadFile()（cOffSet.cpp:367，live，已含 912 的 RogerYang 20260126／Steven 20260316 修正）。
#   golden ReadFile 本身不碰畫面元件（只讀檔 → 結構），另轉一份只會多一個讀檔器。
#   golden 成員 LastFileName：GetOffsetPath（移植樹 cOffSet.cpp:188）寫它、ReadFile 讀它 → #define 到 fOffSet 那一份。
# ReadFile 的呼叫次數（取捨，見 members 的 OS_bSkipReadFile）：golden 每按一次部位鈕就 ReadFile 一次；開頁要跑
#   53＋11 次按鈕事件，檔案在這之間沒有變 → 只在 FormShow 讀一次，按鈕事件裡的 ReadFile 略過。
#   存檔時各組的按鈕事件照 golden 先 ReadFile（非頁面欄位＝檔案現值），尾端的 ReadFile 也照跑。
import os
import re

G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(G, 'cOffSet.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_h = open(os.path.join(G, 'cOffSet.h'), 'rb').read().decode('cp950', errors='replace')
_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_W = {}
for _m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _h[re.search(r'class\s+(?:PACKAGE\s+)?TfOffSet\b[^;{]*\{', _h).end():], re.M):
    _W[_m.group(2)] = _m.group(1) if _m.group(1) in _KNOWN else 'TControl'

METHODS = ['TfOffSet', 'LoadImage', 'FormShow', 'SetXYPitchVCLVisible', 'ShowOneByOneOffSet', 'SaveFile',
           'SaveSetupFile', 'DoIniDataToForm', 'SpBotSelClick', 'IndexOffSetBT2Click', 'spbSaveClick',
           'Timer1Timer', 'ShowFinalAirForce', 'btnBackClick', 'ShowOffSetList']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfOffSet::' + meth + r'\s*\(', l):
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


def EL(name):
    return 'EL<%s>("TfOffSet", "%s")' % (_W[name], name)


def R1(meth, text, why, code, nth=1):
    gl = L(meth, text, nth)
    return (meth, gl, gl, why, code)


# spbSaveClick 的 ASE_CL 段：golden 以 fOffSet->widget 讀自己的元件（產生器的 widget 規則不改 -> 之後的名稱）
def _fof_lines():
    out = []
    a, b = SPANS['spbSaveClick']
    for gl in range(a, b + 1):
        raw = _cpp[gl - 1]
        code = raw.split('//')[0] if not raw.lstrip().startswith('//') else ''
        if 'fOffSet->' not in code:
            continue
        new = re.sub(r'fOffSet->(\w+)', lambda m: EL(m.group(1)), code.rstrip())
        out.append(('spbSaveClick', gl, gl, 'golden 以 fOffSet->元件 讀自己表單的元件（ASE_CL scale）→ 同一個具名替身', new.strip()))
    return out


_ct_sg = RANGE('TfOffSet', 'sgOffsetList->ColCount=oiArmTotal;', 'sgOffsetList->Cells[oiArmPickH_Y ][0]=" H_Y";')
_fs_cap = RANGE('FormShow', 'S.sprintf("Position Offset', 'Caption=S;')
_fs_pk = RANGE('FormShow', 'pnlPicker->Width', 'iXYPitch16Bd_Be)?976:487;')
_fs_oee = RANGE('FormShow', 'if(CosFunction.bOEEFunction && fProductionInfo!=NULL)', 'pnlOEE_Contact->Visible     =false;')
_fs_oee = (_fs_oee[0], _fs_oee[1] + 1)          # 含 else 的收尾 }
_fs_cl = RANGE('FormShow', 'sg_Offset_HotTempShiftOffset->ColWidths[0]', 'fOffSet->ReadHotTempShiftOffsetData();')
_sl_body = (SPANS['ShowOffSetList'][0] + 2, SPANS['ShowOffSetList'][1] - 1)

REPLACE = [
    # ---- 建構子（golden :186）
    R1('TfOffSet', 'OffSetSelBot[i]->OnClick=SpBotSelClick;', 'OnClick 事件指派：頁面按鈕由入口 FileRW/Offset_File_C.cpp 以 Tag 呼叫 OS_SpBotSelClick(i)（同一支 golden 事件）', ';'),
    R1('TfOffSet', 'OfffSetSpecSelBot[i]->OnClick=IndexOffSetBT2Click;', 'OnClick 事件指派：同上（OS_IndexOffSetBT2Click(i)）', ';'),
    ('TfOffSet', _ct_sg[0], _ct_sg[1], 'sgOffsetList（TStringGrid）表頭／欄寬／列數：總表分頁 tsArmOffset 的畫面，替身 ELStringGrid 沒有 ColCount／ColWidths；'
     '總表內容頁面可由各組 offsets 自己組（ShowOffSetList 同樣不轉）', ';'),
    R1('TfOffSet', 'LoadImage();', 'golden LoadImage() 用 cOffSet.h 的預設參數 bInArm=true', 'OS_LoadImage(true);'),
    R1('LoadImage', 'Image2->Picture->LoadFromFile(asString);', '示意圖（HTML 自己有）；無大括號 if 的本體 → 空區塊（免 -Wempty-body）', '{ }'),
    # ---- FormShow（golden :496）
    ('FormShow', _fs_cap[0], _fs_cap[1], 'Caption＝視窗標題（含 LastFileName；AnsiString 丟進 sprintf 的 ... g++ 不收）', ';'),
    R1('FormShow', 'Left=10;', '視窗位置', ';'),
    R1('FormShow', 'Top=10;', '視窗位置', ';'),
    R1('FormShow', 'btnBackClick(this);', '事件以 (this) 呼叫（參數已拿掉）', 'OS_btnBackClick();'),
    R1('FormShow', 'if(fBarCode->bShow && CUSTOMER_CODE==CC_KYEC_XILINX)',
       'fBarCode->bShow（BarCode 表單開著）：TfBarCode 在 aHotPlateSubstrate.h:984（與 HTEditList.h 衝突）且沒有 bShow；web 開這頁時 BarCode 表單不會開著 → false',
       '{ if(CUSTOMER_CODE==CC_KYEC_XILINX) filerw::ELTodo("golden cOffSet.cpp:587 fBarCode->bShow taken as false (TfBarCode has no bShow in the port)"); } '
       'if(false && CUSTOMER_CODE==CC_KYEC_XILINX)'),
    R1('FormShow', 'FrmAOI->RunTopBottomInspect()',
       'FrmAOI->RunTopBottomInspect()：移植樹 TFrmAOI 沒有這支 → false（只影響 Pnl_ScanAOI／sbScanAOI 顯示），Top&Bottom AOI 機種回報 todo',
       '{ if(USE_Scanner_AOI_Inspection==(int)eBtnAOI_TopBottomInstall) filerw::ELTodo("golden cOffSet.cpp:625 FrmAOI->RunTopBottomInspect() not in the port -- taken as false (Scan AOI button hidden)"); } '
       + EL('Pnl_ScanAOI') + '->Visible =(USE_Scanner_AOI_Inspection==(int)eBtnAOI_BottomInstall && ScannerAOIIF.iEnableScannerMode!=0) || (false);'),
    R1('FormShow', 'pnlMain->Width', '版面寬度（HTML 不用；vclcompat TControl 沒有 Width）', ';'),
    ('FormShow', _fs_pk[0], _fs_pk[1], '版面寬度（HTML 不用）', ';'),
    R1('FormShow', 'fOffSet->Width          =pnlMain->Width+pnlPicker->Width+5;', '視窗寬度（HTML 不用）', ';'),
    R1('FormShow', 'LoadImage();', 'golden LoadImage() 用預設參數 bInArm=true', 'OS_LoadImage(true);'),
    ('FormShow', _fs_oee[0], _fs_oee[1],
     'OEE Contact Force offset：fProductionInfo->CheckContactForceExist／GetOffsetContactForce 全樹沒有（同移植樹 GATE O-7）→ pnlOEE_Contact 不顯示（golden else 分支），OEE 機台回報 todo',
     '{ if(CosFunction.bOEEFunction) filerw::ELTodo("golden cOffSet.cpp:723-732 OEE contact force offset (fProductionInfo->CheckContactForceExist/GetOffsetContactForce) not in the port (GATE O-7) -- pnlOEE_Contact hidden"); } '
     + EL('pnlOEE_Contact') + '->Visible=false;'),
    ('FormShow', _fs_cl[0], _fs_cl[1],
     'CC_ASE_CL：sg_Offset_HotTempShiftOffset 表頭（ELStringGrid 沒有 ColWidths）＋fOffSet->ReadHotTempShiftOffsetData()（移植樹 TfOffSet 沒有）',
     'filerw::ELTodo("golden cOffSet.cpp:783-828 ASE_CL hot temp shift offset grid + ReadHotTempShiftOffsetData() not in the port");'),
    # ---- ShowOneByOneOffSet（golden :888）
    R1('ShowOneByOneOffSet', 'if(fContact->IsRun2DCheck()==true)', '移植樹 fContact 是 TfContactShim（atester_shims.h，IsRun2DCheck 離線回 false）；真表單 fContactForm（forms/fContact.h:1573 ACTIVE）→ 入口檔轉接',
       'if(FileRW_Offset_IsRun2DCheck()==true)', 1),
    R1('ShowOneByOneOffSet', 'if(fContact->IsRun2DCheck()==true)', '同上', 'if(FileRW_Offset_IsRun2DCheck()==true)', 2),
    # ---- SpBotSelClick（golden :2476）：頁面「點部位鈕」＝入口以 Tag 呼叫
    R1('SpBotSelClick', 'return;', 'KYEC barcode 登入被拒（golden return）：記下讓入口知道這個選取沒生效',
       '{ OS_bSelRefused=true; filerw::ELMark("barcodeRefused"); return; }'),
    R1('SpBotSelClick', 'TSpeedButton *Ptr=(TSpeedButton *)Sender;', 'Sender＝被按的部位鈕：入口傳 Tag（＝golden 建構子 :326 OffSetSelBot[i]->Tag=i）',
       'TSpeedButton *Ptr=OffSetSelBot[iPtrTag];'),
    R1('SpBotSelClick', 'bool bResult=SaveFile(iNowOffsetSel, iSaveStander, false);',
       'golden 換部位前先存「上一個部位」。C 路的顯示是開頁時逐一切換、不是使用者操作 → 不存；使用者改過的每一組都由存檔請求各自跑 spbSaveClick 存（最後檔案內容相同）',
       'bool bResult=false;'),
    # ---- IndexOffSetBT2Click（golden :2594）
    R1('IndexOffSetBT2Click', 'TSpeedButton *Ptr=(TSpeedButton *)Sender;', 'Sender：入口傳 Tag（golden 建構子 :367 OfffSetSpecSelBot[i]->Tag=i）',
       'TSpeedButton *Ptr=OfffSetSpecSelBot[iPtrTag];'),
    R1('IndexOffSetBT2Click', 'SaveFile(iSpecialOffSetSel, iSaveSpecial, false);', '同 SpBotSelClick：換選取前先存上一個 → C 路顯示不存（存檔請求各組自己存）', ';'),
    # ---- spbSaveClick（golden :2807）
    R1('spbSaveClick', 'fMain->iOperatorModeCount=0;', 'fMain->iOperatorModeCount（時間到登出的計數）：移植樹 TfMain 沒有這個成員',
       'filerw::ELTodo("golden cOffSet.cpp:2811 fMain->iOperatorModeCount=0 not done (member not in the port TfMain)");'),
    R1('spbSaveClick', 'if(PageControl1->ActivePageIndex==0)', 'ActivePageIndex==0（使用者在 In Out Arm 分頁）：入口依這一組是 stander 還是 special 設 OS_iActivePage',
       'if(OS_iActivePage==0)'),
    R1('spbSaveClick', 'SaveFile(iNowOffsetSel, iSaveStander, false);', '多組存檔：尾端（ReadFile 起）只在全部組存完後跑一次 → 尾端那次跳過 SaveFile',
       'if(!OS_bTailOnly) OS_SaveFile(iNowOffsetSel, iSaveStander, false);'),
    R1('spbSaveClick', 'SaveFile(iSpecialOffSetSel, iSaveSpecial, false);', '同上',
       'if(!OS_bTailOnly) OS_SaveFile(iSpecialOffSetSel, iSaveSpecial, false);'),
    R1('spbSaveClick', 'ReadFile();', '多組存檔：前面各組只跑到 SaveFile（OS_bPartOnly），ReadFile 以後的全域副作用（重讀、ASE_CL、Pause、SetWorkParameter）最後跑一次',
       'if(OS_bPartOnly) return;   ReadFile();'),
    R1('spbSaveClick', 'SetTempShiftOffsetPeriod(25,', 'SetTempShiftOffsetPeriod（pig 20240215）：移植樹沒有',
       'filerw::ELTodo("golden cOffSet.cpp:2830 SetTempShiftOffsetPeriod(25, recipe) not in the port");'),
    R1('spbSaveClick', 'SetTempShiftOffsetPeriod(Temperature.fWorkTemperBase', '同上',
       'filerw::ELTodo("golden cOffSet.cpp:2834 SetTempShiftOffsetPeriod(fWorkTemperBase, recipe) not in the port");'),
    R1('spbSaveClick', 'S.sprintf("D:\\\\HT9045\\\\data\\\\%s.ini", fMain->cbSetupFileName->Text);', 'AnsiString 丟進 sprintf 的 ...（g++ 不收）→ .c_str()，路徑照 golden',
       'S.sprintf("D:\\\\HT9045\\\\data\\\\%s.ini", fMain->cbSetupFileName->Text.c_str());'),
    R1('spbSaveClick', 'fMain->Pause("Save Offset");', '移植樹 TfMain::Pause 是離線計數器（forms/fMain.h:167，不停機）→ 照叫並回報',
       'fMain->Pause("Save Offset"); filerw::ELTodo("golden cOffSet.cpp:2880 fMain->Pause(\\"Save Offset\\") is an offline counter in the port -- machine not paused");'),
    # ---- ShowOffSetList（golden :3392）：總表 grid
    ('ShowOffSetList', _sl_body[0], _sl_body[1], 'sgOffsetList（TStringGrid）的 ColWidths／RowHeights／Cells：總表分頁的畫面，替身沒有 ColWidths；資料就是各組 offsets（頁面自己組表）', ';'),
] + [r for r in _fof_lines() if 'S.sprintf' not in _cpp[r[1] - 1]]


# ---- //AI(W906-EVB3) 20260928 [W906]：OS-1（事件移植批次 B3，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md）——
#   微調 offset 的上／下／右／左四顆鈕（golden cOffSet.cpp:2892-2943，dfm:9241／:8888／:9123／:9005，在 pan_AutoOffsetMove 裡，
#   pan_AutoOffsetMove->Visible＝IniConfig.bUseAutoOffsetFunction，FormShow :572）：SystemStart 時 return；edArmY／edArmX ±0.1；
#   spbSaveClick(this) 立刻存目前選的部位；勾了 cb_AutoOffsetPositionCheck 就 fMain->Start(...)（OS-1b，B8，機台會動 → 不做，記 todo 給 Jimmy）。
#   事件表由產生器產生；手寫入口 FileRW/Offset_File.cpp 檔尾另包一層（Offset_File 不是 PageDesc，同 IniConfig 的別名頁；還要先確定
#   伺服器端選的就是頁面上那個部位，見該處註解）。
def _expect(gl, text):
    if _cpp[gl - 1].strip() != text:
        raise SystemExit('Offset_File.py: golden cOffSet.cpp:%d is %r, expected %r' % (gl, _cpp[gl - 1].strip(), text))
    return gl


_AUTO = [('sb_AutoOffsetUpClick', 'edArmY', '+'), ('sb_AutoOffsetDownClick', 'edArmY', '-'),
         ('sb_AutoOffsetRightClick', 'edArmX', '+'), ('sb_AutoOffsetLeftClick', 'edArmX', '-')]
_AUTO_REPLACE = []
for _m, _ed, _sg in _AUTO:
    SPANS[_m] = _span(_m)
    _expect(SPANS[_m][0] + 2, 'if(SystemStart)')
    _expect(L(_m, '->Text=FormatFloat('), '%s->Text=FormatFloat("0.00", atof(%s->Text.c_str())%s0.1);' % (_ed, _ed, _sg))
    _gl = _expect(L(_m, 'spbSaveClick(this);'), 'spbSaveClick(this);')
    _AUTO_REPLACE.append((_m, _gl, _gl,
                          'spbSaveClick(this)：golden 同一個存檔鈕（:2807）。這幾顆鈕在 In Out Arm 分頁上（ActivePageIndex==0）→ 存目前的部位 iNowOffsetSel；'
                          '多組存檔的旗標（OS_bPartOnly／OS_bTailOnly）歸零＝整支 spbSaveClick 照 golden 跑（SaveFile＋ReadFile 以後的尾端）',
                          'OS_bSkipReadFile=false; OS_bPartOnly=false; OS_bTailOnly=false; OS_iActivePage=0; OS_spbSaveClick();'))
    _gl = _expect(L(_m, 'fMain->Start('), 'fMain->Start("%s");' % _m)
    _AUTO_REPLACE.append((_m, _gl, _gl,
                          'OS-1b：勾了「Auto Offset Position Check」時 golden 叫 fMain->Start 讓機台跑一次 —— 會讓機台動，B8／Jimmy（WebStart.cpp 是 START 入口）；'
                          '這裡不跑，記 todo 讓頁面看得到',
                          'filerw::ELTodo("golden cOffSet.cpp:%d fMain->Start(\\"%s\\") (OS-1b: auto offset position check runs the machine once) '
                          'not ported -- B8 / Jimmy (WebStart.cpp); the offset was saved, the machine did not move");' % (_gl, _m)))
REPLACE += _AUTO_REPLACE


STRUCT = {
    'struct': 'Offset_File',
    'prefix': 'OS',
    'class': 'TfOffSet',
    'cpp': 'cOffSet.cpp',
    'h': 'cOffSet.h',
    'files': ['Position Offset.Data', 'Position Offset Hot.Data（shuttle Hand X/Y、Test Arm shuttle 四鍵，LastSet.iTemperature==Hot）',
              'Tri_Position_Offset()（Tri_Temp_Machine：Position Offset cool／Hot／一般）', 'D:\\HT9045\\data\\<recipe>.ini（CC_ASE_CL scale）'],
    'lists': [],
    'methods': METHODS + [m for m, _e, _s in _AUTO],   # //AI(W906-EVB3) 20260928 [W906]：OS-1 四支加在最後（既有方法的產生碼不動）
    'save_methods': ['spbSaveClick', 'SaveFile', 'SaveSetupFile'],
    'params': {'TfOffSet': '', 'FormShow': '', 'spbSaveClick': '', 'Timer1Timer': '', 'btnBackClick': '',
               'SpBotSelClick': 'int iPtrTag', 'IndexOffSetBT2Click': 'int iPtrTag',
               # //AI(W906-EVB3) 20260928 [W906]：OS-1，golden 不讀 Sender
               'sb_AutoOffsetUpClick': '', 'sb_AutoOffsetDownClick': '', 'sb_AutoOffsetRightClick': '', 'sb_AutoOffsetLeftClick': ''},
    'rettype': {'SaveFile': 'bool', 'SaveSetupFile': 'bool'},
    'members': [
        # golden cOffSet.cpp:172-174 檔案層級常數（cOffSet.h:504-506 extern；移植樹沒有）
        'const int iSaveStander=1;     // golden cOffSet.cpp:172',
        'const int iSaveSpecial=2;     // golden cOffSet.cpp:173',
        'const int iSaveAll=3;         // golden cOffSet.cpp:174',
        # golden cOffSet.h:452-494 非元件成員
        'TSpeedButton *OffSetSelBot[OfsTotal];          // golden cOffSet.h:452',
        'TSpeedButton *OfffSetSpecSelBot[trayOfsTotal]; // golden cOffSet.h:453',
        'TEdit *MyPickEdit[2][8];                       // golden cOffSet.h:454',
        'TEdit *MyRelsEdit[2][8];                       // golden cOffSet.h:455',
        'TEdit *MySingleOffsetTEditX[2][8];             // golden cOffSet.h:458',
        'TEdit *MySingleOffsetTEditY[2][8];             // golden cOffSet.h:459',
        'TLabel *MyPickLab[2][8];                       // golden cOffSet.h:460',
        'TLabel *MyRelsLab[2][8];                       // golden cOffSet.h:461',
        'TLabel *MySingleOffsetLabX[2][8];              // golden cOffSet.h:462',
        'TLabel *MySingleOffsetLabY[2][8];              // golden cOffSet.h:463',
        'bool bUseUpdate=false;                         // golden cOffSet.h:465',
        'double dTemp=0;                                // golden cOffSet.h:470',
        'bool fShow=false;                              // golden cOffSet.h:475（本 TU 自己的：頁面沒有關表單事件）',
        'int iNowOffsetSel=-1;                          // golden cOffSet.h:476（建構子 :371 設 -1）',
        'bool bEnterSpecialOffset=false;                // golden cOffSet.h:477',
        '#define LastFileName (fOffSet->LastFileName)   // golden cOffSet.h:478 —— 移植樹 GetOffsetPath／ReadFile 用 fOffSet 那一份',
        'int iSpecialOffSetSel=-1;                      // golden cOffSet.h:479（建構子 :372 設 -1）',
        'int iIndexChange=0;                            // golden cOffSet.h:480',
        'int iOffsetMap[OfsTotal];                      // golden cOffSet.h:481',
        # 移植樹已有本體的 golden 方法（見檔頭）
        '#define GetOffsetPath fOffSet->GetOffsetPath   // golden :1430 —— 移植樹 cOffSet.cpp:188（live）',
        '#define Tri_Position_Offset fOffSet->Tri_Position_Offset   // golden :3530 —— 移植樹 forms/fOffSet.cpp:381（live）',
        # C 路入口的控制旗標（NOT golden）
        'bool OS_bSkipReadFile=false;   // 開頁逐一切換選取時略過按鈕事件裡的 ReadFile（檔案沒變；見檔頭）',
        'bool OS_bSelRefused=false;     // SpBotSelClick 被 KYEC barcode 拒（golden return）',
        'int  OS_iActivePage=0;         // spbSaveClick 的 PageControl1->ActivePageIndex（0＝In Out Arm，1＝Index／Tray）',
        'bool OS_bPartOnly=false;       // spbSaveClick 只跑到 SaveFile（多組存檔的前面幾組）',
        'bool OS_bTailOnly=false;       // spbSaveClick 跳過 SaveFile、只跑尾端（ReadFile 起，全部組存完後一次）',
        'void OS_PortReadFile() { if(!OS_bSkipReadFile) fOffSet->ReadFile(); }   // golden ReadFile() → 移植樹 fOffSet->ReadFile()（cOffSet.cpp:367）',
        '#define ReadFile OS_PortReadFile   // FileRW/Offset_File_C.cpp 在 include 其他 header 之前 #undef',
        # InArmSuck／OutArmSuck（TMyKitSuck，aHotPlateSubstrate.h 與 HTEditList.h 衝突）→ FileRW/_KitSuck.cpp 轉接
        '#define InArmSuck OS_SuckOf(0)    // golden InArmSuck.iMotRow／iMotCol／HasIC() → FileRW/_KitSuck.cpp',
        '#define OutArmSuck OS_SuckOf(1)   // golden OutArmSuck.…（同上）',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fOffSet.h', 'csystem.h',
                 'cinitial.h', 'BarcodeReader.h', 'canary_support.h'],
    'decls': [
        'extern bool authMainForm[12];            // cAuthority.h:56（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）',
        'void GetLimitAuth();                     // cAuthority.h:77',
        'bool FileRW_Offset_IsRun2DCheck();       // FileRW/Offset_File_C.cpp 下半（fContactForm->IsRun2DCheck()）',
        'void FileRW_ArmSuckDims(int which, int* row, int* col);   // FileRW/_KitSuck.cpp（0＝InArmSuck，1＝OutArmSuck）',
        'bool FileRW_ArmSuckHasIC(int which);                      // FileRW/_KitSuck.cpp',
        '// golden TMyKitSuck 用到的三樣（iMotRow／iMotCol／HasIC()）的轉接物件：InArmSuck／OutArmSuck 巨集展開成它',
        'struct OS_Suck { int which, iMotRow, iMotCol; bool HasIC() const { return FileRW_ArmSuckHasIC(which); } };',
        'static inline OS_Suck OS_SuckOf(int w) { OS_Suck s; s.which=w; FileRW_ArmSuckDims(w, &s.iMotRow, &s.iMotCol); return s; }',
    ],
    'overrides': [],
    # //AI(W906-EVB3) 20260928 [W906]：OS-1 WS form.event（Steven 20260928「任何畫面的事件, 都是我們做」）。FileRW/Offset_File.cpp 檔尾包一層再註冊
    #   （別名頁 tag Setup.OffSet；部位鈕 53 顆的 SpBotSelClick 參數是 int，產生器的 events 收不了 → 那 53 列在入口檔手寫）。
    'events': [('sb_AutoOffsetUp', 'click', 'sb_AutoOffsetUpClick'), ('sb_AutoOffsetDown', 'click', 'sb_AutoOffsetDownClick'),
               ('sb_AutoOffsetRight', 'click', 'sb_AutoOffsetRightClick'), ('sb_AutoOffsetLeft', 'click', 'sb_AutoOffsetLeftClick')],
}
