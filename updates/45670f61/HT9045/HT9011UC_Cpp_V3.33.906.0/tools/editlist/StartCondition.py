# -*- coding: utf-8 -*-
# tools/editlist/StartCondition.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only StartCondition
#
# Steven 團隊 20260925：golden TfStartCondition（cStartCondition.cpp，912，1890 行）—— Data.StartCondition.html。
# 結構名 StartCondition：這張表單沒有自己的主結構，它讀寫的是別人的欄位 ——
#   LastSet.bCTClear[2][7]／iStartMode／ContactSet[2]／iContactCT[2]／strSocketID[4][8]／iSocketContactCount[4][8]
#   IniConfig.ContactSet／HeadContactCount[3][2][16]、SocketContactSet／Count[4][8]、iVibrator*（記憶體；由 WriteLastDataFile 落 config.ini）
#   TestIF_File.iContactAlarmCount[4]／iContactWarningCount[2]／InOutArmLifeCntSet／*PickerLifeCnt
#   → 用表單名（去掉 Tf）當 tag，不冒用任何一個結構名（LastSet／IniConfig 都已有自己的 C 路 tag）。
#
# golden 按鈕（網頁一顆 golden 按鈕 → 伺服器跑那顆的 golden 處理器，見 FileRW/StartCondition.cpp 檔頭）：
#   sbHeadCondition1Save（:1091）Life Time 頁的 Save：A02 守衛 → ReadWriteStartCondition(false)（HandlerCondition.Data 或
#                                   IniData\SocketCount.ini）→ Socket ID／計數進 LastSet → DoIniDataToForm → sgSocketCount 進 IniConfig
#   sbSave（:708）              Contact count 的 Save：sbSaveClick → LastSet.ContactSet／iContactCT → WriteLastDataFile
#   spbExit（:751）             Exit：bCTClear → Close()（modal 關窗 → FormClose :619）→ WriteLastDataFile
# 清除類按鈕（sbSameAsHead1／sbClearCount／sbHeadCondition1Clear／btnClear*／btnInArm*…）：頁面記下點擊順序，存檔時
#   伺服器在套頁面值之前依序重放 golden 處理器，再跑存檔鈕（FileRW/StartCondition.cpp BeforeApply／SaveFlow）。
#
# 本檔的自動 replace（_auto）處理 golden 用了、vclcompat 沒有的四樣：
#   (1) pgLifeTime->ActivePage（TTabSheet* 屬性；vclcompat TPageControl 只有 ActivePageIndex）→ 本 TU 的 SC_pgLifeTimeActivePage
#   (2) TStringGrid 的 ColCount／RowCount／Width（FileRW/_EditList.h ELStringGrid 只轉出 Cells）→ ->grid.ColCount／RowCount；Width 是畫面
#   (3) TPanel 的 Caption 由頁面編輯（PanelAa..Dh：golden PanelAaDblClick 開小鍵盤改 Caption）／顯示（palVibrator*）
#       → SC_CapEdit（TEdit 衍生，Caption 是 Text 的別名），頁面值照 text 收送
#   (4) VCL 自動行為：TRadioButton 設 Checked=true 會把同一個 Parent 的其他 radio 清掉（SC_CheckRadio）；
#       TTabSheet 設 TabVisible=false 時若是 ActivePage，TPageControl::DeleteTab 會換到同位置的下一個可見頁（SC_VclTabShowing）
import os
import re

G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
F = 'TfStartCondition'
_cpp = open(os.path.join(G, 'cStartCondition.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_h = open(os.path.join(G, 'cStartCondition.h'), 'rb').read().decode('cp950', errors='replace')
_dfm = open(os.path.join(G, 'cStartCondition.dfm'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n')

_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_MAPPED = {'TStringGrid': 'filerw::ELStringGrid', 'TTrackBar': 'filerw::ELTrackBar', 'TUpDown': 'filerw::ELTrackBar',
           'TDateTimePicker': 'filerw::ELDateTimePicker'}
_W = {}
for _m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _h[re.search(r'class\s+TfStartCondition\b[^;{]*\{', _h).end():], re.M):
    _t = _m.group(1)
    _W[_m.group(2)] = _MAPPED.get(_t) or (_t if _t in _KNOWN else 'TControl')

# (3) TPanel → SC_CapEdit（名稱＝頁面元件 id；只在本檔的 replace 碼與 FileRW/StartCondition.cpp 裡出現，產生器的
#     EL<TPanel> 不會先建它 —— 這 36 個在 golden DFM 沒有 Enabled／Visible／ReadOnly，DfmState 不碰）
CAP = ['Panel' + r + c for r in 'ABCD' for c in 'abcdefgh'] + \
      ['palVibratorHP1', 'palVibratorSht1', 'palVibratorSht2', 'palVibratorUnloader']
for _n in CAP:
    assert _W.get(_n) == 'TPanel', _n
    _W[_n] = 'SC_CapEdit'
GRIDS = ['sgContactCount', 'sgHeadCondition1', 'sgHeadCondition2', 'sgHeadCondition3', 'sgSocketCount']
LIFE_TABS = ['tsCondition01', 'tsCondition02', 'tsCondition03', 'TabsSocketID', 'tsVibration', 'tsSmartDiagnostic',
             'tsSocketCount', 'tsPickerLifeTime', 'tsCylinderView']      # pgLifeTime 的頁，golden DFM 順序（:687-4113）

METHODS = ['TfStartCondition', 'FormShow', 'FormClose', 'sbSameAsHead1Click', 'sbClearCountClick', 'sbSaveClick',
           'spbExitClick', 'sbHeadCondition1ClearClick', 'ReadWriteStartCondition', 'sbHeadCondition1SaveClick',
           'DoIniDataToForm', 'pgLifeTimeChange', 'ClearPickerCount', 'btnInArmAClick', 'btnOutArmAClick',
           'btnArm2AaClick', 'btnArm1AaClick', 'WritePickerCount', 'btnClearAaClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfStartCondition::' + meth + r'\s*\(', l):
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


def _split(line):
    """[(是程式碼?, 文字)]：字串常值與 // 註解不改寫（同 gen_editlist.split_code）。"""
    out, i, n, buf = [], 0, len(line), ''
    while i < n:
        c = line[i]
        if line.startswith('//', i):
            if buf: out.append((True, buf)); buf = ''
            return out                      # 註解丟掉（replace 碼是單行，註解留在 #if 0 的原文裡）
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
_WRE = re.compile(r'(?<![\w.>:"])(' + '|'.join(map(re.escape, _NAMES)) + r')\b(?!\s*::)')
_MRE = re.compile(r'(?<![\w.>:])(' + '|'.join(METHODS) + r')\s*\(')


def _conv(code):
    """一行 golden 程式碼 → 產生器同樣的改寫（widget→EL<>、方法→SC_），再加本檔的 (1)(2)(3) 規則。
    只用在 replace 的取代碼（產生器不改寫 replace 的取代碼）。"""
    out = []
    for is_code, t in _split(code):
        if is_code:
            t = t.replace('pgLifeTime->ActivePage', 'SC_pgLifeTimeActivePage')                       # (1)
            t = re.sub(r'TStringGrid\s*\*\s*StrGrid\s*\[\s*\]', 'filerw::ELStringGrid *StrGrid[]', t)   # (2)
            t = re.sub(r'\bStrGrid\[(\w+)\]->(RowCount|ColCount)', r'StrGrid[\1]->grid.\2', t)
            t = _MRE.sub(lambda m: 'SC_' + m.group(1) + '(', t)
            t = _WRE.sub(lambda m: 'EL<%s>("%s", "%s")' % (_W[m.group(1)], F, m.group(1)), t)
            t = re.sub(r'(EL<filerw::ELStringGrid>\("%s", "\w+"\))->(RowCount|ColCount)' % F, r'\1->grid.\2', t)
            t = re.sub(r'EL<filerw::ELStringGrid>\("%s", "\w+"\)->Width\s*=[^;]*;' % F, ';', t)       # (2) Width：畫面
        out.append(t)
    return ''.join(out).strip()


_ct_cnt_end = L('TfStartCondition', '{PanelDa, PanelDb') + 1
assert _cpp[_ct_cnt_end - 1].strip() == '};', _cpp[_ct_cnt_end - 1]

MANUAL = [
    # ---- 建構子
    ('TfStartCondition', L('TfStartCondition', 'TPanel *TempSocketContactCnt'), _ct_cnt_end,
     'TPanel *TempSocketContactCnt[4][8]={{PanelAa,…}}：golden TPanel 的 Caption 由操作員改（PanelAaDblClick 小鍵盤），'
     '替身改 SC_CapEdit（TEdit 衍生、Caption＝Text 別名），頁面值照 text 收送。名稱照 golden（PanelAa..PanelDh）',
     'SC_CapEdit *TempSocketContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL]; '
     'for(int r=0; r<MAX_SOCKET_ROW; r++) for(int c=0; c<MAX_SOCKET_COL; c++) '
     'TempSocketContactCnt[r][c]=EL<SC_CapEdit>("%s", (AnsiString("Panel")+AnsiString((char)(\'A\'+r))+AnsiString((char)(\'a\'+c))).c_str());' % F),
    ('TfStartCondition', L('TfStartCondition', 'PtrDCCyl=GetDC('), L('TfStartCondition', 'pCanvasCyl=new TCanvas;'),
     'GetDC(strngrdCylinderView->Handle)／new TCanvas：Cylinder 頁的 GDI 繪圖（strngrdCylinderViewDrawCell），HTML 自己畫；Cylinder 頁未移植（見 FileRW/StartCondition.cpp）',
     ';'),
    # ---- FormShow
    ('FormShow', L('FormShow', 'fSetup->ReadFile();'), L('FormShow', 'fSetup->ReadFile();'),
     'fSetup->ReadFile()：移植樹 TfSetup::ReadFile（cSetUp.cpp:416）。golden ReadFile 在 cSetUp.cpp:2954 呼叫 '
     'fStartCondition->ReadWriteStartCondition(true)，移植樹那一行是 #if 0 GATE(G-SU-StartCond)（cSetUp.cpp:1190，沒有 fStartCondition 實例）'
     '→ 在這裡緊接著呼叫本表單的那一支（golden ReadFile 在 :2954 之後沒有任何敘述用到它讀的值，順序等價）',
     'fSetup->ReadFile(); SC_ReadWriteStartCondition(true);'),
    ('FormShow', L('FormShow', 'Left=75;'), L('FormShow', 'Top=10;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
    # ---- FormClose
    ('FormClose', L('FormClose', 'sbSaveClick(this);'), L('FormClose', 'sbSaveClick(this);'),
     'sbSaveClick(this)：事件處理器的 Sender 參數已拿掉', 'SC_sbSaveClick();'),
    # ---- spbExitClick
    ('spbExitClick', L('spbExitClick', 'Close();'), L('spbExitClick', 'Close();'),
     'Close()：golden 關表單（ShowModal 的表單 → ModalResult，spbExitClick 回傳後 modal 迴圈才跑 FormClose）'
     '→ 記 closed；FormClose 由 FileRW/StartCondition.cpp 在本處理器之後呼叫（同 VCL 順序）',
     'filerw::ELMark("closed");'),
    # ---- sbHeadCondition1SaveClick
    ('sbHeadCondition1SaveClick', L('sbHeadCondition1SaveClick', 'Close();'), L('sbHeadCondition1SaveClick', 'Close();'),
     'Close()：A02 權限不足時關表單 → 記 closed（FileRW/StartCondition.cpp 照 VCL 接著跑 FormClose）',
     'filerw::ELMark("closed");'),
    # ---- sbHeadCondition1ClearClick
    ('sbHeadCondition1ClearClick', L('sbHeadCondition1ClearClick', 'ClearPickerCount();'), L('sbHeadCondition1ClearClick', 'ClearPickerCount();'),
     'ClearPickerCount()：golden 預設參數 (iArea=-1, iTag=-1) 在 header；產生的 static 函式沒有預設值 → 明寫',
     'SC_ClearPickerCount(-1, -1);'),
]

_MANUAL_LINES = set()
for _b in MANUAL:
    _MANUAL_LINES.update(range(_b[1], _b[2] + 1))

_PAT = re.compile(r'pgLifeTime->ActivePage|TStringGrid\s*\*\s*StrGrid|StrGrid\[\w+\]->(RowCount|ColCount)|'
                  r'\b(' + '|'.join(GRIDS) + r')->(RowCount|ColCount|Width)\b|\bpalVibrator\w+|'
                  r'\b(' + '|'.join(LIFE_TABS) + r')->TabVisible\s*=|\brbStartMode[123]->Checked\s*=\s*true')


def _auto():
    out = []
    for m, (a, b) in SPANS.items():
        for gl in range(a + 1, b):
            if gl in _MANUAL_LINES:
                continue
            raw = _cpp[gl - 1]
            code = ''.join(t for c, t in _split(raw) if c)       # 只看程式碼部分
            if not _PAT.search(code):
                continue
            new = _conv(raw.rstrip())
            why = []
            tv = re.match(r'^(.*?)\bEL<TTabSheet>\("%s", "(\w+)"\)->TabVisible\s*=(.*);$' % F, new)
            rb = re.match(r'^EL<TRadioButton>\("%s", "(rbStartMode[123])"\)->Checked\s*=\s*true\s*;$' % F, new)
            if tv and tv.group(2) in LIFE_TABS:
                # (4) VCL TTabSheet.SetTabVisible → TPageControl.DeleteTab：大括號包起來（golden 有無大括號的 if 本體，例 :363-364）
                assert tv.group(1).strip() == '', (gl, new)
                t = 'EL<TTabSheet>("%s", "%s")' % (F, tv.group(2))
                new = '{ %s->TabVisible=%s; SC_VclTabShowing(%s); }' % (t, tv.group(3).strip(), t)
                why.append('TabVisible 換頁（VCL DeleteTab：ActivePage 被藏起來 → 換到同位置的下一個可見頁）')
            elif rb:
                new = 'SC_CheckRadio(EL<TRadioButton>("%s", "%s"));' % (F, rb.group(1))
                why.append('TRadioButton Checked=true 清掉同 Parent（gbStartMode）的其他 radio（VCL TRadioButton.SetChecked）')
            if 'SC_pgLifeTimeActivePage' in new:
                why.append('pgLifeTime->ActivePage → SC_pgLifeTimeActivePage（vclcompat TPageControl 沒有 ActivePage）')
            if '->grid.' in new or 'ELStringGrid *StrGrid' in new or re.search(r'(' + '|'.join(GRIDS) + r')->Width', code):
                why.append('TStringGrid ColCount／RowCount → ELStringGrid::grid；Width 是畫面')
            if 'SC_CapEdit' in new:
                why.append('palVibrator*（TPanel）→ SC_CapEdit：Caption 送頁面顯示')
            out.append((m, gl, gl, '自動：' + '；'.join(why), new))
    return out


def _dfm_tags_and_grids():
    """golden DFM 的 Tag（按鈕／grid）與 grid ColCount／RowCount → SC_DfmStatic()（產生器不帶這兩樣）。"""
    types, props, stack = {}, {}, []
    for ln in _dfm.split('\n'):
        t = ln.strip()
        m = re.match(r'(?:object|inherited|inline)\s+(\w+)\s*:\s*(\w+)', t)
        if m:
            stack.append(m.group(1)); types[m.group(1)] = m.group(2); continue
        if t == 'end' and stack:
            stack.pop(); continue
        pm = re.match(r'(Tag|ColCount|RowCount)\s*=\s*(-?\d+)$', t)
        if pm and stack:
            props.setdefault(stack[-1], {})[pm.group(1)] = pm.group(2)
    body = []
    for n in sorted(props):
        if n not in _W or _W[n] == 'TControl':
            continue                                    # TMenuItem（Cylinder 頁，未移植）
        p = props[n]
        e = 'filerw::EL<%s>("%s", "%s")' % (_W[n], F, n)
        if 'Tag' in p:
            body.append('%s->Tag=%s;' % (e, p['Tag']))
        if types[n] == 'TStringGrid':
            # VCL TStringGrid 設計期預設 5x5；DFM 有寫才覆蓋（cStartCondition.dfm）
            body.append('%s->grid.ColCount=%s; %s->grid.RowCount=%s;' % (e, p.get('ColCount', '5'), e, p.get('RowCount', '5')))
    for n in GRIDS:
        assert any('"%s"' % n in x and 'grid.ColCount' in x for x in body), n
    act = re.search(r'object pgLifeTime: TPageControl.*?ActivePage = (\w+)', _dfm, re.S).group(1)
    body.append('SC_pgLifeTimeActivePage=filerw::EL<TTabSheet>("%s", "%s");   /* golden DFM pgLifeTime.ActivePage */' % (F, act))
    return ('void SC_DfmStatic()   // golden cStartCondition.dfm 設計期 Tag（btnClear*／btnInArm*…／sgHeadCondition2..）、'
            'grid ColCount／RowCount、pgLifeTime.ActivePage —— 產生器的 DfmState 不帶這三樣；FileRW_StartCondition_Boot 在建構子之前呼叫\n{\n    '
            + '\n    '.join(body) + '\n}')


def _build_head_row_map():
    """golden cStartCondition.cpp:39-89（檔案層級 static，不是 TfStartCondition 的方法 → 產生器轉不到）照抄；
    FTestSuck 由本檔的巨集轉到 FileRW/_KitSuck.cpp。"""
    a = next(i for i, l in enumerate(_cpp) if l.startswith('static int  g_iHeadRowMap[2][16];'))
    b = next(i for i, l in enumerate(_cpp) if l.startswith('static void BuildHeadRowMap(int iGridRowCount)'))
    e = next(i for i in range(b, len(_cpp)) if _cpp[i] == '}')
    assert _cpp[a + 1].startswith('static bool g_bHeadRowMapReady=false;')
    src = _cpp[b:e + 1]
    src[0] = src[0].replace('static void ', 'void ', 1)
    return (['int  g_iHeadRowMap[2][16];      // golden cStartCondition.cpp:%d（檔案層級 static）' % (a + 1),
             'bool g_bHeadRowMapReady=false;  // golden cStartCondition.cpp:%d' % (a + 2),
             '\n'.join(src) + '   // golden cStartCondition.cpp:%d-%d（照抄）' % (b + 1, e + 1)])


STRUCT = {
    'struct': 'StartCondition',
    'prefix': 'SC',
    'class': F,
    'cpp': 'cStartCondition.cpp',
    'h': 'cStartCondition.h',
    'files': ['system\\lastdata.dat＋lastdata_backup.dat（WriteLastDataFile）',
              'config.ini [O_Count]／[Vibrate_Time]／[SocketContact]（WriteLastDataFile 內）',
              '<recipe>\\HandlerCondition.Data [HeadCondition]／[O_Count]（或 IniData\\SocketCount.ini，D05_1）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['sbSaveClick', 'spbExitClick', 'FormClose', 'sbHeadCondition1SaveClick', 'ReadWriteStartCondition'],
    'params': {'TfStartCondition': '', 'FormShow': '', 'FormClose': '', 'sbSameAsHead1Click': '', 'sbClearCountClick': '',
               'sbSaveClick': '', 'spbExitClick': '', 'pgLifeTimeChange': ''},
    'members': [
        'bool fShow=false;   // golden cStartCondition.h:431 —— 本 TU 自己的（移植樹沒有 fStartCondition 實例；'
        'golden Command.cpp:7358 讀 fStartCondition->fShow 決定 Bit4_HandlerDiagnostics，移植樹那段不讀這裡）',
        'TTabSheet *SC_pgLifeTimeActivePage=nullptr;   // golden pgLifeTime->ActivePage（VCL 屬性，跨開頁保留；'
        '開機＝DFM tsSocketCount，存檔時＝頁面目前的分頁，見 FileRW/StartCondition.cpp）',
        # golden cStartCondition.cpp:30-36 檔案層級全域（本表單私用）
        'TEdit       *SocketID[MAX_SOCKET_ROW][MAX_SOCKET_COL];           // golden cStartCondition.cpp:30',
        'SC_CapEdit  *SocketContactCnt[MAX_SOCKET_ROW][MAX_SOCKET_COL];   // golden cStartCondition.cpp:31（TPanel* → SC_CapEdit，見 decls）',
        'TLabel      *TestLabelCol[MAX_SOCKET_COL];                       // golden cStartCondition.cpp:32',
        'TLabel      *TestLabelRow[MAX_SOCKET_ROW];                       // golden cStartCondition.cpp:33',
        'TLabel      *TestIDLabelRow[MAX_SOCKET_ROW];                     // golden cStartCondition.cpp:34',
        'TLabel      *TestCountLabelRow[MAX_SOCKET_ROW];                  // golden cStartCondition.cpp:35',
        'TButton     *ClearCntButton[MAX_SOCKET_ROW][MAX_SOCKET_COL];     // golden cStartCondition.cpp:36',
        '#define TestSocket SC_KitOf(0)   // golden TestSocket.iShtRow／iShtCol／iMaxRow／iMaxCol → FileRW/_KitSuck.cpp',
        '#define FTestSuck  SC_KitOf(1)   // golden FTestSuck.iShtRow／iShtCol（BuildHeadRowMap）→ FileRW/_KitSuck.cpp',
    ] + _build_head_row_map() + [
        # (4) VCL 自動行為
        'void SC_CheckRadio(TRadioButton *rb)   // VCL TRadioButton.SetChecked(true)：同 Parent（gbStartMode）的其他 radio 變 false\n'
        '{\n'
        '    TRadioButton *g[3]={filerw::EL<TRadioButton>("%s", "rbStartMode1"), filerw::EL<TRadioButton>("%s", "rbStartMode2"),\n'
        '                        filerw::EL<TRadioButton>("%s", "rbStartMode3")};\n'
        '    for(int i=0; i<3; i++) if(g[i]!=rb) g[i]->Checked=false;\n'
        '    rb->Checked=true;\n'
        '}' % (F, F, F),
        'void SC_VclTabShowing(TTabSheet *t)   // VCL TTabSheet.SetTabShowing(false)→TPageControl.DeleteTab：藏起來的是 ActivePage →\n'
        '{                                      // 換到「同一個索引」＝下一個可見頁（沒有就最後一個可見頁）。pgLifeTime 的頁照 golden DFM 順序\n'
        '    static const char* const kTabs[]={%s};\n'
        '    if(t!=SC_pgLifeTimeActivePage || t->TabVisible) return;\n'
        '    int n=(int)(sizeof(kTabs)/sizeof(kTabs[0])), at=-1;\n'
        '    for(int i=0; i<n; i++) if(filerw::EL<TTabSheet>("%s", kTabs[i])==t) at=i;\n'
        '    for(int i=at+1; i<n; i++) { TTabSheet *p=filerw::EL<TTabSheet>("%s", kTabs[i]); if(p->TabVisible) { SC_pgLifeTimeActivePage=p; return; } }\n'
        '    for(int i=at-1; i>=0; i--) { TTabSheet *p=filerw::EL<TTabSheet>("%s", kTabs[i]); if(p->TabVisible) { SC_pgLifeTimeActivePage=p; return; } }\n'
        '    SC_pgLifeTimeActivePage=nullptr;\n'
        '}' % (', '.join('"%s"' % x for x in LIFE_TABS), F, F, F),
        _dfm_tags_and_grids(),
    ],
    'replace': MANUAL + _auto(),
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h', 'forms/fSetup.h', 'forms/fLotInfo.h',
                 'csystem.h', 'cMyDB.h'],   # cMyDB.h：RecordChangeLogProcess／RecordProcess（與 canary_support.h 預設參數重複宣告，不能兩個都 include）
    'decls': [
        'extern bool authCounterClr[9];          // cAuthority.h:61（cAuthority.h 帶進 language.h，與 HTEditList.h 衝突）',
        'void GetCountClrAuth();                 // cAuthority.h:76',
        'void GetLimitAuth();                    // cAuthority.h:77',
        'void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket，1＝FTestSuck）',
        '// golden TMyKitSuck 用到的四個欄位的轉接物件：TestSocket／FTestSuck 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
        'struct SC_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };',
        'static inline SC_Kit SC_KitOf(int w) { SC_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }',
        '// golden TPanel 的 Caption 由頁面收送（PanelAa..Dh 操作員可改；palVibrator* 顯示）：TEdit 衍生 → filerw 照 text 收送，Caption 是 Text 的別名',
        'class SC_CapEdit : public TEdit { public: AnsiString& Caption; SC_CapEdit() : Caption(Text) {} };',
    ],
    'overrides': [],
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cStartCondition.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 cStartCondition.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('StartCondition.py (E030-Q78): golden V912 cStartCondition.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(1094, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(1098, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 cStartCondition.cpp:929-934：A02 分支只有 Close()（:933），後面沒有 return，存檔照跑；主選單 main.cpp:27563 用 ShowModal 開（對話框），Close() 只設 ModalResult ⇒ 操作員改的值照樣寫進檔。'
                'V912 cStartCondition.cpp:1098-1099：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('sbHeadCondition1SaveClick', _e030q78_expect(1099, 'return;'), 1099, _E030Q78_WHY, 'return;'))
