# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_FixAICCD.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_FixAICCD
#
# //AI(W906-FRW-S63) 20260926: 新檔（Steven 團隊，S63「TfFixAICCD：開機 DoIniDataToForm 未接、存檔未移植」）。
#   golden V912 TfFixAICCD（FixAICCD.cpp，1422 行，cp950）的讀寫段：<配方>\HandlerCondition.Data [Configuration] 的
#   「Fix2 AI …」10 鍵 → TestIF_File 的 10 個欄位。網頁沒有這一頁（web/page/ 沒有 FixAICCD 頁；HW.HandlerSys 的 rgFixAICCD
#   是 USE_Fix_AI_CCD 安裝選項，不是這個表單）→ 只翻、不登記 PageDesc（同 c675594d Rotate／AutoAlignment 的做法）。
#   一律照 V912（RULINGS 第 37 條）；客戶專屬條件（S25）：轉的兩支方法裡沒有 CUSTOMER_CODE 條件。
#
# 轉的 golden 方法：
#   DoIniDataToForm（:77）  ＝ HSys.asFix2BGAAICCDIP／Port[0..1]＋TestIF_File 9 欄 → 元件；rgResultShowType->Enabled 由
#                             Gerneral.ini [AICCD] LockNoWaitAOIResult 決定 —— ⚠ CheckAndReadIniDataGeneral：鍵不在就照 golden
#                             補寫 D:\HT9045\system\Gerneral.ini（本機 Gerneral.ini:672 已有，不會寫）。
#                             golden 呼叫點：DoReadLastData main.cpp:9410（開機 :9993 與換配方 :25722／:25770）、FormShow :127、
#                             FormClose :136（JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）。
#   spbSaveClick（:669）    ＝ 存檔鈕：WriteIniData ×10 寫 HandlerCondition.Data [Configuration] → ReadFile() → spbSave->Down=false →
#                             fMain->BackupSetupFile()。⚠ 目前沒有觸發點（沒有頁面；FileRW/TestIF_File_FixAICCD.cpp 的
#                             FileRW_FixAICCD_spbSaveClick() 沒有呼叫者）。
# 讀檔器 ReadFile（:95）不在這裡：移植樹 forms/fFixAICCD.cpp 已逐字翻好、開機鏈已接（tools/wb_serve.cpp golden :9370 那一行），
#   spbSaveClick 結尾的 ReadFile() 取代成 fFixAICCD->ReadFile()，全樹只有一份讀檔器。
# 不轉：
#   建構子（:42）：沒有讀寫檔；設的成員（Socket 收訊清單、clntsckt_FixAOI／edAddress／edPort／edCMD 指標表、tmrProcessFixAICCDData
#     啟動、iRetryConnectTimer=15、UnloadAICntNG[]…）DoIniDataToForm／spbSaveClick 都不讀 —— 屬 CCD TCP 通訊與出料流程（Jimmy）。
#     iRetryConnectTimer 的 15 由 ReadFile 的 CheckAndReadIniDataGeneral 蓋過，開機值與 golden 相同。
#   FormShow :124／FormClose :132（頁面，頁面接上時再加進 METHODS）、sbtExitClick :694。
#   ChangeFix2AICCDSetupFile :511（ReadFile 尾巴；移植樹 forms/fFixAICCD.cpp 已翻，捲軸那行 GATE G-FX-BGALIGHT）、
#   scrBGALightValueChange :1272／LightDataReflesh :1290／LightControlUpdate :1297／tmrLightControlTimer :1310（COM2->WriteVisionLight
#     送光源控制器 —— 外部設備動作，S48 歸 Jimmy）。
#   btnFix2AICCD_Connect／Disconnect／Trigger :701-723、ClientSocket_* :581-667、tmrProcessFixAICCDDataTimer :155、
#     TimerDownFixAICCDConnectTimer :375（CCD TCP 通訊，Jimmy）；Fix2AICCDFunction／DoBGAView*（出料流程，Jimmy）；*Click 小鍵盤。
#
# ⚠ golden 看起來錯（照翻，不修）：spbSaveClick 寫「Fix2 AI CCD BGA Light Value」用的是 lblBGALightValue->Caption，而 Caption 只在
#   scrBGALightValueChange（InitialOK 之後）→ LightDataReflesh 才更新。golden 開機時 DoReadLastData（main.cpp:9993）在
#   InitialOK=true（:10898）之前，捲軸 OnChange 第一行就 return；TfMain::FormShow :11328 再設一次同值的 Position 不會觸發
#   OnChange（VCL 值沒變不發事件）→ 開機後沒動過捲軸就按存檔，Light Value 會被寫成 DFM 的 '0'（FixAICCD.dfm:352）。
#   移植樹照 golden：FileRW_FixAICCD_Boot() 把替身 Caption 種成 DFM 的 "0"（產生器的 DfmState 不收 TLabel Caption）。見交件報告決策題。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'FixAICCD.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfFixAICCD'

METHODS = ['DoIniDataToForm', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfFixAICCD::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('TestIF_File_FixAICCD.py: span %s' % meth)


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
    raise SystemExit('TestIF_File_FixAICCD.py: L(%s, %r) not found' % (meth, text))


_WR1 = L('spbSaveClick', 'WriteIniData(szDir, "Configuration", "Fix2 AI CCD Enable",')
_RD = L('spbSaveClick', 'ReadFile();')

REPLACE = [
    # ---- spbSaveClick（golden :669）
    ('spbSaveClick', _WR1, _WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地（前面只有組路徑，沒有任何 return）',
     'filerw::ELMark("FX_WriteIniData"); WriteIniData(szDir, "Configuration", "Fix2 AI CCD Enable",                  '
     '(EL<TCheckBox>("TfFixAICCD", "chkEnableFix2AICCD")->Checked)?1:0);'),
    ('spbSaveClick', _RD, _RD,
     'golden ReadFile（:95）：移植樹 forms/fFixAICCD.cpp 已逐字翻好（開機鏈 golden :9370 也叫它）→ 叫同一支，全樹一份讀檔器。'
     '⚠ 它尾巴的 ChangeFix2AICCDSetupFile 在移植樹把捲軸那行 GATE 掉（G-FX-BGALIGHT），golden 那一行在 InitialOK 之後會經'
     ' OnChange 送 COM2 光源控制（外部設備，Jimmy）',
     'fFixAICCD->ReadFile();'),
]

STRUCT = {
    'struct': 'TestIF_File_FixAICCD',
    'prefix': 'FX',
    'class': _F,
    'cpp': 'FixAICCD.cpp',
    'h': 'FixAICCD.h',
    'files': ['<DataPath>\\<配方>\\HandlerCondition.Data [Configuration] Fix2 AI …（寫：spbSaveClick；讀：移植樹 forms/fFixAICCD.cpp ReadFile）',
              'D:\\HT9045\\system\\Gerneral.ini [AICCD] LockNoWaitAOIResult（讀：DoIniDataToForm；CheckAndReadIniDataGeneral 缺鍵補寫）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'spbSaveClick': ''},
    'members': [],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'cprod.h', 'common.h', 'database.h', 'forms/fFixAICCD.h', 'forms/fMain.h'],
    # cprod.h：TestIF_File；common.h：GetLastOpenFN／DataPath／WriteIniData／CheckAndReadIniDataGeneral；
    # database.h：HSys（asFix2BGAAICCDIP／asFix2BGAAICCDPort，database.h:343-344）；forms/fFixAICCD.h：移植樹門面 fFixAICCD（ReadFile）；
    # forms/fMain.h：fMain->BackupSetupFile（移植樹目前是 offline no-op，forms/fMain.h:270）
    'decls': [],
    'overrides': [],
}
