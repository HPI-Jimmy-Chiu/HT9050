# -*- coding: utf-8 -*-
# tools/editlist/IniConfig_CounterSel.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only IniConfig_CounterSel
#
# //AI(W906-E031) 20261003 [W906] (St01)：E-031 全面切換第 1 批 —— golden 換成 906 0618（STRUCT['golden']＝'906'，tools/golden_root.py；
#   Jimmy RULINGS_20261003 第 2、4 條）。cCounterSel.cpp／.h／.dfm 0618 與 V912 逐位元組相同 ⇒ 本檔 cCounterSel.cpp 的行號兩棵都對、
#   產生的程式不變（只有檔頭路徑變）；其他 golden 檔的引用改成 0618，括號裡是 V912（Config.h:721-731 是移植樹的 Config.h，不動）。
# AI(W906-CRT-CounterSel) 20260926: 新檔。golden TfCounterSel（cCounterSel.cpp，150 行；20260926～E-030 讀 V912）—— Status.CounterSel.html 的讀寫。
# 結構名 IniConfig_CounterSel（同 TestIF_File_BarCode／TestIF_File_SetUp 的「<結構>_<表單>」慣例）：這張表單沒有自己的結構，
#   它讀寫的是 IniConfig 的 11 個欄位（Config.h:721-731 bShowUPH … iShowCateByArm），經 golden cprod.cpp:2702（V912 :2721）
#   ProcessLastSetIni_Visible(bRead) 讀寫 AuthPath+"config.ini" 的 [Visible] 區段（不是 LastSet.ini；LastSet.* 只是讀取時缺鍵的預設值）。
#   config.ini 的擁有者是 IniConfig（FileRW/IniConfig.cpp，golden SaveLastSetIni cprod.cpp:3092（V912 :3111）也寫同一個 [Visible]）
#   → 併入 IniConfig 的 C 路：兩條存檔流程都經同一份記憶體 IniConfig.bShow*，先讀後寫，不會互蓋。
#   tag 不能用 IniConfig（TfConfiguration 已經用了），所以用 IniConfig_CounterSel。
#
# 轉的 golden 方法：建構子 :23、FormShow :29（開頁）、FormClose :56（golden 關窗＝存檔：Exit 鈕 spbExitClick :145 → Close() → OnClose）。
#   CheckFormIni :111 不轉：只有 FormClose :91 的 FormPos.def 段呼叫它（經 fCounterSel->，本檔把那一段擋掉），
#   另外兩個呼叫點是 golden main.cpp:20925／:24236／:24253（V912 :21622／:24944／:24961；TfMain 的視窗位置，不是這一頁）。
#
# 擋掉的段落（blocks，每一條都寫原因；原文留在 #if 0，存檔時進 ack.todo）：
#   FormClose :72-75  fMain->StatusBar1（主畫面狀態列）與 fShowMessage->lblIndexCycleTime／lblTestTime：
#                     移植樹門面沒有這三個元件（forms/fMain.h 只在註解提到 StatusBar1；forms/fShowMessage.h 沒有兩個 label，20260926 重查）。
#   FormClose :91-103 CheckFormIni＋12 個 ReplaceIniData：寫 config\FormPos.def（BCB6 視窗位置）。網頁視窗位置由 background.html
#                     自己的版面表管；移植樹沒有 FormPos.def 的讀者（全樹 grep 只有 forms/fCounterSel.* 提到，20260926）。
#                     :90 cbDefaultValue->Checked=false 照 golden 留著（勾了之後照樣被清掉，只是不寫檔）。
# 照 golden 留著、沒有擋的：
#   FormClose :77     fShowBinSelect->Tab_UPH->TabVisible —— forms/fShowBinSelect.h:966 有這個元件（cShowBinSelect.cpp，ht9045_sm）。
#   FormClose :79-83  fMain->lbArm0Torque->Caption="" —— forms/fMain.h:1262 現在是真成員（AI(W906-R28TORQ) 20260925，atester.cpp
#                     拿它的 Caption 比對 "1:Reading"）；golden 寫兩次同一行（第二行大概本來要寫 lbArm1Torque），照抄不修。
_F = 'TfCounterSel'


import golden_root as _GR   # AI(W906-E031) 20261003 [W906]：golden 一律經過 tools/golden_root.py（行號＝產生器的行號）

_TREE = _GR.tree_of('IniConfig_CounterSel', '906')   # AI(W906-E031)：E-031 全面切換第 1 批 —— 906 0618；STRUCT['golden'] 同一個值
_cpp = _GR.lines(_TREE, 'cCounterSel.cpp')


def _chk(gl, text):
    """golden 行號核對（golden 改版行號移動時中止，不要默默擋錯段落）。"""
    if text not in _cpp[gl - 1]:
        raise SystemExit('IniConfig_CounterSel.py: golden cCounterSel.cpp:%d is not %r (got %r)' % (gl, text, _cpp[gl - 1]))
    return gl


STRUCT = {
    'struct': 'IniConfig_CounterSel',
    'golden': _TREE,   # AI(W906-E031) 20261003 [W906]：906 0618（tools/golden_root.py）
    'prefix': 'CSL',
    'class': _F,
    'cpp': 'cCounterSel.cpp',
    'h': 'cCounterSel.h',
    'files': ['config\\config.ini [Visible] 11 鍵（golden cprod.cpp:2702（V912 :2721）ProcessLastSetIni_Visible；FormShow 讀、FormClose 寫）',
              'config\\FormPos.def（golden FormClose :91-103，cbDefaultValue 勾選時；本檔擋掉，見檔頭）'],
    'lists': [],
    'methods': ['TfCounterSel', 'FormShow', 'FormClose'],
    'save_methods': ['FormClose'],
    'params': {'TfCounterSel': '', 'FormShow': '', 'FormClose': ''},
    'members': [
        '#define NeedRef (fCounterSel->NeedRef)   // golden cCounterSel.h:56：門面 forms/fCounterSel.h 的那一個。golden 讀者是 TfMain::Timer1Timer '
        'main.cpp:3208-3213（V912 :3299-3304；NeedRef → fTestCategory->SetShowCateMode()＋DoShowUserDefFrom()）；移植樹還沒有這個讀者（cTestCategory.cpp:640 註記）',
        'bool fShow=false;   // golden cCounterSel.h:57 —— 本 TU 自己的（同 FileRW/ShuttleMove.cpp 的理由）：網頁用視窗框的 ✕ 關頁沒有回報，'
        '寫門面 fCounterSel->fShow 會停在 true；golden 讀者 Command.cpp:7356／:9949（V912 :7357／:9957）「有表單開著」那串條件在移植樹仍是 #if 0',
    ],
    'replace': [
        ('FormShow', _chk(51, 'Top=20;'), _chk(52, 'Left=250;'),
         'Top／Left：視窗位置（網頁視窗位置由 background.html 管）', ';'),
    ],
    'blocks': [
        ('FormClose', _chk(72, 'if(IniConfig.bShowIndexTime==false)'), _chk(75, 'fShowMessage->lblTestTime->Visible'),
         'fMain->StatusBar1 與 fShowMessage->lblIndexCycleTime／lblTestTime：主畫面元件，移植樹門面沒有（主畫面 UI）'),
        ('FormClose', _chk(91, 'fCounterSel->CheckFormIni(AuthPath, "FormPos.def")'), _chk(103, '"fLotInfo",         "Y", "defaultY"'),
         'CheckFormIni＋ReplaceIniData：FormPos.def 是 BCB6 視窗位置，網頁不用，不寫（移植樹沒有 FormPos.def 的讀者）'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h',
                 'forms/fMain.h', 'forms/fShowBinSelect.h', 'forms/fCounterSel.h'],
    'decls': [],
    'overrides': [],
}
