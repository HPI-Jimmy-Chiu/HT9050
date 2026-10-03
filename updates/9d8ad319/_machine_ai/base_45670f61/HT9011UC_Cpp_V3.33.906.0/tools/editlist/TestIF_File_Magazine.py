# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_Magazine.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_Magazine
#
# //AI(W906-FRW-S62) 20260926: 新檔（Steven 團隊，S62「TfMagazine：讀已接（magazine.* 鏈），存檔未移植」）。
#   golden V912 TfMagazine（Magazine.cpp，4475 行，cp950）的讀寫段：
#     <配方>\HandlerCondition.Data [Configuration] 三鍵（Tray Magazine Source／Mag Fix tray Type／Mag display order）→ TestIF_File
#     AuthPath+config.ini（D:\HT9045\config\config.ini）[Magazine Z Offset] 32 鍵 → fMagazine->iInMagOfs[16]／iOutAutoOfs[16]
#   網頁沒有這一頁（web/page/ 沒有 Magazine 頁；Setup.Speed／HW.teach 的 tsMagazine 是別的表單的分頁）→ 只翻、不登記 PageDesc
#   （同 c675594d Rotate／AutoAlignment 的做法）。一律照 V912（RULINGS 第 37 條）；客戶專屬條件（S25）：轉的方法裡沒有
#   CUSTOMER_CODE 條件（CC_KYEC_LEE 在 ReadFile —— 移植樹 Magazine.cpp 已翻 —— 與 FormShow，本次沒轉 FormShow）。
#
# 轉的 golden 方法：
#   建構子（:44）          ＝ CreateForm（HT9045.cpp:274）：fShow=false、EditInMag[16]／EditOutAuto[16] 指向 32 個輸入框（存檔與
#                             DoIniDataToForm 用這兩張表）。不讀寫檔。
#   DoIniDataToForm（:3654）＝ TestIF_File 三欄＋iInMagOfs／iOutAutoOfs → 元件。只寫替身，不改結構、不讀寫檔。
#                             golden 呼叫點：FormShow :88、FormClose :354、spbSaveClick :3609 —— golden DoReadLastData 沒有呼叫它
#                             （main.cpp:9381-9411 的 DoIniDataToForm 段沒有 fMagazine），所以開機鏈不接。
#   spbSaveClick（:3547）   ＝ 存檔鈕：寫 Tray Magazine Source → Fix tray 當 Magazine buffer 的三道檢查（HT9046LS＋FIX3、盤面／
#                             Device 尺寸 ≥40mm、Fix 已設 Bin）→ 寫 Mag Fix tray Type（檢查不過只跳訊息、這一鍵不寫）→ 寫 config.ini
#                             32 鍵 → 寫 Mag display order → 顯示順序有變就 fShowBinSelect->InitShowBinDigital() → ReadFile() →
#                             DoIniDataToForm()。⚠ 目前沒有觸發點（沒有頁面；FileRW/TestIF_File_Magazine.cpp 的
#                             FileRW_Magazine_spbSaveClick() 沒有呼叫者）。
# 讀檔器 ReadFile（:3613）不在這裡：移植樹 Magazine.cpp 已逐字翻好、開機鏈已接兩次（tools/wb_serve.cpp golden :9351／:9357），
#   spbSaveClick 裡的 ReadFile() 取代成 fMagazine->ReadFile()，全樹只有一份讀檔器。
# 不轉：
#   FormShow :82（頁面接上時再加）—— ⚠ 除了 ReadFile＋DoIniDataToForm 與權限（CanChangeData、fMain->CheckCanChangeRealDummy），
#     iMagFixTrayType==1 時還會改機台盤面資料（MOT[MManualTray1..3]／MOT[iMMAuto[]].SetTraySingleData、全域 iYRegNum）——
#     不是讀寫檔、是改執行期狀態，頁面接上時要另外決定（見交件報告）。
#   FormClose :350（fShow=false＋DoIniDataToForm，頁面）、sbtExitClick :3667、Button10Click :357（fMain->BtnStartClick —— 機台啟動）、
#   btnMagazineTrayOutClick :3711（Magazine 出盤流程 InitialDoMagazineTrayFeedTask —— 機台動作，Jimmy）、
#   rgMagFixTypeClick :3695／rgMagTraySourceClick :3703（golden 看起來錯：`if(CanChangeData(false)) return;` 之後本體就結束，
#     成立與否都什麼也不做 —— 註解說「機台內有IC不能更改」，但實際沒有擋；頁面事件，本次不轉）、edInMagOfs01Click（小鍵盤）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'Magazine.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfMagazine'

METHODS = ['TfMagazine', 'DoIniDataToForm', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfMagazine::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('TestIF_File_Magazine.py: span %s' % meth)


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
    raise SystemExit('TestIF_File_Magazine.py: L(%s, %r) not found' % (meth, text))


_WR1 = L('spbSaveClick', 'WriteIniData(szDir, "Configuration", "Tray Magazine Source",')
_RD = L('spbSaveClick', 'ReadFile();')

REPLACE = [
    # ---- spbSaveClick（golden :3547）
    ('spbSaveClick', _WR1, _WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）',
     'filerw::ELMark("MG_WriteIniData"); WriteIniData(szDir, "Configuration", "Tray Magazine Source", '
     '(EL<TRadioGroup>("TfMagazine", "rgMagTraySource")->ItemIndex)?1:0);'),
    ('spbSaveClick', _RD, _RD,
     'golden ReadFile（:3613）：移植樹 Magazine.cpp 已逐字翻好（開機鏈 golden :9351／:9357 也叫它）→ 叫同一支，全樹一份讀檔器',
     'fMagazine->ReadFile();'),
]

STRUCT = {
    'struct': 'TestIF_File_Magazine',
    'prefix': 'MG',
    'class': _F,
    'cpp': 'Magazine.cpp',
    'h': 'Magazine.h',
    'files': ['<DataPath>\\<配方>\\HandlerCondition.Data [Configuration] Tray Magazine Source／Mag Fix tray Type／Mag display order'
              '（寫：spbSaveClick；讀：移植樹 Magazine.cpp ReadFile）',
              'AuthPath+config.ini（D:\\HT9045\\config\\config.ini）[Magazine Z Offset] In Magazine Offset 0..15／Out Auto3 Offset 0..15'
              '（寫：spbSaveClick；讀：移植樹 Magazine.cpp ReadFile）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'TfMagazine': '', 'spbSaveClick': ''},
    'members': [
        'bool fShow;                                   // golden Magazine.h:137（建構子設 false；golden 表單外沒有讀者）',
        'TEdit *EditInMag[16];                         // golden Magazine.h:140（建構子指向 edInMagOfs01..16 替身）',
        'TEdit *EditOutAuto[16];                       // golden Magazine.h:141（建構子指向 edOutAuto3Ofs01..16 替身）',
        # golden 表單外有讀者（Magazine.cpp:3042／:3044 DoMagazineUpDown 的 Z 位置補償）→ 門面 Magazine.h 的那兩張表（移植樹 ReadFile 填的）
        '#define iInMagOfs (fMagazine->iInMagOfs)      // golden Magazine.h:142 → 移植樹門面 Magazine.h（ReadFile 填；golden Magazine.cpp:3044 讀）',
        '#define iOutAutoOfs (fMagazine->iOutAutoOfs)  // golden Magazine.h:143 → 移植樹門面 Magazine.h（ReadFile 填；golden Magazine.cpp:3042 讀）',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'cprod.h', 'common.h', 'MachineType.h', 'Magazine.h', 'forms/fShowBinSelect.h',
                 'vclcompat/SysUtils.h'],
    # cmydef.h：MachineTypeChoice／FIX3_INSTALL／FIX3_FULL_PLACE／iFixRight／iFixRightHalf／iTestBinCount；
    # cprod.h：TestIF_File／Prod（iT6PosCate／iIfErrorT6）／LoadForm／DeviceForm_File；common.h：AuthPath／GetRecipeFileName／WriteIniData；
    # MachineType.h：Type_HT9046_LS／Fix3K_Uninstall／ePosFix1；Magazine.h：移植樹門面 fMagazine（ReadFile、iInMagOfs／iOutAutoOfs）；
    # forms/fShowBinSelect.h：fShowBinSelect->InitShowBinDigital（forms/fShowBinSelect.h:1019）；vclcompat/SysUtils.h：IntToStr
    'decls': [],
    'overrides': [],
}
