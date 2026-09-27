# -*- coding: utf-8 -*-
# tools/editlist/ContactForce.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only ContactForce
#
# //AI(W906-FRW-S57) 20260926: 新檔（Steven 團隊，S57 Setup.ContactForce 讀寫）。golden V912 TfContactForce
#   （D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\ContactForce.cpp，1602 行，cp950）——Setup.ContactForce.html 的「讀寫段」。
#   一律照 V912（RULINGS_20260926 第 26 條：畫面照 912）；客戶專屬（CC_KYEC_LEE／CC_ASE_SG／CC_ASE_KaohSiung／CC_HONPREC_QC）編得過就照留（S25）。
#   延續上一位工程師的計畫（scratchpad contactforce/plan.md），與現況不符處以現況為準（見 FileRW/ContactForce.cpp 檔頭）。
#
# 結構名 ContactForce：這張表單沒有 cprod.h 的主結構，它讀寫 D:\HT9045\system\ContactInfo.ini（系統檔，不跟配方）、
#   Gerneral.ini [System] EP 8 鍵與 [Test Arm] dIndexZOffset 30 鍵、以及 btSaveClick 的 SaveLastSetIni（config.ini 等）；
#   同 ShuttleMove／GroundMan 的慣例用表單名（去掉 Tf）當 tag。
#
# 轉的 golden 方法：
#   建構子 TfContactForce（:421）＝ golden CreateForm（HT9045.cpp:220）；移植樹在第一次開頁時才跑（lazy，見 FileRW/ContactForce.cpp）
#   FormShow（:686）              ＝ 開頁（editlist.get）：ReadFile＋頁籤可見／權限＋NS 切換＋tb*／tbD25／edD25 從記憶體填
#   btSaveClick（:936）           ＝ 存檔鈕（editlist.save）：iContactForceMap／LastSet.dIndexLoadRate ← 畫面 → SaveLastSetIni → WriteFile → ReadFile
#   ReadFile（:966）／WriteFile（:1183）＝ 讀寫器（ContactInfo.ini 全部段＋Gerneral.ini EP 12 讀／8 寫＋[Test Arm] 30 鍵）
#   tb30mm_10kgChange（:1378）    ＝ DFM 8 個 tb*mm_*kg 的 OnChange（EP 直接輸出＋ShowValue）；ShowValue（:1385）
# 四個動態面板類別（THTSLKClass 等）產生器不轉 → 手寫 FileRW/ContactForce_Panels.h（includes 帶進來）。
#
# 不轉：FormDestroy（:655，程式不結束）、FormClose（:917，fContact->ShowArmAndDeviceForce＋DeviceForm.dPress＋ADAM_WriteVoltage；
#   網頁沒有關頁事件）、btExitClick（:930，ADAM_WriteVoltage＋Close）、小鍵盤（edt*Click／ed*MouseDown → 頁面 kb 表）、
#   Button1Click（:1591，電壓換算不存檔 → 頁面 JS 同一條算式，或 ContactForce.h ComputeEpMaxVoltage）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'ContactForce.cpp'), 'rb').read().decode('cp950', errors='replace').split('\n')   # 與 gen_editlist.py 同一種解碼（行號一致）
_F = 'TfContactForce'

METHODS = ['TfContactForce', 'FormShow', 'btSaveClick', 'ReadFile', 'WriteFile', 'tb30mm_10kgChange', 'ShowValue']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfContactForce::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('ContactForce.py: span %s' % meth)


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
    raise SystemExit('ContactForce.py: L(%s, %r) not found' % (meth, text))


def _code(gl):
    """golden 第 gl 行的程式碼部分（去掉行尾 // 註解；字串常值裡的 // 不算）。"""
    line, out, i, q = _cpp[gl - 1].rstrip('\r'), '', 0, None
    while i < len(line):
        c = line[i]
        if q:
            out += c
            if c == '\\' and i + 1 < len(line):
                out += line[i + 1]; i += 2; continue
            if c == q:
                q = None
        elif line.startswith('//', i):
            break
        else:
            if c in '"\'':
                q = c
            out += c
        i += 1
    return out.strip()


# ---- golden header 的 TStringList* 成員：產生器依 header 形狀把它們當元件改寫成 EL<TControl>(…)（沒有 CommaText／Strings／Count）
#      → 用到它們的每一行機械改寫成 static 成員本身（同 HSys.py slCustomerCode、DeviceForm_File.py sOutDiameter 的做法）。
#      同一行裡的 golden 元件只有這 6 個 TEdit（golden ContactForce.h:194-195/:210-211/:221-222），改寫成具名替身；
#      出現其他元件名就中止（表示 golden 變了，要人看）。
SL = ['slSLKTypeInd', 'slSLKTypeIndVisible', 'slSLKType', 'slSLKTypeVisible', 'slDieForceSLKType', 'slDieForceSLKTypeVisible',
      'slDieForceOneByOneSLKType', 'slDieForceOneByOneSLKTypeVisible']
_SLRE = re.compile(r'(?<![\w.>:])(' + '|'.join(sorted(SL, key=len, reverse=True)) + r')\b')
_EDT = ['edtCurrentType', 'edtVisible', 'edtCurrentTypeInd', 'edtVisibleInd', 'edtDieForceCurrentType', 'edtDieForceVisible']
_EDTRE = re.compile(r'(?<![\w.>:])(' + '|'.join(sorted(_EDT, key=len, reverse=True)) + r')\b')
_h = open(os.path.join(_G, 'ContactForce.h'), 'rb').read().decode('cp950', errors='replace')
_hb = _h[re.search(r'\bclass\s+TfContactForce\b[^;{]*\{', _h).end():]
_WIDGETS = set(m.group(2) for m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _hb, re.M)) - set(SL) - set(_EDT)
_WRE = re.compile(r'(?<![\w.>:])(' + '|'.join(sorted(_WIDGETS, key=len, reverse=True)) + r')\b(?!\s*::)')


def _sl_code(gl):
    c = _code(gl)
    bad = _WRE.findall(re.sub(r'"[^"]*"', '""', c))
    if bad:
        raise SystemExit('ContactForce.py: golden :%d 同一行有其他元件 %r，機械改寫不處理' % (gl, bad))
    return _EDTRE.sub(lambda m: 'EL<TEdit>("%s", "%s")' % (_F, m.group(1)), c)


# 越界守衛（golden 會當：vector 下標超出 → BCB AV／標準 C++ 未定義行為；伺服器不能當）。只在「會越界」那一圈起作用，
#   其餘與 golden 同一個迴圈：[SLK Type] 某個 token 被 :466／:508／:521 濾掉（空字串或 ≤15）時，vector 比 Count×8／×16 短，
#   golden 從那一型之後的下標全部錯位並在最後一型越界。sl 為 NULL（bUseDynamicKitDiameter 在建構之後才變 true）也擋。
_GUARD = {
    ('ReadFile', 'for(int i=0; i<slSLKTypeInd->Count; i++)', 1):
        'for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*8+7)<SLKIndClass.size(); i++)',
    ('ReadFile', 'for(int i=0; i<slSLKTypeInd->Count; i++)', 2):
        'for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*16+15)<SLKIndClass.size(); i++)',
    ('WriteFile', 'for(int i=0; i<slSLKTypeInd->Count; i++)', 1):
        'for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*8+7)<SLKIndClass.size(); i++)',
    ('WriteFile', 'for(int ii=0; ii<slDieForceOneByOneSLKType->Count; ii++)', 1):
        'for(int ii=0; slDieForceOneByOneSLKType!=NULL && ii<slDieForceOneByOneSLKType->Count && (unsigned int)(ii*8+7)<DieForceOneByOneSLKClass.size(); ii++)',
    ('WriteFile', 'for(int i=0; i<slSLKTypeInd->Count; i++)', 2):
        'for(int i=0; slSLKTypeInd!=NULL && i<slSLKTypeInd->Count && (unsigned int)(i*16+15)<SLKIndClass.size(); i++)',
}
_GUARD_LINES = {L(m, t, n): code for (m, t, n), code in _GUARD.items()}

REPLACE = []
for _m in ('TfContactForce', 'ReadFile', 'WriteFile'):
    _a, _b = SPANS[_m]
    for _gl in range(_a, _b + 1):
        if not _SLRE.search(_code(_gl)):
            continue
        if _gl in _GUARD_LINES:
            REPLACE.append((_m, _gl, _gl, 'TStringList 成員→static＋越界守衛（tools/editlist/ContactForce.py 的 SL／_GUARD）', _GUARD_LINES[_gl]))
        else:
            REPLACE.append((_m, _gl, _gl, 'TStringList 成員→static（tools/editlist/ContactForce.py 的 SL）', _sl_code(_gl)))

_OBO_J = L('ReadFile', 'for(int j=0; j<8; j++)', 1)          # ReadFile :1094 DieForceOneByOneSLKClass[i*8+j]
_WF_GUARD = L('WriteFile', 'if(bHasFile==false)', 1)          # WriteFile :1193 SLKClass[0..3]（EP_Install==5 :1212-1233 [0..7]）
_WF_ZOFS = L('WriteFile', 'WriteIniDataGeneral("Test Arm", Str, IndexZOffsetEdit[i][j]->Text);')
_KYEC_A = L('ReadFile', 'fContact->rgKitDiameter->Items->Clear();')
_KYEC_B = L('ReadFile', 'fContact->rgKitDiameter->Items->Add(fContactForce->SLKClass[i]->sDiameter);') + 2   # :1141 for 的右大括號
_SAVE_LS = L('btSaveClick', 'SaveLastSetIni();')

REPLACE += [
    # ---- FormShow（golden :686）
    ('FormShow', L('FormShow', 'Left=10;'), L('FormShow', 'Top=10;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
    # ---- ReadFile（golden :966）
    ('ReadFile', _OBO_J, _OBO_J, '越界守衛：DieForceOneByOneSLKClass[i*8+j]（i 走 DieForceSLKClass，與 OneByOne 的 token 濾法相同時兩者對齊；'
     '不對齊時 golden 越界）', 'for(int j=0; j<8 && (unsigned int)(i*8+j)<DieForceOneByOneSLKClass.size(); j++)'),
    ('ReadFile', _KYEC_A, _KYEC_B,
     'CC_KYEC_LEE 缸徑 30→28／60→58 自動轉換後重建 fContact->rgKitDiameter：fContact 是 golden TfContact —— C 路 TfContact 的替身'
     '（FileRW/DeviceForm_File.cpp，同名 EL<TRadioGroup>("TfContact","rgKitDiameter")）；fContactForce->SLKClass＝本 TU 的 SLKClass。程式照 golden',
     '{ TRadioGroup* rgKit=EL<TRadioGroup>("TfContact", "rgKitDiameter"); rgKit->Items->Clear(); '
     'for(unsigned int i=0; i<SLKClass.size(); i++) { if(SLKClass[i]->bShow) { rgKit->Items->Add(SLKClass[i]->sDiameter); } } }'),
    # ---- WriteFile（golden :1183）
    ('WriteFile', _WF_GUARD, _WF_GUARD,
     '越界守衛：golden :1195-1258 直接用 SLKClass[0..3]（EP_Install==5 且有檔時 [0..7]）；[SLK Type] 有效口徑不足 4 個（EP_Install==5 是 8 筆）'
     '時 golden 會當。這裡整支 WriteFile 不做（不寫任何檔）並記 todo；頁面存檔在 FileRW/ContactForce.cpp 先擋（連 SaveLastSetIni 都不跑）',
     'if(SLKClass.size()<((bHasFile==false)?4u:((EP_Install==5)?8u:4u))) { filerw::ELTodo("golden ContactForce.cpp:1195-1258 needs SLKClass[0..3] '
     '(EP_Install==5: [0..7]) but [SLK Type] has fewer valid diameters -- golden AVs here; WriteFile skipped, nothing written"); return; } '
     'if(bHasFile==false)'),
    ('WriteFile', _WF_ZOFS, _WF_ZOFS,
     'IndexZOffsetEdit 是 NULL（golden 只在 ReadFile :972-980 指派；「沒有 ContactInfo.ini、建構子直接 WriteFile」那一次 golden 會 AV）'
     '→ 那 30 鍵不寫（Gerneral.ini 原值不動）並記 todo；指派過就照 golden 寫',
     'if(IndexZOffsetEdit[i][j]!=NULL) WriteIniDataGeneral("Test Arm", Str, IndexZOffsetEdit[i][j]->Text); '
     'else if(i==0 && j==0) filerw::ELTodo("golden ContactForce.cpp:1372 IndexZOffsetEdit is NULL (WriteFile before any ReadFile: ctor no-file path, '
     'golden AVs here) -- Gerneral.ini [Test Arm] dIndexZOffset[0..1][0..14] not written");'),
    # ---- btSaveClick（golden :936）
    ('btSaveClick', _SAVE_LS, _SAVE_LS,
     '存檔標記：golden 第一個寫檔動作（SaveLastSetIni 寫 Gerneral.ini [Version]／config.ini／LastSet.ini／configByRecipe.ini），'
     'btSaveClick 沒有任何 return（savedMark="CF_SaveLastSetIni"）',
     'filerw::ELMark("CF_SaveLastSetIni"); SaveLastSetIni();'),
    # ---- tb30mm_10kgChange（golden :1378，DFM 8 個 tb*mm_*kg 的 OnChange）
    ('tb30mm_10kgChange', L('tb30mm_10kgChange', 'TTrackBar *Ptr=(TTrackBar *)Sender;'), L('tb30mm_10kgChange', 'TTrackBar *Ptr=(TTrackBar *)Sender;'),
     'Sender：事件處理器參數已拿掉（ELTrackBar::OnChange 無參數）；Ptr 只給下一行的 EP 輸出用', ';'),
    ('tb30mm_10kgChange', L('tb30mm_10kgChange', 'ADAM_DirectWriteData('), L('tb30mm_10kgChange', 'ADAM_DirectWriteData('),
     'ADAM_DirectWriteData(Ptr->Position,0)：EP 類比輸出（硬體，會讓機台動作 → 歸 Jimmy；移植樹 ADAM_* 是空殼）。'
     'golden 在 FormShow :891-898 設 tb*->Position 時也會經 OnChange 觸發它（開頁就把 8 個值輸出到 EP 一次）',
     'filerw::ELTodo("golden ContactForce.cpp:1381 ADAM_DirectWriteData(tb*->Position,0) not done (EP analog output = machine action, Jimmy; ADAM_* are empty in the port)");'),
]

BLOCKS = [
    ('btSaveClick', L('btSaveClick', 'ADAM_WriteVoltage(DeviceForm.dPress);'), L('btSaveClick', 'ADAM_WriteVoltage(DeviceForm.dPress);'),
     'ADAM_WriteVoltage(DeviceForm.dPress): EP voltage output after save (machine action -> Jimmy; ADAM_* are empty in the port)'),
]

STRUCT = {
    'struct': 'ContactForce',
    'prefix': 'CF',
    'class': _F,
    'cpp': 'ContactForce.cpp',
    'h': 'ContactForce.h',
    'files': ['D:\\HT9045\\system\\ContactInfo.ini（[SLK Type]、Diameter_*、DieForceDiameter_*；讀＋寫）',
              'Gerneral.ini [System] EP_MAXKPA／EP_MAXA／EP_MINMPA／EP_MINA_FeedBack＋_1032 四鍵（寫）、EPDual_* 四鍵（只讀）、[Test Arm] dIndexZOffset[0..1][0..14]（讀＋寫）',
              'SaveLastSetIni：Gerneral.ini [Version]、config.ini（[Contact Force] 30MM_10KG…）、LastSet.ini、configByRecipe.ini'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['btSaveClick', 'WriteFile'],
    'params': {'TfContactForce': '', 'FormShow': '', 'btSaveClick': '', 'tb30mm_10kgChange': ''},
    'globals': ['bNeedWriteFile'],   # golden ContactForce.cpp:18（KYEC 30→28／60→58 轉換的旗標，只有本檔用）
    'members': [
        'bool fShow=false;                     // golden ContactForce.h:334 —— 本 TU 自己的（網頁沒有關頁事件；golden adam6024.cpp:1822 ADAM_WriteVoltage 看它，移植樹 ADAM_* 是空殼、沒有讀者）',
        'AnsiString FileName;                  // golden ContactForce.h:335（建構子 :428 = "D:\\\\HT9045\\\\system\\\\ContactInfo.ini"）',
        'bool bHasFile=false;                  // golden ContactForce.h:336',
        'std::vector<THTSLKClass *> SLKClass;                                  // golden ContactForce.h:341（面板類別：FileRW/ContactForce_Panels.h）',
        'std::vector<THTSLKIndClass *> SLKIndClass;                            // golden ContactForce.h:342',
        'TStringList *slSLKTypeInd=NULL;       // golden ContactForce.h:343（建構子 :494 new；ReadFile／WriteFile 用 Count）',
        'TStringList *slSLKTypeIndVisible=NULL;   // golden ContactForce.h:344',
        'TStringList *slSLKType=NULL;          // golden ContactForce.h:345（建構子 :456 new、:646-648 delete＋NULL）',
        'TStringList *slSLKTypeVisible=NULL;   // golden ContactForce.h:346（:460 new、:649-651 delete＋NULL）',
        'std::vector<THTDieForceSLKClass *> DieForceSLKClass;                  // golden ContactForce.h:349',
        'TStringList *slDieForceSLKType=NULL;  // golden ContactForce.h:350',
        'TStringList *slDieForceSLKTypeVisible=NULL;   // golden ContactForce.h:351',
        'std::vector<THTDieForceOneByOneSLKClass *> DieForceOneByOneSLKClass;  // golden ContactForce.h:354',
        'TStringList *slDieForceOneByOneSLKType=NULL;          // golden ContactForce.h:355',
        'TStringList *slDieForceOneByOneSLKTypeVisible=NULL;   // golden ContactForce.h:356',
        'TEdit *IndexZOffsetEdit[2][16];       // golden ContactForce.cpp:19 檔案層全域（ReadFile :972-980 指派成 edtArm1Offset_01…edtArm2Offset_15 的替身；指派前是 NULL，同 golden）',
    ],
    'replace': REPLACE,
    'blocks': BLOCKS,
    'includes': ['cmydef.h', 'cprod.h', 'Config.h', 'LastSet.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'FileRW/ContactForce_Panels.h'],
    # cmydef.h：EP_*／dIndexZOffset／INSTALL_DOUBLE_EP／iIndEPCnt／WEIGHT_CALIBRATION／ATC_SYSTEM／USE_16_HEATER／AccessLevel／iDef*Level／CUSTOMER_CODE；
    # cprod.h：TestIF_File／TestIF／DeviceForm／SaveLastSetIni；Config.h：IniConfig；LastSet.h：LastSet.dIndexLoadRate；
    # common.h：CheckAndReadIniData*／WriteIniData*；MachineType.h：CC_*／CheckRange／SingleSite…；CosFunction.h；
    # vclcompat/SysUtils.h：FileExists；FileRW/ContactForce_Panels.h：四個面板類別
    'decls': [],
    'overrides': [],
}
