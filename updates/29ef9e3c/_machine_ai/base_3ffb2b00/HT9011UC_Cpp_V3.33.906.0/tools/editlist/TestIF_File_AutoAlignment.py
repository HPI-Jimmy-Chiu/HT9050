# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_AutoAlignment.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_AutoAlignment
#
# //AI(W906-FRW-NoPage) 20260926: 新檔（Steven 團隊，「golden 會讀寫檔、但網頁沒有頁面」那一批）。
#   golden V912 TfAutoAlignment（AutoAlignment\AutoAlignment.cpp，8996 行，cp950）的讀寫段：<配方>\HandlerCondition.Data
#   [AutoAlignmrnt]（golden 自己的拼法）→ TestIF_File 的 26 個欄位。頁面不做（Steven 20260926）。
#   一律照 V912（RULINGS 第 37 條）；客戶專屬條件（第 25 條）：轉的方法裡沒有 CUSTOMER_CODE 條件
#   （FormShow 的 CC_ASE_KaohSiung、main.cpp:9734 的 #ifdef ASE_KaohSiung 都不在本次範圍）。
#
# ⚠ 哪一支 golden：golden 有兩個 TfAutoAlignment —— AutoAlignment\AutoAlignment.cpp（HT9045.bpr:511 有編）與
#   AutoAlignment\cAutoAlignment.cpp（HT9045.bpr 沒有，死碼；write-inventory.md 二、寫的「AutoAlignment.cpp:426，11 鍵」
#   其實是 cAutoAlignment.cpp:436-450 那一支）。本檔照 bpr 有編的 AutoAlignment.cpp（存檔 :1573，25～28 鍵），同
#   AutoAlignment/AutoAlignment.h 檔頭的判斷。
#
# 轉的 golden 方法：
#   建構子（:47）          ＝ CreateForm（HT9045.cpp:270，TfGroundMan :262 之後）：Gerneral.ini [Auto_Alignment] 四個 CCD 位址／埠
#                             → edtIn/OutArmAddress、edtIn/OutArmPort（CheckAndReadIniDataGeneral：缺鍵照 golden 補寫預設值）。
#   DoIniDataToForm（:1433）＝ TestIF_File → 元件；⚠ 另外 golden 在這裡把 TestIF_File.iAutoAlignmentTrayEvent |= AutoAlignmentTray_AfterHome、
#                             iAutoAlignmentShuttleHotplateEvent |= AutoAlignmentCK_AfterHome（改結構，不只是顯示）。
#                             golden 呼叫點：DoReadLastData main.cpp:9434（開機與換配方）、FormShow :1551、FormClose :1570。
#   spbSaveClick（:1573）   ＝ 存檔鈕：CheckAutoAlignmentEvent()（:1325，元件 → 4 個 Event 欄位）→ 沒有 CCD 就 return →
#                             WriteIniData ×25～28 → ReadFile()。⚠ 目前沒有觸發點（沒有頁面；FileRW/TestIF_File_AutoAlignment.cpp 的
#                             FileRW_AutoAlignment_spbSaveClick() 沒有呼叫者）。
# 讀檔器 ReadFile（:1373）不在這裡：移植樹 AutoAlignment/AutoAlignment.cpp 已逐字翻好、開機鏈已接（tools/wb_serve.cpp golden :9433 那一行），
#   spbSaveClick 結尾的 ReadFile() 取代成 fAutoAlignment->ReadFile()，全樹只有一份讀檔器。
# 不轉：FormShow :1544（SetTechDataToProd、grpInputArea 標題、bNeedHome；頁面接上時再加）、FormClose :1561（bNeedOneCycleByAutoAlignment＋
#   fMain->BtnOneCycleClick —— 機台動作）、cbAutoAlignmentTray_AfterHomeClick :1231（勾選連動 _Z 的 Enabled，頁面事件）、
#   CCD TCP 通訊（TcpipOpen／InArmSendCommand／ClientSocket*，Jimmy）、btnAutoAlignment*PointCalClick（MOT[...] 盤面，Jimmy）、
#   *MouseDown（小鍵盤）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'AutoAlignment', 'AutoAlignment.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfAutoAlignment'

METHODS = ['TfAutoAlignment', 'DoIniDataToForm', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfAutoAlignment::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('TestIF_File_AutoAlignment.py: span %s' % meth)


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
    raise SystemExit('TestIF_File_AutoAlignment.py: L(%s, %r) not found' % (meth, text))


_SELF = re.compile(r'\bfAutoAlignment->(\w+)')


def _proxy(code, qualified):
    """golden 自己用 fAutoAlignment->元件（產生器只改沒有前綴的元件名）→ 具名替身。"""
    el = 'filerw::EL' if qualified else 'EL'
    return _SELF.sub(lambda m: '%s<TCheckBox>("%s", "%s")' % (el, _F, m.group(1)), code)


# golden 檔案層級的自由函式 CheckAutoAlignmentEvent（:1325，spbSaveClick :1579 呼叫）：不是 TfAutoAlignment 的方法，產生器的 body_of
# 抓不到 → 從 golden 原文逐字搬成本 TU 的 static（改名 AA_ 前綴避免與別的 TU 撞名；members 會加 `static `）。
# golden 本體只讀 fAutoAlignment->cb*（18 個 TCheckBox）→ filerw::EL<TCheckBox>(…)（members 在 `using filerw::EL;` 之前輸出）。
def _check_event():
    for i, l in enumerate(_cpp):
        if re.match(r'^void\s+CheckAutoAlignmentEvent\s*\(\s*\)', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    body = [_proxy(x, True) for x in _cpp[i:j + 1]]
                    body[0] = body[0].replace('CheckAutoAlignmentEvent', 'AA_CheckAutoAlignmentEvent', 1)
                    body[-1] += '   // golden AutoAlignment/AutoAlignment.cpp:%d-%d（逐字；fAutoAlignment->cb* → 具名替身）' % (i + 1, j + 1)
                    return '\n'.join(body)
    raise SystemExit('TestIF_File_AutoAlignment.py: CheckAutoAlignmentEvent not found')


_CMD_A = L('TfAutoAlignment', 'sSendCMD[LOAD_FILE]')
_CMD_B = L('TfAutoAlignment', 'sReceiveNGCMD[AUTO_AUTOCLEAN]')
_SOCK_A = L('TfAutoAlignment', 'ClientSocket1->Address')
_SOCK_B = L('TfAutoAlignment', 'ClientSocket2->Port')
if _SOCK_B != _SOCK_A + 3:
    raise SystemExit('TestIF_File_AutoAlignment.py: ClientSocket lines moved (%d..%d)' % (_SOCK_A, _SOCK_B))
_WR1 = L('spbSaveClick', 'WriteIniData(szDir, sGroup, "Enable Auto Alignment",')
_RD = L('spbSaveClick', 'ReadFile();')

REPLACE = [
    # ---- 建構子（golden :47，CreateForm HT9045.cpp:270）
    ('TfAutoAlignment', _CMD_A, _CMD_B,
     'CCD 通訊指令字串表（golden 檔案層級全域 sSendCMD／sReceiveOKCMD／sReceiveNGCMD[TOTAL]，enum ALIGNMENTCMD 在 golden AutoAlignment.h）：'
     '只給 InArmSendCommand／OutArmSendCommand／*GetResult（CCD TCP 通訊，移植樹沒有，Jimmy）用；不是讀寫檔', ';'),
    ('TfAutoAlignment', _SOCK_A, _SOCK_B,
     'TClientSocket 的 Address／Port（CCD TCP 連線，移植樹沒有 ClientSocket1/2）：值留在 edtIn/OutArmAddress、edtIn/OutArmPort 替身裡'
     '（上面四行 CheckAndReadIniDataGeneral 照 golden 讀／補寫 Gerneral.ini），接 CCD 通訊時從替身取', ';'),
    # ---- spbSaveClick（golden :1573）
    ('spbSaveClick', L('spbSaveClick', 'CheckAutoAlignmentEvent();'), L('spbSaveClick', 'CheckAutoAlignmentEvent();'),
     'golden 自由函式 CheckAutoAlignmentEvent（:1325）搬成本 TU 的 AA_CheckAutoAlignmentEvent（見 members；逐字，fAutoAlignment->cb* → 具名替身）',
     'AA_CheckAutoAlignmentEvent();'),
    ('spbSaveClick', _WR1, _WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 CheckAutoAlignmentEvent（改記憶體）與「沒有 CCD 就 return」',
     'filerw::ELMark("AA_WriteIniData"); WriteIniData(szDir, sGroup, "Enable Auto Alignment",                  '
     '(EL<TCheckBox>("TfAutoAlignment", "cbEnabledAutoAlignment")->Checked)?1:0);'),
    ('spbSaveClick', _RD, _RD,
     'golden ReadFile（:1373）：移植樹 AutoAlignment/AutoAlignment.cpp 已逐字翻好（開機鏈也叫它）→ 叫同一支，全樹一份讀檔器',
     'fAutoAlignment->ReadFile();'),
]
# ---- DoIniDataToForm（golden :1433）：golden 自己寫 fAutoAlignment->cbAOA_UseFix1..6（產生器不改帶前綴的名字）→ 逐行換成具名替身
for _n in range(1, 7):
    _gl = L('DoIniDataToForm', 'fAutoAlignment->cbAOA_UseFix%d->Checked' % _n)
    REPLACE.append(('DoIniDataToForm', _gl, _gl,
                    'golden 寫法帶 fAutoAlignment-> 前綴（產生器只改裸名）；同一個元件換成具名替身，其餘逐字',
                    _proxy(_cpp[_gl - 1].split('//')[0].strip(), False)))

STRUCT = {
    'struct': 'TestIF_File_AutoAlignment',
    'prefix': 'AA',
    'class': _F,
    'cpp': 'AutoAlignment/AutoAlignment.cpp',   # 用 /：這個字串會原樣進 ELTodo 的 C 字串（反斜線會變跳脫）
    'h': 'AutoAlignment/AutoAlignment.h',
    'files': ['<DataPath>\\<配方>\\HandlerCondition.Data [AutoAlignmrnt]（寫：spbSaveClick；讀：移植樹 AutoAlignment/AutoAlignment.cpp ReadFile）',
              'D:\\HT9045\\system\\Gerneral.ini [Auto_Alignment] AUTO_ALIGNMENT_CCD1/2_ADRESS／_PORT（讀：建構子；CheckAndReadIniDataGeneral 缺鍵補寫）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'TfAutoAlignment': '', 'spbSaveClick': ''},
    'members': [
        'bool bInArmSendTeachCmd;   // golden AutoAlignment.h:210（public；建構子設 false，CCD 通訊用，移植樹沒有讀者）',
        'bool bOutArmSendTeachCmd;  // golden AutoAlignment.h:211',
        _check_event(),
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'cprod.h', 'common.h', 'MachineType.h', 'AutoAlignment/AutoAlignment.h'],
    # cmydef.h：MACHINE_HAS_AUTO_ALIGNMENT_CCD／AUTO_EMPTY_COLOR／iAutoIndex[]／eFix1..6／AutoAlignmentTray_*／AutoAlignmentCK_*；
    # cprod.h：TestIF_File；common.h：GetRecipeFileName／WriteIniData／CheckAndReadIniDataGeneral；
    # AutoAlignment/AutoAlignment.h：移植樹門面 fAutoAlignment（ReadFile）
    'decls': [],
    'overrides': [],
}
