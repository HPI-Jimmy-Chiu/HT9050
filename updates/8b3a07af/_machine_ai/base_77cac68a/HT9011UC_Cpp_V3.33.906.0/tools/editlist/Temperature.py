# -*- coding: utf-8 -*-
# tools/editlist/Temperature.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only Temperature
#
# Steven 團隊 20260925：Temperature（SYSTEM_TEMPERATURE，<recipe>\Temperature.Data ＋ Tester.Data [InitialMode]
# ＋ DefineTemp\*.Data ＋ Config\ATC.ini [Setup] iCheckSameTempTime）—— golden TfTemp_Set（uTemp_Set.cpp，912，7085 行）。
# 沒有 HTEditList：存檔鈕 spbSaveClick（:4238）自己檢查 → SaveSetupFile（:4606，逐鍵 WriteIniData）→ DefineTemp 補償檔
# → SaveLastSetIni → BackupSetupFile → ReadTempFile(true) → DoIniDataToForm(true)。
# 開頁 FormShow（:433）照 golden：ReadTempFile(true)（:1986，尾端 DoIniDataToForm(bUpdateAll)）＋顯示／權限。
#
# adopt：移植樹 forms/fTemp_Set.h 的 852 個 golden 同名同型別元件（含子類別 TfTemp_SetTagEdit／TagButton）（`TEdit *x = new TEdit();`，產生器的 adopt 只認
# `new vclcompat::T()`，所以這裡自己列、在 TS_AdoptPortWidgets 裡 ELKeep）＋ myTempPal（golden :1019，每通道一個
# TMyTempPanel）直接用移植樹 fTemp_Set->myTempPal（fTemp_Set->Init() ＝ golden 建構子 :95-408 建的那一份）。
# 理由：移植樹其他程式讀寫同一批元件（fBuilder/csystem/SECS 呼叫 fTemp_Set->DoIniDataToForm＋SaveSetupFile，
# atester.cpp:2245 寫 edtIdleTime_Mid…、Automation 寫 edWorkTemp/edSoakTime、cprod.cpp:4024 rgIndexHeatMode）。
# myTempPal 的每個資料元件另外以具名替身登記 "myTempPal<i>_<golden 成員名>"（i＝tc* 通道索引 0..tcTotalCount-1）
# ＝ 頁面元件 id 合約；FileRW/Temperature.cpp 的 FileRW_Temperature_BootPanels()（fTemp_Set->Init() 之後）登記。
import os
import re

# gen_editlist.py 用 exec 載入本檔（沒有 __file__）；它在 HT9011UC_Cpp_V3.33.906.0 底下跑
_ROOT = os.path.abspath('.') if os.path.exists(os.path.join('forms', 'fTemp_Set.h')) else r'D:\HT9045\HT9011UC_Cpp_V3.33.906.0'
_GOLDEN = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_N = '"TfTemp_Set"'

# golden TMyTempPanel（MyTempPanel.h）裡 golden 三個方法會讀寫的資料元件（TEdit）與容器（TPanel）
PANEL_EDITS = ['edLow', 'edMid', 'edLowbase', 'edBase', 'edHighBase', 'edSHighBase',
               'edPreOffset', 'edPreOfsTime', 'edAfterOfs',
               'edKit_Low', 'edKit_Mid', 'edKit_Lowbase', 'edKit_Base', 'edKit_HighBase',
               'edOffset', 'edSingleLimit', 'edIndiTemp', 'edInitTempOffset', 'edEOTTempOffset']
PANEL_CONTAINERS = ['palTemp']


def _adopt_list():
    """golden header 與移植樹 forms/fTemp_Set.h 同名同型別的元件（移植樹子類別 TfTemp_SetTag* 也算，基底相同）"""
    g = open(os.path.join(_GOLDEN, 'uTemp_Set.h'), 'rb').read().decode('cp950', errors='replace')
    m = re.search(r'\bclass\s+(?:PACKAGE\s+)?TfTemp_Set\b[^;{]*\{', g)
    W = {mm.group(2): mm.group(1) for mm in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', g[m.end():], re.M)}
    p = open(os.path.join(_ROOT, 'forms', 'fTemp_Set.h'), encoding='utf-8').read()
    base = {'TfTemp_SetTagEdit': 'TEdit', 'TfTemp_SetTagButton': 'TSpeedButton'}
    KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
             'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
             'TListBox', 'TDateTimePicker'}
    out = []
    for mm in re.finditer(r'(\w+)\s*\*\s*(\w+)\s*=\s*new\s+(?:vclcompat::)?(\w+)\s*\(\s*\)', p):
        t, n = base.get(mm.group(3), mm.group(3)), mm.group(2)
        if n in W and W[n] == t and t in KNOWN:
            out.append(n)
    return sorted(out)



ADOPT = _adopt_list()

# ----------------------------------------------------------------------
# golden 原檔與方法範圍（給下面自動產生 replace 用；行號＝golden uTemp_Set.cpp 的行號）
# ----------------------------------------------------------------------
_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_MAPPED = {'TTrackBar': 'filerw::ELTrackBar', 'TUpDown': 'filerw::ELTrackBar',
           'TDateTimePicker': 'filerw::ELDateTimePicker', 'TStringGrid': 'filerw::ELStringGrid'}
_G = open(os.path.join(_GOLDEN, 'uTemp_Set.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_GH = open(os.path.join(_GOLDEN, 'uTemp_Set.h'), 'rb').read().decode('cp950', errors='replace')
_m = re.search(r'\bclass\s+(?:PACKAGE\s+)?TfTemp_Set\b[^;{]*\{', _GH)
_W = {}
for _mm in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _GH[_m.end():], re.M):
    _W[_mm.group(2)] = _MAPPED.get(_mm.group(1)) or (_mm.group(1) if _mm.group(1) in _KNOWN else 'TControl')
_WRE = re.compile(r'(?<![\w.>])(?<!::)(' + '|'.join(sorted(map(re.escape, _W), key=len, reverse=True)) + r')\b(?!\s*::)')


def _span(name):
    """golden TfTemp_Set::name 的 (簽名行, 結束 '}' 行)，1-based"""
    for i, l in enumerate(_G):
        if re.match(r'^[A-Za-z].*\bTfTemp_Set::' + name + r'\s*\(', l):
            d, started = 0, False
            for j in range(i, len(_G)):
                code = re.sub(r'//.*', '', re.sub(r'"(\\.|[^"\\])*"', '""', _G[j]))
                d += code.count('{') - code.count('}')
                started = started or '{' in code
                if started and d == 0:
                    return i + 1, j + 1
    raise SystemExit('golden TfTemp_Set::%s not found' % name)


def _split(line):
    """[(是程式碼?, 文字)]：字串常值與 // 註解不改寫（同 gen_editlist.split_code）"""
    out, i, n, buf = [], 0, len(line), ''
    while i < n:
        c = line[i]
        if line.startswith('//', i):
            if buf: out.append((True, buf)); buf = ''
            out.append((False, line[i:])); return out
        if c in '"\'':
            if buf: out.append((True, buf)); buf = ''
            j = i + 1
            while j < n and line[j] != c:
                j += 2 if line[j] == '\\' else 1
            out.append((False, line[i:j + 1])); i = j + 1; continue
        buf += c; i += 1
    if buf: out.append((True, buf))
    return out


def _code(line):
    return ''.join(t for c, t in _split(line) if c)


_IS33 = '(ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_33 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_35 || ATC_InterfaceForm->iATC_MODE_TYPE==ATC_TYPE_36)'


def _rewrite(line):
    """一行 golden → 替身版（產生器的規則，但 ':' 後面的元件名也改寫；註解丟掉）"""
    out = []
    for is_code, t in _split(line):
        if not is_code:
            if not t.startswith('//'):
                out.append(t)
            continue
        t = re.sub(r'ATC_InterfaceForm->IS_ATC33\s*\(\s*\)', _IS33, t)
        t = re.sub(r'\bShowMyMessageBox_YES_NO\s*\(', 'filerw::ELAsk(', t)
        t = re.sub(r'\bShowMyMessage\s*\(', 'filerw::ELMessage(', t)
        t = _WRE.sub(lambda m: 'EL<%s>(%s, "%s")' % (_W[m.group(1)], _N, m.group(1)), t)
        out.append(t)
    return ''.join(out).strip()


# 方法（gen_editlist 的 methods）＋它們在 golden 的範圍
METHODS = ['FormShow', 'SetTempPanelCaption', 'ReadTempFile', 'DoIniDataToForm', 'UpDateEdit',
           'cbATCReferTempSensorClick', 'rbATCActiveOnClick', 'rbATC70ActiveOnClick',
           'FormClose', 'spbSaveClick', 'SaveSetupFile', 'CheckTempSettingChange', 'DisplayTargetTempEdit']
_SPAN = {m: _span(m) for m in METHODS}


def _meth_of(gl):
    for m, (a, b) in _SPAN.items():
        if a < gl < b:
            return m
    return None


# 手寫的 replace／blocks（golden 行號，逐條原因）
_MANUAL_REPLACE = []   # 見下方 STRUCT 前的清單（由 _manual() 填）
_MANUAL_LINES = set()


def _auto_replace():
    """自動等價改寫（產生器沒改到的兩種）：
       (a) 三元式 '?x:元件' 的 ':元件'（產生器的 lookbehind 排除 ':'，為了避開 '::'）
       (b) ATC_InterfaceForm->IS_ATC33()：912 golden 的方法（ATC_Handler_Side.cpp:4103-4112，
           iATC_MODE_TYPE==33||35||36）；移植樹的 ATC_InterfaceForm 是 acarry_shims.h 的 TATC_InterfaceFormShim，
           只有 iATC_MODE_TYPE 一個欄位 → 展開成同一個判斷。"""
    out = []
    for m, (a, b) in _SPAN.items():
        for gl in range(a + 1, b):
            if gl in _MANUAL_LINES:
                continue
            raw = _G[gl - 1]
            c = _code(raw)
            if c.lstrip().startswith('#'):
                continue
            why = []
            if re.search(r'(?<!:):\s*(' + '|'.join(map(re.escape, _W)) + r')\b(?!\s*::)', c):
                why.append("三元式 ':元件' 產生器沒改寫（lookbehind 排除 ':'）→ 等價改寫")
            if 'IS_ATC33' in c:
                why.append('IS_ATC33()：912 golden ATC_Handler_Side.cpp:4103-4112 的判斷（33/35/36）展開；移植樹 shim 沒有這個方法')
            if why:
                out.append((m, gl, gl, '；'.join(why), _rewrite(raw)))
    return out


def _adopt_fn():
    body = ' '.join('filerw::ELKeep(%s, "%s", fTemp_Set->%s);' % (_N, n, n) for n in ADOPT)
    return 'static void TS_AdoptPortWidgets() { %s }   // %d 個（forms/fTemp_Set.h ↔ golden uTemp_Set.h 同名同型別）' % (body, len(ADOPT))


# TMyTempPanel（golden MyTempPanel.h）的替身包裝：成員是移植樹 fTemp_Set->myTempPal[i] 那個物件的元件（同一份），
# Caption 是 golden __property（read=GetCaption, write=SetCaption，移植樹 MyTempPanel.h 收成兩個方法）
_PANEL_STRUCT = [
    '// golden MyTempPanel.h TMyTempPanel 的替身包裝（Steven 團隊 20260925）：golden 程式寫 myTempPal[i]->edBase->Text、',
    '// ->Caption=… —— 成員就是移植樹 fTemp_Set->myTempPal[i]（fTemp_Set->Init()＝golden 建構子 :152 建的）那個物件的元件，',
    '// Caption 轉成移植樹的 SetCaption／GetCaption（golden __property Caption={read=GetCaption, write=SetCaption}）。',
    'struct TS_TMyTempPanel {',
    '    TMyTempPanel* p;',
    '    struct CaptionProp { TMyTempPanel* p; CaptionProp& operator=(const AnsiString& s) { p->SetCaption(s); return *this; } operator AnsiString() const { return p->GetCaption(); } };',
    '    CaptionProp Caption;',
    '    int &iIndexTag, &iOffsetByRecipeMaxLimit, &iOffsetByRecipeMinLimit;',
    '    TPanel *&palTemp, *&palLine; TLabel *&labName;',
    '    TEdit ' + ', '.join('*&' + e for e in PANEL_EDITS) + ';',
    '    void SetEnable(bool b) { p->SetEnable(b); }',
    '    void SetParent(TTabSheet* t) { p->SetParent(t); }',
    '    void SetIndexTag(int t) { p->SetIndexTag(t); }',
    '    explicit TS_TMyTempPanel(TMyTempPanel* q) : p(q), Caption{q}, iIndexTag(q->iIndexTag),',
    '        iOffsetByRecipeMaxLimit(q->iOffsetByRecipeMaxLimit), iOffsetByRecipeMinLimit(q->iOffsetByRecipeMinLimit),',
    '        palTemp(q->palTemp), palLine(q->palLine), labName(q->labName),',
    '        ' + ', '.join('%s(q->%s)' % (e, e) for e in PANEL_EDITS) + ' {}',
    '};',
    'static TS_TMyTempPanel* TS_myTempPal[tcTotalCount] = {};   // golden uTemp_Set.h:1019 TMyTempPanel *myTempPal[tcTotalCount]',
]


def _panel_fn():
    s = ['static void TS_AdoptPanels() { AnsiString n; for(int i=0; i<tcTotalCount; i++) { if(fTemp_Set->myTempPal[i]==NULL) continue;',
         'if(TS_myTempPal[i]==NULL) TS_myTempPal[i]=new TS_TMyTempPanel(fTemp_Set->myTempPal[i]);']
    for m in PANEL_EDITS + PANEL_CONTAINERS:
        s.append('n.sprintf("myTempPal%%d_%s", i); filerw::ELKeep(%s, n.c_str(), fTemp_Set->myTempPal[i]->%s);' % (m, _N, m))
    # 權限：每個資料元件的父容器＝同一通道的 palTemp（golden MyTempPanel.cpp 建構子 ->Parent=palTemp）；
    # palTemp->Visible=false 的通道（golden FormShow／UpDateEdit 關掉的）頁面送來的值不收
    s.append('AnsiString pal; pal.sprintf("myTempPal%d_palTemp", i);')
    for m in PANEL_EDITS:
        s.append('{ n.sprintf("myTempPal%%d_%s", i); const char* pr[1][2]={{n.c_str(), pal.c_str()}}; filerw::ELSetParents(%s, pr, 1); }' % (m, _N))
    s.append('} }   // golden uTemp_Set.h:1019 myTempPal[tcTotalCount]；頁面 id 合約 myTempPal<i>_<成員>（i＝tc* 通道索引）')
    return ' '.join(s)


# golden cTesterIF.cpp:563 TFTestIF::ReadTestIFFile 的 [InitialMode] 段（:713-779）逐行照抄 —— 本表單顯示／存的那一組
# TestIF_File 欄位（DoIniDataToForm :3563-3600、SaveSetupFile :5103-5150）。整支在移植樹 GATE (F-5)。
_TIF = open(os.path.join(_GOLDEN, 'cTesterIF.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _tif_fn():
    a, b = 713, 779
    assert 'bEveryFirstDeviceUseInitialDelay' in _TIF[a - 1] and 'dInitialDelay_10_RT' in _TIF[b - 1], 'cTesterIF.cpp 行號變了'
    assert 'szDir=DataPath+S;' in _TIF[568] and 'Tester.Data' in _TIF[569], 'cTesterIF.cpp :569-570 變了'
    out = ['// golden cTesterIF.cpp:563-570（路徑）＋:713-779（[InitialMode]）—— FTestIF->ReadTestIFFile() 的子集（其餘 GATE F-5）',
           'static void TS_ReadTestIFFile_InitialMode()',
           '{']
    for gl in list(range(566, 571)) + list(range(a, b + 1)):
        out.append('    ' + _TIF[gl - 1].rstrip() + ('   // golden cTesterIF.cpp:%d' % gl if not _TIF[gl - 1].strip().startswith('#') and _TIF[gl - 1].strip() else ''))
    out.append('}')
    return out


# ----------------------------------------------------------------------
# 手寫的等價取代（golden 行號＋原因）。(方法, 起, 迄, 原因, 取代碼)；取代碼原樣輸出（元件要自己寫 EL<>）。
# ----------------------------------------------------------------------
def _find(meth, pat, nth=0):
    """golden 方法範圍內第 nth 個符合 pat（regex，比對程式碼部分）的行號"""
    a, b = _SPAN[meth]
    hits = [gl for gl in range(a + 1, b) if re.search(pat, _code(_G[gl - 1]))]
    if len(hits) <= nth:
        raise SystemExit('Temperature.py: %s 找不到 %r（第 %d 個）' % (meth, pat, nth))
    return hits[nth]


def _all(meth, pat):
    a, b = _SPAN[meth]
    return [gl for gl in range(a + 1, b) if re.search(pat, _code(_G[gl - 1]))]


_MANUAL_REPLACE = []


def _R(meth, a, b, why, code):
    _MANUAL_REPLACE.append((meth, a, b, why, code))
    for gl in range(a, b + 1):
        _MANUAL_LINES.add(gl)


def _UI(meth, pat, why, every=True):
    for gl in (_all(meth, pat) if every else [_find(meth, pat)]):
        _R(meth, gl, gl, why, ';')


_UI('FormShow', r'^\s*Caption\s*=', 'Caption：視窗標題')
_UI('FormShow', r'^\s*(Left|Top)\s*=', 'Left／Top：視窗位置')
_UI('FormShow', r'\bSetBasePointIMG\s*\(', 'SetBasePointIMG：基準點示意圖（TImage，純畫面）')
_UI('FormShow', r'\bShowLineOnTop\s*\(', 'ShowLineOnTop：Panel->BringToFront（Z 序，純畫面）')
_UI('FormShow', r'->Repaint\s*\(', 'Repaint：重畫（純畫面）')
for _m in ('FormShow', 'UpDateEdit'):
    _UI(_m, r'->palTemp->Align\s*=', 'palTemp->Align：面板排版（vclcompat TPanel 沒有 Align，純畫面）')

# FormShow :1062-1063 —— InArmSuck／MOT[MMPlate1/2]（aHotPlateSubstrate.h）與 HTEditList.h 衝突、本 TU 看不到。
# 改用移植樹 HasICUnderHotPlate()（csystem.cpp:13412：MOT[MMPlate1/2]||InArmSuck||OutArmSuck||Shuttle||Index||RotateKit），
# 是 golden 條件的**超集**（多了 OutArmSuck／RotateKit）→ 只會更常把 edSoakTime／edWorkTemp 鎖住（保守）。
_a = _find('FormShow', r'InArmSuck\.HasIC\(\)')
_R('FormShow', _a, _a + 1,
   'InArmSuck／MOT[] 本 TU 看不到（aHotPlateSubstrate.h 與 HTEditList.h 衝突）；HasICUnderHotPlate() 是超集（多 OutArmSuck／RotateKit），保守鎖定',
   'if(IndexHasIC() || ShuttleHasIC() || HasICUnderHotPlate())')

# ReadTempFile :2037／:2040 —— fMain->palIndivisual 移植樹 fMain 沒有（主畫面面板，純畫面）；:2040 讀它剛設的值 → 等價展開
_a = _find('ReadTempFile', r'fMain->palIndivisual->Visible\s*=')
_R('ReadTempFile', _a, _a, 'fMain->palIndivisual：主畫面面板，移植樹 fMain 沒有（純畫面）', ';')
_a = _find('ReadTempFile', r'=\s*!fMain->palIndivisual->Visible')
_R('ReadTempFile', _a, _a, 'fMain->palIndivisual->Visible 就是上一句 :%d 設的值 → 等價展開' % (_a - 3),
   'fMain->edWorkTemperBase->Visible=!(CosFunction.bUseIndividulTempSet && Temperature.bUseIndividualTemp);')

# ReadTempFile :2193 —— rgIndexHeatMode->Controls[i]->Visible：vclcompat TRadioGroup 沒有 Controls[]。golden 全樹只有
# cprod.cpp:3818-3823（CustomerFunctionSelect）設它 → 同一組公式（TS_IndexHeatModeItemVisible，decls）
_a = _find('ReadTempFile', r'rgIndexHeatMode->Controls\[')
_R('ReadTempFile', _a, _a,
   'rgIndexHeatMode->Controls[i]->Visible：vclcompat 沒有 Controls[]；golden 唯一設定點 cprod.cpp:3818-3823 的公式（TS_IndexHeatModeItemVisible）',
   'if(TS_IndexHeatModeItemVisible(Temperature.iIndexHeatMode)==false)')

# ReadTempFile :3178 —— COM2->ATCInitialTask()（golden rs232.cpp:4260）：只在 eATCSiliconType＋ActiveCooling＋Dual/Single site
# 時重設 ATC 初始化任務旗標（iATCInitialTask／iATCProcessTask／bATCInitialOK）。TCOM2 移植樹沒有 → 同條件才記 todo
_a = _find('ReadTempFile', r'COM2->ATCInitialTask\s*\(')
_R('ReadTempFile', _a, _a, 'COM2->ATCInitialTask：TCOM2 移植樹沒有（golden rs232.cpp:4260，只在 eATCSiliconType 生效）',
   'if(ATC_SYSTEM==eATCSiliconType && Temperature.bATCActiveCooling==true && (TestIF.iTestMode==DualSite || TestIF.iTestMode==SingleSite)) '
   'filerw::ELTodo("golden rs232.cpp:4260 COM2->ATCInitialTask (ATC initial-task flags, eATCSiliconType) not ported");')

# ReadTempFile —— ATKRecipeInfo->SaveFile()（golden 尾端，Steven 20170901 For ATK）：移植樹沒有建立 ATKRecipeInfo（database.cpp:149 註解掉）
# → 空指標。golden ATK_RECIPE_INFO::SaveFile（移植樹 cprod.cpp:497）非 CC_AMKOR_Korea／China 一進去就 return。
# AI(W906-FRW-S88) 20260926：S88 工程師發現；比照 tools/editlist/TestIF_File_TesterIF.py 同一條取代（指標在就照呼叫；不在時 AMKOR 才記 todo）
_a = _find('ReadTempFile', r'ATKRecipeInfo->SaveFile\s*\(')
_R('ReadTempFile', _a, _a, 'ATKRecipeInfo：移植樹沒有建立（database.cpp:149 註解掉）→ 空指標。golden ATK_RECIPE_INFO::SaveFile（移植樹 cprod.cpp:497）'
   '非 CC_AMKOR_Korea／China 一進去就 return → 指標在就照呼叫；不在時 AMKOR 才記 todo（其他客戶 golden 本來就什麼都不做）',
   'if(ATKRecipeInfo) ATKRecipeInfo->SaveFile(); else if(CUSTOMER_CODE==CC_AMKOR_Korea || CUSTOMER_CODE==CC_AMKOR_China) '
   'filerw::ELTodo("golden uTemp_Set.cpp:%d ATKRecipeInfo->SaveFile() (AMKOR Information.txt / D:\\\\eRMS) not run -- ATKRecipeInfo is not created in the port (database.cpp:149)");' % _a)

# DoIniDataToForm :3186／SaveSetupFile :5162 —— FTestIF->ReadTestIFFile()（golden cTesterIF.cpp:563）在移植樹 GATE (F-5)
# （porting-gaps.md 十四）。這個表單顯示／存檔的是 Tester.Data [InitialMode] 那一組 TestIF_File 欄位（DoIniDataToForm
# :3563-3600 讀、SaveSetupFile :5103-5150 寫）；不重讀的話頁面拿到的是開機初值，存檔會把初值寫回 Tester.Data。
# → 只跑 golden ReadTestIFFile 的 [InitialMode] 那一段（TS_ReadTestIFFile_InitialMode，decls，逐行照抄 cTesterIF.cpp），
#   其餘（Tester Type 回寫、TCP 計時器、fLotInfo／fSCKART…）記 todo。
for _m in ('DoIniDataToForm', 'SaveSetupFile'):
    _a = _find(_m, r'FTestIF->ReadTestIFFile\s*\(')
    _R(_m, _a, _a, 'FTestIF->ReadTestIFFile 移植樹 GATE (F-5)：只跑 golden cTesterIF.cpp:713-779 的 [InitialMode] 段（本表單顯示／寫的欄位）',
       'TS_ReadTestIFFile_InitialMode(); filerw::ELTodo("golden cTesterIF.cpp:563 FTestIF->ReadTestIFFile only the [InitialMode] block (:713-779) re-read -- rest gated F-5 (porting-gaps 14)");')

# spbSaveClick :4244 —— Close()：A02 權限不足關表單
_a = _find('spbSaveClick', r'^\s*Close\s*\(\s*\)')
_R('spbSaveClick', _a, _a, 'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（ack.trace）', 'filerw::ELMark("closed");')

# spbSaveClick :4319 —— FormHS->CheckTempOffset(0,0,true) 發 WAR15194 告警後 return（golden 行為照做，另記 trace）
_a = _find('spbSaveClick', r'FormHS->CheckTempOffset\(0,\s*0,\s*true\)')
_R('spbSaveClick', _a, _a, 'CheckTempOffset(0,0,true) 發 WAR15194（golden），另記 trace 讓頁面知道為什麼沒存',
   'FormHS->CheckTempOffset(0, 0, true); filerw::ELMark("WAR15194 CheckTempOffset over limit -- not saved");')

# spbSaveClick :4558／:4562 —— SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有（同 FileRW/Offset_File.cpp:725）
for _gl in _all('spbSaveClick', r'SetTempShiftOffsetPeriod\s*\('):
    _R('spbSaveClick', _gl, _gl, 'SetTempShiftOffsetPeriod（ASE_CL）移植樹沒有本體（同 FileRW/Offset_File.cpp:725）',
       'filerw::ELTodo("golden uTemp_Set.cpp SetTempShiftOffsetPeriod (CC_ASE_CL) not in the port");')

STRUCT = {
    'struct': 'Temperature',
    'prefix': 'TS',
    'class': 'TfTemp_Set',
    'cpp': 'uTemp_Set.cpp',
    'h': 'uTemp_Set.h',
    'files': ['Temperature.Data', 'Tester.Data', r'DefineTemp\*.Data', r'Config\ATC.ini'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick', 'SaveSetupFile', 'CheckTempSettingChange'],
    'params': {'FormShow': '', 'FormClose': '', 'spbSaveClick': '',
               'cbATCReferTempSensorClick': '', 'rbATCActiveOnClick': '', 'rbATC70ActiveOnClick': ''},
    'rettype': {'CheckTempSettingChange': 'bool'},
    'members': ['#define myTempPal TS_myTempPal                 // golden uTemp_Set.h:1019（替身包裝，指向移植樹 fTemp_Set->myTempPal）',
                'bool fShow=false;   // golden uTemp_Set.h:1017 —— 本 TU 自己的（不是 fTemp_Set->fShow：那是 WebStart.cpp:1945／Command.cpp:10247 的 START 互鎖訊號，網頁開著不能擋 START；同 ArmSpeed_File 等 C 路）',
                '#define iTempMoldSet (fTemp_Set->iTempMoldSet)  // golden uTemp_Set.h:1018',
                '#define bNeedChange (fTemp_Set->bNeedChange)    // golden uTemp_Set.h:1043',
                # 沒有轉的 golden 方法：呼叫移植樹 fTemp_Set 的同名方法（uTemp_Set.cpp 已翻）
                '#define MaxTempSetting() (fTemp_Set->MaxTempSetting())   // golden :5692（移植樹 uTemp_Set.cpp，純計算）',
                '#define MinTempSetting() (fTemp_Set->MinTempSetting())   // golden :5765（同上）',
                '#define ControlATC60AirFlow(...) (fTemp_Set->ControlATC60AirFlow(__VA_ARGS__))   // golden :6240 ⚠ 硬體（ATC6.0 氣流）；移植樹本體 SAFETY GATE (S13) 全擋',
                '#define FormHS TS_FormHS()                      // golden HS_Function.h FormHS（移植樹 FormHS 名字被 SCK_ART_Remainder.h 的 stub 佔走）'],
    'replace': _MANUAL_REPLACE + _auto_replace(),
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h', 'forms/fTemp_Set.h',
                 'forms/fATCHandlerSide.h', 'forms/fHS.h', 'forms/fDynamicTemp.h', 'atester_shims.h',
                 'ATC/ATCInterface.h', 'TempCtrl/TriTemp.h', 'database.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'decls': ['extern bool bAuthCriticalPara[26];   // cAuthority.h:64（golden cAuthority.h；cAuthority.h 與 HTEditList.h 衝突）',
              'void GetLimitAuth();                  // cAuthority.h:77',
              '// csystem.h／cinitial.h／canary_support.h 帶進 aHotPlateSubstrate.h 或另一份 LAST_GENERAL_SET → 只前置宣告',
              'bool HasICUnderMachine();             // csystem.h:105',
              'bool HasICUnderHotPlate();            // csystem.h:110',
              'bool ShuttleHasIC();                  // csystem.h:135',
              'bool IndexHasIC();                    // csystem.h:140',
              'void InitialHeaterDoor();             // cinitial.h:42',
              'void RecordProcess(AnsiString S, AnsiString S2="");   // canary_support.h:70（golden cMyDB.h:63）',
              'int  ShowErrorMessage(AnsiString Code, int KCode, int Pos, bool bDuplicateErr=false, AnsiString errPart=" ");   // canary_support.h:66',
              '// acarry_shims.h 帶進 aHotPlateSubstrate.h → 照抄同一個類別定義（ODR：逐 token 相同）',
              'class TATC_InterfaceFormShim',
              '{',
              'public:',
              '    int iATC_MODE_TYPE;',
              '    TATC_InterfaceFormShim();',
              '};',
              'extern TATC_InterfaceFormShim *ATC_InterfaceForm;   // acarry_shims.h:115',
              '// golden uTemp_Set.cpp:72-93 檔案層級全域（移植樹 uTemp_Set.cpp:248-270 定義、fTemp_Set->Init() 填）',
              'extern TEdit *ATCOffsetEdit[32];',
              'extern TEdit *ATCPackageOffsetEdit[3];',
              'extern TEdit *ATCPackageTempEdit[3];',
              'extern TEdit *ATC_FFCOffsetOnTimeEdit[2][10];',
              'extern TEdit *ATC_FFCOffsetOffTimeEdit[2][10];',
              'extern TEdit *ATC_FFCOffsetEdit[2][10];',
              'extern TCheckBox *ATC_FFCPointUse[2][10];',
              'extern TStringList *sATC_CH_Tj;',
              'extern TCheckBox *ZoneTempUse[4];',
              'extern TEdit *ZoneTempSetting[4];',
              'extern TCheckBox *MultiSensorOffsetUse[4];',
              'extern TEdit *ATC_MultiSensorOffsetEdit[32];',
              'extern TfDynamicTemp *fDynamicTemp;   // DynamicTemp.cpp:58',
              '// golden FormHS->CheckTempOffset（HS_Function.cpp:3958）：移植樹 forms/fHS.cpp:804 TFormHS::CheckTempOffset 有本體（純門檻比較＋WAR15194），',
              '// 但全域名 FormHS 被 Automation/SCK_ART_Remainder.h:629 的 stub 佔走 → 本 TU 自己的一個實例（函式內 static 狀態照舊）',
              'static TFormHS* TS_FormHS() { static TFormHS* p = new TFormHS(); return p; }',
              '// golden cprod.cpp:3818-3823（CustomerFunctionSelect）決定 rgIndexHeatMode 各選項的 Visible；vclcompat TRadioGroup 沒有 Controls[]',
              'static bool TS_IndexHeatModeItemVisible(int i) {',
              '    switch(i) {',
              '    case HeadOnly:          return true;',
              '    case ChamberOnly:       return (ATC_SYSTEM==eATCUninstall);',
              '    case HeadChamber:       return (ATC_SYSTEM==eATCUninstall && IniConfig.bNoHeadaddChamberOption==false);',
              '    case SocketChamber:     return (ATC_SYSTEM==eATCUninstall);',
              '    case HeadSocket:        return (IniConfig.bHeadSocketMode);',
              '    case HeadChamberSocket: return (ATC_SYSTEM==eATCUninstall && IniConfig.bHeadChamberSocketMode);',
              '    default:                return true;   // 之後 Items->Add 的選項（VCL 新建 radio button 預設 Visible）',
              '    }',
              '}',
              ] + _tif_fn() + _PANEL_STRUCT + [_adopt_fn(), _panel_fn(),
              'static bool TS_PortPanelsBuilt() { return fTemp_Set!=NULL && fTemp_Set->myTempPal[0]!=NULL; }   // FileRW/Temperature.cpp 用（myTempPal 巨集之前）'],
    'blocks': [],
    'overrides': [],
}
