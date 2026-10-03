# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_QAMode.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_QAMode
#
# Steven 團隊 20260925：TestIF_File 的 TfQAMode 半邊（<recipe>\Tester.Data [QA Mode]／[QA Sampling]）——
# golden TfQAMode（QAMode.cpp，V912，337 行）。結構名加後綴 _QAMode：TestIF_File 已有 A 形狀 FileRW/TestIF_File.cpp（TFTestIF）
# 與 C 形狀 TestIF_File_YieldMonitoring／TestIF_File_SetUp；另一位工程師的 Setup.TesterIF 用別的名字。
# 沒有 HTEditList：存檔鈕 btnApplyClick（:73）＝ A02 守衛 → DoFormToData（:56，widget → TestIF_File）→ 12 個
# WriteIniData（:92-105）→ fMain->BackupSetupFile → ReadFile → DoIniDataToForm → fBinSel->ReadFile。
# C 路 PageSave 在跑 golden 之前先擋 mustSend 缺值（400），所以 golden DoFormToData 直接寫機台正在用的 TestIF_File 也安全。
# 開頁 FormShow（:113）＝ golden 重建 cbQAModeBin／cbbQASampleTray 的 Items → ReadFile（:179）→ DoIniDataToForm → 權限／顯示。
#
# ⚠ golden btnApplyClick **不寫** config.ini。config.ini [Index] QAMode_Backup* 是 QABackupStatus（:286-336）寫的，它的呼叫點是
#   TfMain::SetRunStartMode（main.cpp:546-551，CosFunction.bVerifyMode 且切進／切出 rsmQAMode），不是這張表單 ——
#   照 V912 的讀寫位置，這裡不轉（見 FileRW/TestIF_File_QAMode.cpp 檔頭）。
_F = 'TfQAMode'

STRUCT = {
    'struct': 'TestIF_File_QAMode',
    'prefix': 'QA',
    'class': _F,
    'cpp': 'QAMode.cpp',
    'h': 'QAMode.h',
    'files': ['<recipe>\\Tester.Data [QA Mode]／[QA Sampling]'],
    'lists': [],
    'methods': ['TfQAMode', 'FormShow', 'FormClose', 'ReadFile', 'DoIniDataToForm', 'DoFormToData', 'btnApplyClick',
                'ShowTrayDirectIMG'],
    'save_methods': ['btnApplyClick', 'DoFormToData'],
    'params': {'TfQAMode': '', 'FormShow': '', 'FormClose': '', 'btnApplyClick': ''},
    'members': ['bool fShow=false;   // golden QAMode.h:58（本 TU 自己的；移植樹 fQAMode->fShow 是 forms/fQAMode.h 的另一個，Command.cpp 的讀者仍是 #if 0）'],
    'replace': [
        ('btnApplyClick', 79, 79, 'Close()：golden A02 權限不足時關表單 → 伺服器端記 closed（PageSave 沒寫檔就照 golden FormClose 還原替身）',
         'filerw::ELMark("closed");'),
        ('FormShow', 126, 127, 'Left／Top：視窗位置（HTML 不用；vclcompat 沒有 Width／Height）', ';'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'Config.h', 'LastSet.h', 'cmydef.h', 'common.h', 'MachineType.h', 'CosFunction.h',
                 'vclcompat/SysUtils.h', 'forms/fMain.h', 'forms/fLotInfo.h', 'forms/fBinSel.h'],
    'decls': ['void MyDBIProcess(AnsiString S1, AnsiString S2);   // golden aHotPlateSubstrate.h:924（aHotPlateSubstrate.h 與 HTEditList.h 衝突，只宣告這一支；ShowTrayDirectIMG :274）'],
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'QAMode.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 QAMode.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('TestIF_File_QAMode.py (E030-Q78): golden V912 QAMode.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(75, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(79, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 QAMode.cpp:75-80：A02 分支只有 Close()（:79），後面沒有 return，存檔照跑；主選單 main.cpp:28881 用 Show 開（非對話框），Close() 當場跑 FormClose（:170-176），其中 DoIniDataToForm()（:173，JerryYang 20250411「離開頁面要刷新一次, 避免誤存檔」）先把畫面刷回結構裡的值，再照畫面存檔 ⇒ 還沒進結構的修改被丟掉。'
                'V912 QAMode.cpp:79-80：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('btnApplyClick', _e030q78_expect(80, 'return;'), 80, _E030Q78_WHY, 'return;'))
