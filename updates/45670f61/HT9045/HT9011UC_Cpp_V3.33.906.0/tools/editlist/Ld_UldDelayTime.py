# -*- coding: utf-8 -*-
# tools/editlist/Ld_UldDelayTime.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔；Steven 20260924 拆檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py [--only Ld_UldDelayTime]
# Steven 20260924：Ld_UldDelayTime（UdUld.Data，elUdUld）—— golden TfLd_ULd（cLd_ULd.cpp，263 行）。
# 建構子就是註冊（golden 在 HT9045.cpp:191 CreateForm 時跑，早於 TfConfiguration :207）；
# 開頁 FormShow（ReadFile＋權限），存檔鈕 spbSaveClick → SaveSetupFile → elUdUld->SaveEditTextToFile。
STRUCT = {
    'struct': 'Ld_UldDelayTime',
    'prefix': 'LU',
    'class': 'TfLd_ULd',
    'cpp': 'cLd_ULd.cpp',
    'h': 'cLd_ULd.h',
    'files': ['UdUld.Data'],
    'lists': ['elUdUld'],
    'methods': ['TfLd_ULd', 'FormShow', 'ReadFile', 'DoIniDataToForm', 'FormClose', 'spbSaveClick', 'SaveSetupFile'],
    'save_methods': ['spbSaveClick', 'SaveSetupFile'],
    'params': {'TfLd_ULd': '', 'FormShow': '', 'FormClose': '', 'spbSaveClick': ''},
    'members': ['AnsiString LastFileName;   // golden cLd_ULd.h: TfLd_ULd::LastFileName',
                'bool fShow=false;          // golden cLd_ULd.h: TfLd_ULd::fShow'],
    'replace': [
        ('FormShow', 90, 90, 'LoadImage()：示意圖（HTML 自己有）', ';'),
        ('FormShow', 94, 95, 'Caption／SetDefaultPos()：視窗標題與位置（HTML 不用）', ';'),
        ('FormClose', 173, 173, 'rbTemp->SetFocus()：焦點（HTML 端）', ';'),
        ('spbSaveClick', 185, 185, 'Close()：golden 權限不足時關表單 → 伺服器端記 closed（ack.trace），頁面自己關',
         'filerw::ELMark("closed");'),
    ],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fSecurity.h',
                 'SECSGEM/SecsEventType.h', 'SECSGEM/SecsEventReport.h'],
    'blocks': [],
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'cLd_ULd.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 cLd_ULd.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('Ld_UldDelayTime.py (E030-Q78): golden V912 cLd_ULd.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(181, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(185, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 cLd_ULd.cpp:181-186：A02 分支只有 Close()（:185），後面沒有 return，存檔照跑；主選單 main.cpp:27463 用 ShowModal 開（對話框），Close() 只設 ModalResult ⇒ 操作員改的值照樣寫進檔。'
                'V912 cLd_ULd.cpp:185-186：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('spbSaveClick', _e030q78_expect(186, 'return;'), 186, _E030Q78_WHY, 'return;'))
