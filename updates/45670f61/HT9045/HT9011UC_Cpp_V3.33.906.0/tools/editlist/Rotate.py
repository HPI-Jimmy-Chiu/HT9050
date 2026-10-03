# -*- coding: utf-8 -*-
# tools/editlist/Rotate.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only Rotate
#
# //AI(W906-FRW-NoPage) 20260926: 新檔（Steven 團隊，「golden 會讀寫檔、但網頁沒有頁面」那一批）。
#   golden V912 TFrmRotate（RotateKit\fRotate.cpp，1402 行，cp950）的讀寫段：<配方>\Rotate.Data [SETTING] → 全域 tRotate
#   （golden RotateKit/fRotate.h:28-59 struct TRotate；本次整個搬到 forms/fRotate.h，aHotPlateSubstrate.h 的 tRotateShim
#   改成它的 typedef）＋ strIn/OutRotateDutAngle（cmydef.h:4595-4596，SECS EC 16148/16149 讀的兩個字串）。
#   一律照 V912（RULINGS 第 37 條）；客戶專屬條件（第 25 條）：本表單的讀寫段沒有 CUSTOMER_CODE 條件
#   （CC_PTI／CC_KYEC_LEE 只在 FormShow，本次沒轉）。頁面不做（Steven 20260926「先以大量把讀寫檔進行移植為首要工作」）。
#
# 結構名 Rotate：golden 的全域是 tRotate（在 fRotate.h，不在 cprod.h），同 GroundMan／ShuttleMove 的慣例用表單去掉前綴。
#
# 轉的 golden 方法：
#   建構子（:42）          ＝ CreateForm（HT9045.cpp:227）：bShowRotateBySite（USE_ROTATE_KIT／iRotate_Type）、Rotate_Offset[]／
#                             Pre_Rotate[] 指向兩組輸入框、iRotateDutDate 清零、iFromTrayAngle=0
#   fRotate_ReadFile（:241）＝ 讀 Rotate.Data → tRotate＋strIn/OutRotateDutAngle。只用 ReadIniData（不會補寫缺鍵）。
#                             golden 呼叫點：DoReadLastData main.cpp:9361（開機／換配方 :25722、:25770）、FormShow :131、
#                             spbSaveClick :1190
#   DoIniDataToForm（:586） ＝ tRotate → 元件＋iFromTrayAngle／iRotateDutDate（表單成員，spbSaveClick 用）；結尾 SetWorkParameter()。
#                             golden 呼叫點：DoReadLastData main.cpp:9407、FormShow :132、FormClose :1223
#   spbSaveClick（:1036）   ＝ 存檔鈕：A02 → 「與入料角度相同不存檔」→ Active Rotate 流程保護 → ART RT 檢查 → WriteIniData 寫
#                             Rotate.Data → fRotate_ReadFile → bHasSaveSet → fMain->BackupSetupFile()。
#                             ⚠ 目前沒有觸發點（沒有頁面；FileRW/Rotate.cpp 的 FileRW_Rotate_spbSaveClick() 沒有呼叫者）。
# 不轉（頁面／顯示，頁面接上時再加進 METHODS）：FormShow :124、SetGridDraw :643、DrawState :781、sgRotateInDrawCell :827、
#   rgDutNumClick :840、sgRotateInSelectCell :853（點格子改角度 → iRotateDutDate）、sgFTADrawCell／SelectCell :992／:998、
#   sbtExitClick :1210、FormClose :1218、rgSelectClick :1321、cbOutRotateDifferentAngleClick :1345、chkRotateUseRTmodeClick :1377、
#   XPitchKeyPress／YPitchMouseDown／edIn*MouseDown（小鍵盤）、FormCreate :85（建 64 個 TfrmRot 動態表單，aRotateDegreeClass.cpp，移植樹沒有）。
# 不轉（機台動作，已在移植樹）：InitialIn/OutRotateHome、DoIn/OutRotateHome（forms/fRotate.cpp、RotateKit/fRotate.cpp）、
#   SetIn/OutRotateSpeed :1297／:1309（馬達速度，Jimmy）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'RotateKit', 'fRotate.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TFrmRotate'

METHODS = ['TFrmRotate', 'fRotate_ReadFile', 'DoIniDataToForm', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTFrmRotate::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('Rotate.py: span %s' % meth)


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
    raise SystemExit('Rotate.py: L(%s, %r) not found' % (meth, text))


def _expect(gl, text):
    if _cpp[gl - 1].strip() != text:
        raise SystemExit('Rotate.py: golden :%d is %r, expected %r' % (gl, _cpp[gl - 1].strip(), text))
    return gl


# golden 檔案層級的 static 函式 IsRotateFlowNotFinish（:28-41，KevinCheng 20260318）：不是 TFrmRotate 的方法，產生器的
# body_of 抓不到 → 從 golden 原文逐字搬成本 TU 的 static（members 會加 `static `）。
def _free_function(name):
    for i, l in enumerate(_cpp):
        if re.match(r'^static\s+bool\s+' + name + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    body = '\n'.join(_cpp[i:j + 1])
                    body = re.sub(r'^static\s+', '', body)   # members 自己會加 static
                    return body + \
                        '   // golden RotateKit/fRotate.cpp:%d-%d（逐字）' % (i + 1, j + 1)
    raise SystemExit('Rotate.py: free function %s not found' % name)


_CTOR_LOOP_A = L('TFrmRotate', 'for(int i=0; i<sgRotateIn->ColCount; i++)')
_CTOR_LOOP_B = _expect(L('TFrmRotate', 'bShowRotateBySite=false;') - 1, '}')
_ART_A = L('spbSaveClick', 'for(int iR=0; iR<sgRotateIn->RowCount; iR++)')
_ART_B = _expect(_ART_A + 7, '}')
_WR1 = L('spbSaveClick', 'WriteIniData(szDir, "SETTING", "ActiveRotate",')

REPLACE = [
    # ---- 建構子（golden :42，CreateForm HT9045.cpp:227）
    ('TFrmRotate', _CTOR_LOOP_A, _CTOR_LOOP_B,
     '純畫面：非 by-site 時把 sgRotateIn／sgRotateOut（與 RT 兩張）的格子清成 0。具名替身 filerw::ELStringGrid 沒有 ColCount／'
     'RowCount（FileRW/_EditList.h:107-111 只有 Cells），而且格子內容 golden 開頁 SetGridDraw（:643，FormShow :134）會整張重畫；'
     '檔案與 tRotate 都不受影響', ';'),
    # ---- fRotate_ReadFile（golden :241）
    ('fRotate_ReadFile', L('fRotate_ReadFile', 'FrmRotDegree[0][i][j]->SetDegree('), L('fRotate_ReadFile', 'FrmRotDegree[0][i][j]->SetDegree('),
     '純畫面：FrmRotDegree[2][4][8] 是 golden FormCreate（:85）建的 64 個 TfrmRot 動態表單（RotateKit/aRotateDegreeClass.cpp，移植樹沒有）；'
     'SetDegree（aRotateDegreeClass.cpp:49-58）只設 rgRotateDegree->ItemIndex 與 ReloadPicture。下一行 tRotate.RotateDutDate[1] 的'
     '就地改寫（資料）照 golden 保留', ';'),
    ('fRotate_ReadFile', L('fRotate_ReadFile', 'FrmRotDegree[1][i][j]->SetDegree('), L('fRotate_ReadFile', 'FrmRotDegree[1][i][j]->SetDegree('),
     '同上（Out 那一組 TfrmRot 的顯示）', ';'),
    # ---- spbSaveClick（golden :1036）
    ('spbSaveClick', L('spbSaveClick', 'Close();'), L('spbSaveClick', 'Close();'),
     'Close()：golden A02（Operator 權限）時關表單（下一行 return，golden AI 20260907 註解）；網頁端記 closed，沒寫檔',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', _WR1, _WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地 —— 前面的 A02、「與入料角度相同不存檔」、Active Rotate 流程保護、'
     'ART RT 檢查都在它之前 return（沒寫檔）',
     'filerw::ELMark("RT_WriteIniData"); WriteIniData(szDir, "SETTING", "ActiveRotate",  EL<TCheckBox>("TFrmRotate", "cbActiveRotate")->Checked);'),
]

BLOCKS = [
    ('spbSaveClick', _ART_A, _ART_B,
     'ART RT No Rotate 的「每格角度都要相同」檢查掃 sgRotateIn->RowCount×ColCount：替身沒有格子大小（golden SetGridDraw :650-682 在開頁時'
     '依 rgDutNum 設成 4x2／1x1／2x1／2x2），而且 golden 的 iR 直接當 iRotateDutDate 第二維（上限 4）用，格子大小不對會越界 —— '
     '頁面接上（FormShow＋SetGridDraw 加進 METHODS）之前不跑，回報 todo'),
    ('spbSaveClick', L('spbSaveClick', 'WriteIniData(szDir, "SETTING", "ColCount",      sgRotateIn->ColCount)'),
     L('spbSaveClick', 'WriteIniData(szDir, "SETTING", "RowCount",      sgRotateIn->RowCount)'),
     'ColCount／RowCount 寫的是 sgRotateIn 的格子大小（golden SetGridDraw 開頁時設）：替身沒有，照寫會把錯的值寫進 Rotate.Data —— '
     '頁面接上之前不寫，回報 todo（這兩鍵保持檔案原值）'),
]

STRUCT = {
    'struct': 'Rotate',
    'prefix': 'RT',
    'class': _F,
    'cpp': 'RotateKit/fRotate.cpp',   # 用 /：產生器把這個字串原樣放進 ELTodo("golden <cpp>:…") 的 C 字串，反斜線會變成跳脫（\f＝換頁）
    'h': 'RotateKit/fRotate.h',
    'files': ['<DataPath>\\<配方>\\Rotate.Data [SETTING]（讀 fRotate_ReadFile：只有 ReadIniData；寫 spbSaveClick：WriteIniData）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'TFrmRotate': '', 'spbSaveClick': ''},
    'members': [
        # golden TFrmRotate 的 public 成員：golden 表單外有讀者 → 放在門面 forms/fRotate.h，這裡接過去（同 GroundMan 的做法）
        '#define bShowRotateBySite (FrmRotate->bShowRotateBySite)   // golden fRotate.h:142 → forms/fRotate.h（main.cpp:32717、aRotateKIT_In/Out 讀）',
        '#define bIsAngleZero (FrmRotate->bIsAngleZero)             // golden fRotate.h:136 → forms/fRotate.h（csystem.cpp:18865 讀）',
        '#define iRotateDutDate (FrmRotate->iRotateDutDate)         // golden fRotate.h:138 → forms/fRotate.h（aRotateDegreeClass.cpp:100 寫）',
        '#define iFromTrayAngle (FrmRotate->iFromTrayAngle)         // golden fRotate.h:139 → forms/fRotate.h（ainarm9045.cpp:2712 讀）',
        # golden TestSocket.iShtRow／iShtCol（fRotate_ReadFile :306-307 by-site 時的格數）：aHotPlateSubstrate.h 與 HTEditList.h 同名類別衝突，
        # 經 FileRW/_KitSuck.cpp 的 FileRW_KitSuckDims 轉接（同 tools/editlist/StartCondition.py 的 SC_KitOf）
        '#define TestSocket RT_KitOf(0)                             // golden TestSocket.iShtRow／iShtCol → FileRW/_KitSuck.cpp',
        # golden fRotate.cpp:25-26 檔案層級全域（只有本檔用；golden 全樹 grep 只有 RotateKit/fRotate.cpp）
        'TEdit *Rotate_Offset[2];                                   // golden RotateKit/fRotate.cpp:25 //Ifor 20251210 add:Rotate Offset',
        'TEdit *Pre_Rotate[2];                                      // golden RotateKit/fRotate.cpp:26 //Ifor 20260901 add:Pre Rotate degree before pick',
        _free_function('IsRotateFlowNotFinish'),
    ],
    'replace': REPLACE,
    'blocks': BLOCKS,
    'includes': ['cmydef.h', 'Config.h', 'common.h', 'MachineType.h', 'CosFunction.h', 'forms/fRotate.h', 'forms/fMain.h',
                 'cinitial.h', 'RotateKit/aRotateKIT.h'],
    # cmydef.h：USE_ROTATE_KIT／iRotate_Type／USE_PICKER_COUNT／AccessLevel／bWaitRotateFinish／bCanRunSCKART／bHasSaveSet／
    #   strIn/OutRotateDutAngle；Config.h：IniConfig（A02、bEnable_SECS_GEM）；common.h：DataPath／GetLastOpenFN／ReadIniData／WriteIniData；
    # MachineType.h：e4MotRotate…eInOutArm1Motor、ep1Picker；CosFunction.h：bPassBinNoRotate／bRotateUseRTmode／bART_RT_NoRotate；
    # forms/fRotate.h：FrmRotate、struct TRotate／tRotate、tDutType_*；forms/fMain.h：fMain->BackupSetupFile；cinitial.h：SetWorkParameter；
    # RotateKit/aRotateKIT.h：iInArmRotateKit／iOutArmRotateKit（IsRotateFlowNotFinish）
    'decls': [
        'void FileRW_KitSuckDims(int which, int* shtRow, int* shtCol, int* maxRow, int* maxCol);   // FileRW/_KitSuck.cpp（0＝TestSocket）',
        '// golden TMyKitSuck 的四個格數欄位的轉接物件：TestSocket 巨集展開成它（aHotPlateSubstrate.h 與 HTEditList.h 的 TList 重複定義）',
        'struct RT_Kit { int iShtRow, iShtCol, iMaxRow, iMaxCol; };',
        'static inline RT_Kit RT_KitOf(int w) { RT_Kit k; FileRW_KitSuckDims(w, &k.iShtRow, &k.iShtCol, &k.iMaxRow, &k.iMaxCol); return k; }',
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'RotateKit', 'fRotate.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 fRotate.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('Rotate.py (E030-Q78): golden V912 fRotate.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(1038, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(1042, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 fRotate.cpp:982-987：A02 分支只有 Close()（:986），後面沒有 return，存檔照跑；主選單 main.cpp:29653 用 Show 開（非對話框），Close() 當場跑 FormClose（:1141-1150），其中 DoIniDataToForm()（:1146，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。'
                'V912 fRotate.cpp:1042-1043：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）；這句 return 是 V912 AI(ht9045-a012-save-guard) 20260907 加的（原註解在下面 #if 0 裡）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('spbSaveClick', _e030q78_expect(1043, 'return;'), 1043, _E030Q78_WHY, 'return;'))
