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
           'FormClose', 'spbSaveClick', 'SaveSetupFile', 'CheckTempSettingChange', 'DisplayTargetTempEdit',
           # AI(W906-FRW-S158) 20260927 [W906]：TS-1（Q41 盤點 docs/Q41_INVENTORY_20260927.md §3.9）—— WS form.event 的 golden 處理器
           #   （Steven ★ Q40＝A，RULINGS_20260926 S157），見 STRUCT 'events'。兩支都只有幾行、照 golden 原樣轉（不用 replace）：
           #   rgIndexHeatModeClick（:4232）＝ fShow 時 ReadTempFile(false)（依 rgIndexHeatMode->ItemIndex 選 DefineTemp 補償表，:2900-2907；只分 HeadChamber／其他）；
           #   chkTempCalByRecipeClick（:6333）＝ bTempCalByRecipe 功能開著且 fShow 時 Temperature.bTempCalByRecipe=Checked、
           #   ReadTempFile(false)（:3027 依它改讀 <recipe>\DefineTemperature.Data）、DoIniDataToForm(false)。
           #   golden 怪處照翻：ReadTempFile 尾端 :3157 已經 DoIniDataToForm(bUpdateAll)，:6339 再跑一次（結果相同，ack.todo 會多一筆
           #   FTestIF->ReadTestIFFile 的待辦）。兩支都直接改全域 Temperature.*（golden 同：點下去機台記憶體就換成另一份補償表，
           #   存檔前也是）—— 風險見 FileRW/Temperature.cpp SaveFlow 註解與交件。
           'rgIndexHeatModeClick', 'chkTempCalByRecipeClick',
           # AI(W906-FRW-S158) 20260927 [W906]：TS-7（Q41 盤點 §3.9；St02 FROM_STEVEN §4 20260927 17:21 (b)）—— 換基準點數（5 個 TRadioButton，
           #   uTemp_Set.dfm:10719／:10735／:10751／:10767／:10783 OnClick＝rb1PointClick :421）與「System／Kit Temp. Offset」切換
           #   （rgBasePoint，dfm:10668 OnClick＝rgBasePointClick :6384）都只跑 UpDateEdit（:3708）重排欄位的看得見／可改。
           #   btnSortClick（:5842，dfm:10544）不轉：見 STRUCT 'events' 的註解。加在最後：既有方法的產生碼不動。
           'rb1PointClick', 'rgBasePointClick']
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

# AI(W906-FSHOW-B2) 20260929 [W906]：ReadTempFile :2495 fContact->fShow（Sam 20240118 CC_SIGURD_PeiXing：Contact 畫面開著時 bUseInitTempOffset=false）
#   → 問頁面表 W906_FormShowing（Steven Q51／S168「bcb 的 fShow 的 flag 使用網頁顯示的陣列來替代」、Q-P3 直接生效；成員照傳）。
#   golden 表單名 fContact ＝ 網頁視窗 contact（WebPageTable.cpp kRows，kPgWeb）。V912 的 fContact 是非模態（main.cpp:28344 ShowModal → Show），
#   所以 golden 在 Contact 開著時也會跑到這裡（例 main.cpp:28407 換溫度模式）；網頁開著 Contact 時同樣生效。
_a = _find('ReadTempFile', r'CC_SIGURD_PeiXing\s*&&\s*fContact->fShow')
_R('ReadTempFile', _a, _a,
   'fContact->fShow（Contact 畫面開著沒）→ 問頁面表 W906_FormShowing（golden 表單名 fContact ＝ 網頁視窗 contact；成員照傳；Steven Q51／Q-P3 直接生效）',
   'if(CUSTOMER_CODE==CC_SIGURD_PeiXing && W906_FormShowing("fContact", fContact->fShow))')

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

# ---- AI(W906-FRW-S158) 20260927 [W906]：TS-7 基準點數的 VCL 語意（vclcompat TRadioButton 只是欄位，沒有下面兩件事）----
# VCL TRadioButton.SetChecked(Value)：值有變時設值；設成 true 時先 TurnSiblingsOff（同一個父容器的其他 TRadioButton 取消勾選，
#   不觸發它們的 OnClick），再 Click → OnClick。使用者點一下＝SetChecked(True)；程式寫 ->Checked=true 也一樣會觸發 OnClick。
# golden 靠這個隱含的 OnClick 重排欄位的地方：
#   (a) DoIniDataToForm :3455-3459 依 Temperature.iTempMode 設 rb1..rb6Point —— 這幾行在 bUpdateAll 區塊外（區塊 :3188-3375），
#       TS-1 的 rgIndexHeatModeClick／chkTempCalByRecipeClick → ReadTempFile(false) :3035 讀到另一份補償檔的 [Mode] Points 時，
#       golden 就在這裡換基準點數＋跑 rb1PointClick → UpDateEdit；
#   (b) FormShow :679 iTempMode 不是 1／2／4／8／16 時 rb1Point->Checked=true（在 :665 的 UpDateEdit 之後）。
# 照 VCL 補上（同 _EditList.h ELComboText 補 TComboBox 連動的做法）：值有變才設；設成 true 就呼叫 golden 處理器（Sender＝那顆鈕），
#   處理器開頭補 TurnSiblingsOff（rb1PointClick 的 :423-424 取代）。
_RB = '{"rb1Point", "rb2Point", "rb3Point", "rb5Point", "rb6Point"}'   # golden uTemp_Set.dfm gbBasePoint 底下的 5 顆（Tag 1／2／4／8／16）
_a = _find('rb1PointClick', r'TRadioGroup\s*\*\s*Ptr\s*=')
assert 'SetBasePointIMG(Ptr->Tag)' in _G[_a] and 'UpDateEdit()' in _G[_a + 1], 'uTemp_Set.cpp rb1PointClick 變了'
_R('rb1PointClick', _a, _a + 1,
   'TRadioGroup *Ptr=(TRadioGroup*)Sender 只給 SetBasePointIMG(Ptr->Tag)（基準點示意圖 TImage，純畫面）→ 換成 VCL 在 OnClick 之前做的 '
   'TurnSiblingsOff（gbBasePoint 其他 TRadioButton 取消勾選；vclcompat 沒有）',
   '{ TRadioButton* self = dynamic_cast<TRadioButton*>(Sender); if (self && self->Checked) { static const char* const rbs[5] = %s; '
   'for (int k = 0; k < 5; ++k) { TRadioButton* r = EL<TRadioButton>(%s, rbs[k]); if (r != self) r->Checked = false; } } }' % (_RB, _N))
_a = _find('DoIniDataToForm', r'rb1Point->Checked\s*=\s*Temperature\.iTempMode\s*&\s*0x01')
assert all(('rb%dPoint->Checked' % n) in _G[_a - 1 + k] for k, n in enumerate((1, 2, 3, 5, 6))), 'uTemp_Set.cpp DoIniDataToForm rb*Point 變了'
_R('DoIniDataToForm', _a, _a + 4,
   'rb1..rb6Point->Checked=iTempMode&bit：VCL SetChecked 值有變且設成 true 時 TurnSiblingsOff＋OnClick（rb1PointClick → UpDateEdit）；'
   'vclcompat 沒有 → 照 VCL 補（TS-1 讀到點數不同的補償檔時 golden 靠它重排欄位）',
   '{ static const char* const rbs[5] = %s; static const int bits[5] = {0x01, 0x02, 0x04, 0x08, 0x10}; '
   'for (int k = 0; k < 5; ++k) { TRadioButton* r = EL<TRadioButton>(%s, rbs[k]); const bool v = (Temperature.iTempMode & bits[k]) != 0; '
   'if (r->Checked != v) { r->Checked = v; if (v) TS_rb1PointClick(r); } } }' % (_RB, _N))
_a = _find('FormShow', r'rb1Point->Checked\s*=\s*true')
assert 'Temperature.iTempMode=1' in _G[_a], 'uTemp_Set.cpp FormShow :679 變了'
_R('FormShow', _a, _a,
   'rb1Point->Checked=true（iTempMode 不合法的 default 分支，在 UpDateEdit 之後）：VCL 值有變就 OnClick（rb1PointClick → UpDateEdit）；vclcompat 沒有 → 照 VCL 補',
   '{ TRadioButton* r = EL<TRadioButton>(%s, "rb1Point"); if (!r->Checked) { r->Checked = true; TS_rb1PointClick(r); } }' % _N)

# ---- //AI(W906-EVB3) 20260928 [W906]：TS-2（事件移植批次 B3，docs/EVENT_PORT_BATCH_20260928.md）—— 三支處理器早就在 METHODS 裡
#   （FileRW/Temperature.cpp SaveFlow (2)(3) 存檔前重播），這次只加 'events'，產生碼不動。處理器原文釘住（golden 改了就中止）：
def _expect(gl, text):
    if _G[gl - 1].strip() != text:
        raise SystemExit('Temperature.py: golden uTemp_Set.cpp:%d is %r, expected %r' % (gl, _G[gl - 1].strip(), text))
    return gl


_expect(_SPAN['rbATC70ActiveOnClick'][0], 'void __fastcall TfTemp_Set::rbATC70ActiveOnClick(TObject *Sender)')
_expect(_SPAN['rbATC70ActiveOnClick'][0] + 2, 'if( rbATC70ActiveOn->Checked ==true)')
_expect(_SPAN['rbATCActiveOnClick'][0], 'void __fastcall TfTemp_Set::rbATCActiveOnClick(TObject *Sender)')
_expect(_SPAN['rbATCActiveOnClick'][0] + 3, 'if(ATC_SYSTEM==eNewATCSystem)')
_expect(_SPAN['cbATCReferTempSensorClick'][0], 'void __fastcall TfTemp_Set::cbATCReferTempSensorClick(TObject *Sender)')
_expect(_SPAN['cbATCReferTempSensorClick'][0] + 2, 'if(cbATCReferTempSensor->Checked)')

# ---- //AI(W906-EVB10A) 20260929 [W906]：TS-10（事件批次 B10 part a，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
#   Steven 20260928「如果已經有移植, 就接上」、20260929「照 BCB 的邏輯」）—— Exit 鈕 sbtExit → golden TfTemp_Set::sbtExitClick（uTemp_Set.cpp:5175-5198）：
#   依「ATC Active」（rbATCActiveOn，TS-2 的事件讓伺服器端的勾選跟著頁面）設 bStartATCRun、LotInfo 的 ATC On/Off Line 面板、HonPrec ATC 時
#   ATCInterfaceForm 的 On Line／Off Line 鈕（VCL Click() ＝ 呼叫它的 OnClick：btOnLineClick／btOffLineClick，移植樹 ATC/ATCInterface.cpp:1800／:1807），
#   iATCForHSMode=0、bATCTempAdjustmentOffset=false、bSetTempChange=true（工作檔切換要重送 ATC 參數），Close() → FormClose（:4196）。
#   ⚠ 硬體：btOnLineClick → OnLine()（LoadATCSystem、ATC_SYS.SetOnLine(true)、SetChillerTemperature）／btOffLineClick → OffLine() —— 只在
#   ATC_SYSTEM==eATCHonPrecType（golden 同）；要上機驗（交件列出）。移植樹舊表單翻譯 uTemp_Set.cpp:5412 把這兩行擋著（G-Delegate），這裡照 golden 接上。
#   WS form.event（頁面 D:\HT9045\web\page\ht9045_temp_set_ts1.js 攔 sbtExit）；ack.closed＝true 頁面關視窗。golden ✕ 只跑 FormClose，不跑這支。
#   只加在 STRUCT 的 methods 尾端（既有方法的產生碼不動）。
_SPAN['sbtExitClick'] = _span('sbtExitClick')
_TS_EX = _SPAN['sbtExitClick'][0]
_expect(_TS_EX, 'void __fastcall TfTemp_Set::sbtExitClick(TObject *Sender)')
_TS_EX_DOWN = _expect(_TS_EX + 2, 'sbtExit->Down=false;')
_TS_EX_ON = _expect(_TS_EX + 9, 'ATCInterfaceForm->btOnLine->Click();')
_TS_EX_OFF = _expect(_TS_EX + 17, 'ATCInterfaceForm->btOffLine->Click();')
_TS_EX_CLOSE = _expect(_TS_EX + 22, 'Close();')
_TS_EXIT_REPLACE = [
    ('sbtExitClick', _TS_EX_DOWN, _TS_EX_DOWN, 'sbtExit->Down=false：按鈕放開（純畫面）', ';'),
    ('sbtExitClick', _TS_EX_ON, _TS_EX_ON,
     'ATCInterfaceForm->btOnLine->Click()：VCL TButton::Click() 呼叫 OnClick＝btOnLineClick（golden ATC/ATCInterface.dfm 綁定；移植樹 ATC/ATCInterface.cpp:1800 有本體）',
     'ATCInterfaceForm->btOnLineClick(ATCInterfaceForm->btOnLine);'),
    ('sbtExitClick', _TS_EX_OFF, _TS_EX_OFF,
     'ATCInterfaceForm->btOffLine->Click()：同上，OnClick＝btOffLineClick（移植樹 ATC/ATCInterface.cpp:1807）',
     'ATCInterfaceForm->btOffLineClick(ATCInterfaceForm->btOffLine);'),
    ('sbtExitClick', _TS_EX_CLOSE, _TS_EX_CLOSE,
     'Close()：VCL 同步觸發 OnClose＝FormClose（:4196）→ 伺服器端記 closed（ack.closed，頁面關視窗）再照 golden 跑 FormClose',
     'filerw::ELMark("closed"); TS_FormClose();'),
]

# ---- //AI(W906-EVB10B) 20260929 [W906]：X-5（事件批次 B10 part b，D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\EVENT_PORT_BATCH_20260928.md B10 表；
#   R130＝照 BCB「等切分頁程式移植好一起解」；Steven 20260929「照 BCB 的邏輯」）—— 溫度補償那一排分頁的切頁事件
#   golden TfTemp_Set::pgcTempOffsetChange（uTemp_Set.cpp:5224-5262，uTemp_Set.dfm:4456 pgcTempOffset OnChange）：
#   切到 ATC 頁藏起「基準溫度」那一組（lblTempBase／btClearAll／edLowBase…edSHighBase）、其他頁顯示；Arm 1／Arm 2 頁才顯示排序鈕
#   btnSort（bNeedChange 時）；TSMC 台南另外切 pnlArm1Offset／pnlArm2Offset（R130：只有在自己的分頁上才看得見＝存得進去）。
#   vclcompat TPageControl 只有 ActivePageIndex ⇒ `->ActivePage`／`ts->TabIndex`／`ts->PageIndex` 走 filerw::ELActivePage／ELTabIndexOf／
#   ELPageIndexOf（FileRW/_EditList.h；頁序由 FileRW/Temperature.cpp 開機照 golden uTemp_Set.dfm:4473 起登記）。
#   golden 怪處照翻：:5226 拿 ActivePageIndex（頁序，藏起來的頁也算）跟 tsATC->TabIndex（看得見的位置）比 —— V912 只會藏 ATC 後面的頁，
#   ATC 前面四頁從不藏，所以兩個數字一樣（D:\HT9045\.claude\skills\ht9050-st01-evaluations\references\page-control-tabs.md §3.3）；
#   這裡照 VCL 算 TabIndex，前面的頁真的被藏時結果也跟 golden 一樣。
#   頁面點頁籤送 WS form.event {"control":"pgcTempOffset","event":"change","activePageIndex":n,"state":{…}}（D:\HT9045\web\page\ht9045_temp_set_ts1.js）。
_SPAN['pgcTempOffsetChange'] = _span('pgcTempOffsetChange')
_TS_PG = _SPAN['pgcTempOffsetChange'][0]
_expect(_TS_PG, 'void __fastcall TfTemp_Set::pgcTempOffsetChange(TObject *Sender)')
_TS_PC = 'EL<TPageControl>(%s, "pgcTempOffset")' % _N
_R('pgcTempOffsetChange', _expect(_TS_PG + 2, 'if(pgcTempOffset->ActivePageIndex==tsATC->TabIndex)'), _TS_PG + 2,
   'AI(W906-EVB10B) 20260929 tsATC->TabIndex（VCL TTabSheet.GetTabIndex：前面看得見的頁數，自己藏起來＝-1）→ filerw::ELTabIndexOf',
   'if(%s->ActivePageIndex==filerw::ELTabIndexOf(%s, "pgcTempOffset", "tsATC"))' % (_TS_PC, _N))
_a = _find('pgcTempOffsetChange', r'btnSort->Visible\s*=')
assert 'pgcTempOffset->ActivePage==tsArm1' in _G[_a] and 'pgcTempOffset->ActivePage==tsArm2' in _G[_a + 1] and 'bNeedChange==true' in _G[_a + 2], \
    'uTemp_Set.cpp pgcTempOffsetChange btnSort 那一段變了'
_R('pgcTempOffsetChange', _a, _a + 3,
   'AI(W906-EVB10B) 20260929 pgcTempOffset->ActivePage==tsArm1／tsArm2 → filerw::ELActivePage（其餘照 golden）',
   'EL<%s>(%s, "btnSort")->Visible=(CosFunction.bUseOldATCTempOffset==false && (filerw::ELActivePage(%s, "pgcTempOffset")==EL<TTabSheet>(%s, "tsArm1") || '
   'filerw::ELActivePage(%s, "pgcTempOffset")==EL<TTabSheet>(%s, "tsArm2")) && bNeedChange==true);' % (_W['btnSort'], _N, _N, _N, _N, _N))
for _k, _p in ((1, 'pnlArm1Offset'), (2, 'pnlArm2Offset')):
    _a = _find('pgcTempOffsetChange', r'%s->Visible\s*=' % _p)
    assert ('tsArm%d->PageIndex' % _k) in _G[_a - 1], 'uTemp_Set.cpp pgcTempOffsetChange %s 變了' % _p
    _R('pgcTempOffsetChange', _a, _a,
       'AI(W906-EVB10B) 20260929 tsArm%d->PageIndex → filerw::ELPageIndexOf（頁序位置）' % _k,
       'EL<%s>(%s, "%s")->Visible=(%s->ActivePageIndex==filerw::ELPageIndexOf(%s, "pgcTempOffset", "tsArm%d"));'
       % (_W[_p], _N, _p, _TS_PC, _N, _k))

STRUCT = {
    'struct': 'Temperature',
    'prefix': 'TS',
    'class': 'TfTemp_Set',
    'cpp': 'uTemp_Set.cpp',
    'h': 'uTemp_Set.h',
    'files': ['Temperature.Data', 'Tester.Data', r'DefineTemp\*.Data', r'Config\ATC.ini'],
    'lists': [],
    'methods': METHODS + ['sbtExitClick', 'pgcTempOffsetChange'],   # //AI(W906-EVB10A) 20260929 [W906]：TS-10 加在最後（見上）  //AI(W906-EVB10B) 20260929 [W906]：X-5 再加在最後
    'save_methods': ['spbSaveClick', 'SaveSetupFile', 'CheckTempSettingChange'],
    'params': {'FormShow': '', 'FormClose': '', 'spbSaveClick': '', 'sbtExitClick': '', 'pgcTempOffsetChange': '',   # //AI(W906-EVB10A) 20260929 [W906]：TS-10 golden 不讀 Sender  //AI(W906-EVB10B)：X-5 同
               'cbATCReferTempSensorClick': '', 'rbATCActiveOnClick': '', 'rbATC70ActiveOnClick': '',
               'rgIndexHeatModeClick': '', 'chkTempCalByRecipeClick': '', 'rgBasePointClick': ''},   # AI(W906-FRW-S158)：golden 不讀 Sender（rb1PointClick 保留 TObject *Sender：TurnSiblingsOff 要知道點的是哪一顆）
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
    'replace': _MANUAL_REPLACE + _auto_replace() + _TS_EXIT_REPLACE,   # //AI(W906-EVB10A) 20260929 [W906]：TS-10 sbtExitClick 四條
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fLotInfo.h', 'forms/fTemp_Set.h',
                 'forms/fATCHandlerSide.h', 'forms/fHS.h', 'forms/fDynamicTemp.h', 'atester_shims.h',
                 'ATC/ATCInterface.h', 'TempCtrl/TriTemp.h', 'database.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h',
                 'W906FormShowing.h'],   # AI(W906-FSHOW-B2) 20260929：W906_FormShowing（ReadTempFile :2495 的取代）
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
    # AI(W906-FRW-S158) 20260927 [W906]：WS form.event 事件表（Steven ★ Q40＝A，RULINGS_20260926 S157；格式 FROM_STEVEN 20260927 10:15；
    #   Q41 盤點 TS-1）。golden uTemp_Set.dfm:10839 rgIndexHeatMode OnClick、:10650 chkTempCalByRecipe OnClick。產生器在 .gen.inc 檔尾
    #   產生 kTS_Events，FileRW/Temperature.cpp 用 filerw::PageEventsRegistrar 註冊。
    'events': [('rgIndexHeatMode', 'click', 'rgIndexHeatModeClick'),
               ('chkTempCalByRecipe', 'click', 'chkTempCalByRecipeClick'),
               # AI(W906-FRW-S158) 20260927 [W906]：TS-7。頁面點基準點數送 {"control":"rb3Point","event":"click","checked":true,"state":{…}}，
               #   點 System／Kit 送 {"control":"rgBasePoint","event":"click","itemIndex":n,"state":{…}}；changed 回各欄的 visible／enabled。
               #   state 要帶頁面目前的值（RunPageEvent 先照「點之前」的看得見／可改套 state、再跑處理器 ⇒ 使用者先改欄位再換點數，
               #   那些欄位的值在點的當下就收下，存檔時看不見也照 golden 存）。
               #   btnSortClick（uTemp_Set.cpp:5842，dfm:10544）不列：btnSort 在 FormShow :952 藏起來，只有換分頁的 pgcTempOffsetChange（:5251，
               #   TS-5，頁面做）會讓它顯示；form.event 帶不了 TPageControl 的新分頁（FileRW/_EditPage.cpp RunPageEvent 第 4 步把控制項自己
               #   從 state 拿掉、第 5 步不套分頁）⇒ 伺服器端 btnSort 永遠點不到、事件一定回 bad-payload。它的效果只有排版（palTemp->Align，
               #   產生器已當純畫面）、按鈕字，以及 Tag==2（依 Site 排）時把 site map 沒用到的通道 palTemp 藏起來 —— 伺服器端 Tag 停在 0、
               #   不藏，存檔照收那些通道的頁面值（＝golden：看不見的欄照樣存）。排序由頁面自己做。
               ('rb1Point', 'click', 'rb1PointClick'), ('rb2Point', 'click', 'rb1PointClick'),
               ('rb3Point', 'click', 'rb1PointClick'), ('rb5Point', 'click', 'rb1PointClick'),
               ('rb6Point', 'click', 'rb1PointClick'),
               ('rgBasePoint', 'click', 'rgBasePointClick'),
               # //AI(W906-EVB3) 20260928 [W906]：TS-2（Steven 20260928「任何畫面的事件, 都是我們做」「如果已經有移植, 就接上」）。
               #   golden uTemp_Set.dfm 的 OnClick 綁定：rbATCActiveOn（dfm:6794，TCheckBox）→ rbATCActiveOnClick（:5430，ATC Active 勾了就把
               #   ATC 7.0 取消並停用）；rbATC70ActiveOn（dfm:7130）→ rbATC70ActiveOnClick（:5407，反過來）；cbATCReferTempSensor（dfm:6809）
               #   → cbATCReferTempSensorClick（:7073，取消時 cbUseTC2Offset 藏起來並取消）。golden 怪處照留：cbEnableATCConsFailOffset
               #   （dfm:7260）、cbEnableATCQAModeOffset（dfm:7395）、cbATCTestTimeOffset（dfm:7525）的 OnClick 也是 rbATC70ActiveOnClick
               #   （只看 rbATC70ActiveOn->Checked 重設同一批 Enabled，點了結果不變）。FileRW/Temperature.cpp 檔尾包一層：跑完 golden 處理器
               #   把 SaveFlow (2)(3) 比對用的 g_open 同步成伺服器現值（點的先後伺服器已經照 golden 跑過，存檔不再重播／不再拒存）。
               ('rbATCActiveOn', 'click', 'rbATCActiveOnClick'),
               ('rbATC70ActiveOn', 'click', 'rbATC70ActiveOnClick'),
               ('cbEnableATCConsFailOffset', 'click', 'rbATC70ActiveOnClick'),
               ('cbEnableATCQAModeOffset', 'click', 'rbATC70ActiveOnClick'),
               ('cbATCTestTimeOffset', 'click', 'rbATC70ActiveOnClick'),
               ('cbATCReferTempSensor', 'click', 'cbATCReferTempSensorClick'),
               # //AI(W906-EVB10A) 20260929 [W906]：TS-10 Exit 鈕（uTemp_Set.dfm sbtExit OnClick＝sbtExitClick）；替身在 FileRW/Temperature.cpp 開機時建
               ('sbtExit', 'click', 'sbtExitClick'),
               # //AI(W906-EVB10B) 20260929 [W906]：X-5 切分頁（uTemp_Set.dfm:4456 pgcTempOffset OnChange＝pgcTempOffsetChange）；帶 activePageIndex（X-2）
               ('pgcTempOffset', 'change', 'pgcTempOffsetChange')],
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'uTemp_Set.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 uTemp_Set.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('Temperature.py (E030-Q78): golden V912 uTemp_Set.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(4240, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(4244, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 uTemp_Set.cpp:4203-4208：A02 分支只有 Close()（:4207），後面沒有 return，存檔照跑；主選單 main.cpp:27362 用 ShowModal 開（對話框），Close() 只設 ModalResult ⇒ 操作員改的值照樣寫進檔（cContact.cpp:15031／17186 用 Show 開時，Close() 先跑 FormClose :4159-4169，那裡不重讀畫面，也照樣寫進檔）。'
                'V912 uTemp_Set.cpp:4244-4245：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Temperature 屬第 20a 條範圍（溫控照 V912），這一句同樣照 V912。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('spbSaveClick', _e030q78_expect(4245, 'return;'), 4245, _E030Q78_WHY, 'return;'))
