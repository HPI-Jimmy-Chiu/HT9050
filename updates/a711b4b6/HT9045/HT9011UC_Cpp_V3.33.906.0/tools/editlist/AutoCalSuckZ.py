# -*- coding: utf-8 -*-
# tools/editlist/AutoCalSuckZ.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only AutoCalSuckZ
#
# //AI(W906-FRW-S70) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S70）。golden V912 TfProductionInfo
#   （ProductionInfo\ProductionInfo.cpp，6242 行，cp950）裡「自動校正吸嘴 Z 高度」（Jimmychiu 20240712）那一段的讀寫：
#   HTEditList elData（golden ProductionInfo.h:552，表單成員）→ D:\HT9045\system\ProductionInfo\AutoCalSuckZ.Data [ArmSuckZAuto]
#   22 鍵 → cmydef.h 的 8 個全域（golden cmydef.cpp:5914-5921）：
#     bEnableInarmSuckZAuto／bEnableOutarmSuckZAuto、iInArmZHeightDiff[2][4]／iOutArmZHeightDiff[2][4]（各 8 鍵，-500..500）、
#     InArmAutoCalSuckZPoint／OutArmAutoCalSuckZPoint（uPoint2D 的 X／Y）、iInArmSearchStartZ／iOutArmSearchStartZ。
#   路徑是 golden 寫死的系統檔（GetProInfoFilePath() ProductionInfo.h:487），不跟配方。一律照 V912（RULINGS 第 37 條）。
#
# 結構名 AutoCalSuckZ：這段沒有 cprod.h 的主結構（值是 cmydef 的散裝全域），同 GroundMan／Rotate 的慣例用檔名當 tag。
#
# golden 讀寫時機（全部 grep 過 V912，20260926）：
#   建構（CreateForm(TfProductionInfo) HT9045.cpp:257 → 建構子 :52）：ZeroMemory 兩個高度差陣列、兩個 Enable=false、
#       elData=new HTEditList、InitAutoCalSuckZ()（22 筆 elData->Add）、兩個 SearchStartZ=0。不讀檔。
#   讀：TfMain::DoReadLastData main.cpp:9430 fProductionInfo->DoIniDataToForm()（開機 FormShow :9993、換配方 ChangeSetUpFile
#       :25722／:25770 兩次）→ DoIniDataToForm :83 **第一行 `if(CosFunction.bOEEFunction==false) return;`** —— 只有 OEE 客戶
#       （CosFunction.cpp:1434 設 true 的那一家，超豐）才會走到結尾的 elData->InitialDataToEdit()＋LoadAutoCalSuckZ()。
#       這是 golden 的行為（設定頁 btnAutoCalSuckZ 也在 cConfiguration 的 [N14] OEE 群組 tsN14_23 底下），照翻。
#       其他讀點：ShowAutoCalSuckZForm :6105（golden 全樹 0 個呼叫者；⚠ 它直接 LoadAutoCalSuckZ，不看 bOEEFunction ——
#       開這一頁就會把檔案值讀進執行中的全域）、btnAutoCalSuckZSaveClick :6135（存完重讀）、FormClose :4684（bShow=false＋
#       DoIniDataToForm()，「離開頁面要刷新一次, 避免誤存檔」；Abort 鈕 btnCancelClick :5931 也是 Close()）。
#   寫：SaveAutoCalSuckZ :6055（elData->SaveEditTextToFile）。golden 呼叫者只有兩個，而且兩個在 V912 都**到不了**：
#       btnAutoCalSuckZSaveClick :6135（在 tsCalibrateSuckZHeight 分頁上；打開那頁的 ShowAutoCalSuckZForm 沒有呼叫者，
#       設定頁的 btnAutoCalSuckZ 在 cConfiguration.dfm:17154 沒有 OnClick）；
#       ainarm9045.cpp:9229 DoInArmAutoCalSuckZ（機台量測流程，golden 全樹 0 個呼叫者；移植樹 ainarm9045.cpp:3105 本體 GATE）。
#   讀到的值的使用者：cinitial.cpp:15127-15145 GetIn/OutArmSuckBaseHeight（吸嘴 Z 高度 += 高度差，Enable 時）——
#       移植樹 cinitial.cpp:16413／:16439 GATE n5-G17／G18（等 fProductionInfo->EnableIn/OutArmAutoCalSuckZ()，本次補在門面
#       forms/fProductionInfo.*；解 GATE 會改變機台 Z 高度，屬 Jimmy，片段見交件報告）。
#
# 轉的 golden 方法：建構子 :52、DoIniDataToForm :83、InitAutoCalSuckZ :6019、SaveAutoCalSuckZ :6055、LoadAutoCalSuckZ :6063、
#   btnAutoCalSuckZSaveClick :6135。
# 不轉：EnableInArmAutoCalSuckZ :6071／EnableOutArmAutoCalSuckZ :6076（讀者在 ht9045_sm，放在門面 forms/fProductionInfo.cpp，
#   不放 wb_serve 專用的 FileRW）；ShowAutoCalSuckZForm :6105（開頁＋視窗位置＋ShowSheet，沒有頁面、golden 也沒有呼叫者）；
#   TfProductionInfo 的 OEE／IPSC／FTP／ESD 全部（N14 超豐客戶功能，要額外移植才編得過 → RULINGS 第 25 條擋並註記）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'ProductionInfo', 'ProductionInfo.cpp'), 'rb').read().decode('cp950', errors='replace') \
    .replace('\r\n', '\n').split('\n')
_F = 'TfProductionInfo'

METHODS = ['TfProductionInfo', 'DoIniDataToForm', 'InitAutoCalSuckZ', 'SaveAutoCalSuckZ', 'LoadAutoCalSuckZ',
           'btnAutoCalSuckZSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfProductionInfo::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('AutoCalSuckZ.py: span %s' % meth)


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
    raise SystemExit('AutoCalSuckZ.py: L(%s, %r) not found' % (meth, text))


def _expect(gl, text):
    if _cpp[gl - 1].strip() != text:
        raise SystemExit('AutoCalSuckZ.py: golden :%d is %r, expected %r' % (gl, _cpp[gl - 1].strip(), text))
    return gl


# ---- 建構子（golden :52）的 OEE／IPSC 兩段
_CT_OEE1_A = L('TfProductionInfo', 'bShow=false;')
_CT_OEE1_B = L('TfProductionInfo', 'cDynaThres=new cDynamicMultiContinualPassBinBySocket;')
_CT_OEE2_A = L('TfProductionInfo', 'bN14_21_UsedGDK04=false;')
_CT_OEE2_B = L('TfProductionInfo', 'bN14_21_SetUpConfig_Success=false;')
# ---- DoIniDataToForm（golden :83）：第一行的 OEE return 照留；OEE 本體到 AutoCalSuckZ 兩行之前擋掉
_DI_OEE_A = L('DoIniDataToForm', 'CopyIPSCback2IPSC();')
_DI_OEE_B = _expect(L('DoIniDataToForm', 'elData->InitialDataToEdit();') - 1, '#endif')

BLOCKS = [
    ('TfProductionInfo', _CT_OEE1_A, _CT_OEE1_B,
     'OEE／IPSC 成員（bShow、bRecFromErrorNote、CompareDataList、bAgreeExportConfig、sIPSCFlag_*、bIPSC*、asTempstring、cDynaThres）：'
     'TfProductionInfo 的 N14 OEE（超豐）功能，不是 AutoCalSuckZ.Data 讀寫段；成員在門面 forms/fProductionInfo.h（bShow 的初值 false '
     '已由門面的預設成員初始化給），其餘 OEE 物件要額外移植（RULINGS 第 25 條擋並註記）'),
    ('TfProductionInfo', _CT_OEE2_A, _CT_OEE2_B,
     'bN14_21_UsedGDK04／InitialOEECount()×2（golden :842，清 OEE 計時與 jam/service 計數）／iAutoClean_IntervalContact_Server／'
     'bN14_21_SetUpConfig_Success：同上，N14 OEE 段，門面沒有 InitialOEECount（要額外移植）'),
    ('DoIniDataToForm', _DI_OEE_A, _DI_OEE_B,
     'OEE 本體（CopyIPSCback2IPSC 讀 IPSC ini、LoadSetting 讀 OEE 設定、複製 7z.exe 到 D:\\HT9045、LoadMOInformation、ESDForm、'
     'tm_PI／tm_IPSCControl 計時器、ACM 訊息、LoadConfigFromServer FTP）：N14 超豐客戶功能，要額外移植才編得過（RULINGS 第 25 條）。'
     '上面第一行 `if(CosFunction.bOEEFunction==false) return;` 照 golden 留著 —— 它同時決定下面 AutoCalSuckZ.Data 讀不讀'),
]

_SV = L('SaveAutoCalSuckZ', 'elData->SaveEditTextToFile(szDir, "AutoCalSuckZ.Data");')

REPLACE = [
    ('SaveAutoCalSuckZ', _SV, _SV,
     '存檔標記：golden 唯一的寫檔點（之前的 MyForceDirectories 只建資料夾）。之後有頁面時當 PageDesc.savedMark 用',
     'filerw::ELMark("ACZ_SaveEditTextToFile"); elData->SaveEditTextToFile(szDir, "AutoCalSuckZ.Data");'),
    ('btnAutoCalSuckZSaveClick', L('btnAutoCalSuckZSaveClick', 'Close();'), L('btnAutoCalSuckZSaveClick', 'Close();'),
     'Close()：golden 存完、重讀之後關表單（沒有條件）；網頁端記 closed。golden Close() 觸發 FormClose :4684'
     '（bShow=false; DoIniDataToForm();「離開頁面要刷新一次, 避免誤存檔」）—— 上一行剛 LoadAutoCalSuckZ 過，DoIniDataToForm '
     '在 OEE 機台只會再讀同一個檔一次（其餘是擋掉的 OEE 本體），非 OEE 機台第一行就 return；資料上等於沒事，不重跑',
     'filerw::ELMark("closed");'),
]

STRUCT = {
    'struct': 'AutoCalSuckZ',
    'prefix': 'ACZ',
    'class': _F,
    'cpp': 'ProductionInfo/ProductionInfo.cpp',   # 用 /：產生器把這個字串原樣放進 ELTodo("golden <cpp>:…") 的 C 字串（同 Rotate.py）
    'h': 'ProductionInfo/ProductionInfo.h',
    'files': ['D:\\HT9045\\system\\ProductionInfo\\AutoCalSuckZ.Data [ArmSuckZAuto] 22 鍵（讀 LoadAutoCalSuckZ＝ReadEditTextFromFile；'
              '寫 SaveAutoCalSuckZ＝SaveEditTextToFile）'],
    'lists': ['elData'],
    'methods': METHODS,
    'save_methods': ['btnAutoCalSuckZSaveClick', 'SaveAutoCalSuckZ'],
    'params': {'TfProductionInfo': '', 'DoIniDataToForm': '', 'btnAutoCalSuckZSaveClick': ''},
    'members': [
        'HTEditList *elData=nullptr;   // golden ProductionInfo.h:552（public 成員）。本 TU 自己的：golden 表單外沒有人碰 elData',
        'AnsiString GetProInfoFilePath(){return "D:\\\\HT9045\\\\system\\\\ProductionInfo";}   '
        '// golden ProductionInfo.h:487（inline 成員，逐字）—— golden 寫死的系統路徑，不跟配方、不經 DataPath',
    ],
    'replace': REPLACE,
    'blocks': BLOCKS,
    'includes': ['cmydef.h', 'CosFunction.h', 'common.h'],
    # cmydef.h：8 個全域（bEnableIn/OutarmSuckZAuto、iIn/OutArmZHeightDiff、In/OutArmAutoCalSuckZPoint、iIn/OutArmSearchStartZ）、
    #   uPoint2D、MAX_ARM_Row／MAX_ARM_Col；CosFunction.h：CosFunction.bOEEFunction；common.h：MyForceDirectories
    'decls': [],
    'overrides': [],
}
