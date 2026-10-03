# -*- coding: utf-8 -*-
# tools/editlist/IniConfig_OCR.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only IniConfig_OCR
#
# //AI(W906-FRW-S86) 20260926: 新檔（Steven 團隊，S86「AOI.Data 的 [OCR SETTING]（golden TfOCR）」；S69 交件時發現）。
#   golden V912 TfOCR（OCR.cpp，2681 行，cp950）的讀寫段：<配方>\AOI.Data 的 [OCR SETTING] 19 鍵 → 全域 IniConfig 的 OCR 欄位
#   （Config.h:67-75、:246、:258-268），另外 system\Gerneral.ini [OCR SETTING] "OCR Port" 一鍵 → IniConfig.iOCRPort（CheckAndReadIniData，
#   缺鍵照 golden 補寫）。
#   結構名 IniConfig_OCR（同 IniConfig_CounterSel 的「<結構>_<表單>」慣例）：TfOCR 沒有自己的結構，讀寫的是 IniConfig 的欄位；
#   tag 不能用 IniConfig（TfConfiguration 已用）。前綴 OC。
#   網頁沒有這一頁（web/page/ 沒有 OCR 頁）→ 只翻、不登記 PageDesc（同 c675594d Rotate、c79ee4e9 FixAICCD、9ec84450 AOISetup）。
#   一律照 V912（RULINGS 第 37 條）；客戶專屬（S25）：FormShow 的 CC_KYEC_LEE／bSPILFunction 只是兩個勾選框的顯示，編得過 → 照 golden 留。
#   OCR 相機／光源通訊與動作不接（外部設備，歸 Jimmy）：FormShow 的 CreateOCRSimulationControls（V899 AI 加的模擬分頁）與兩張圖、
#   btSend 的外觀擋掉；存檔沒有設備動作（golden 存檔鈕不碰相機／光源）。
#
# AOI.Data 是兩個表單共用的檔：TFrmAOI（FileRW/AOISetup.*，S69）寫 [SETTING]／[DutOnOff_*]／[RS232]／[AOITRAY]，本表單寫 [OCR SETTING]。
#   兩邊 golden 都是 WriteIniData 逐鍵寫（移植樹 common.cpp WriteIniData → vclcompat TIniFile::WriteString，write-through ＝
#   Win32 WritePrivateProfileStringA 就地改一行，IniFiles.cpp EOF 的註解）；讀都是 ReadIniData（每次讀磁碟現況）。沒有整檔回寫 → 不會互蓋。
#   AOISetup 的 Top&Bottom elParameter->SaveEditTextToFile（HTEditList 整份寫）在移植樹是擋掉的；golden 的 HTEditList 也只寫自己註冊的鍵。
#
# 轉的 golden 方法：
#   fOCR_ReadFile（:2197）       ＝ AOI.Data [OCR SETTING] → IniConfig 19 欄（讀寫同一組 19 鍵、鍵名一致）＋ Gerneral.ini "OCR Port" → iOCRPort；iOCRTriggerMode 夾 0..1；
#                                   INSTALL_OCR!=eocrUninstal 時依 TestIF_File.bOcrFunction／b2DUsePinInspection 送 4 種 MSG_CMD 給 GPIB 橋
#                                   （fMain->SendMSG_CMD，移植樹門面 forms/fMain.cpp:446 是 offline no-op；照 golden 留，同 TestIF_File_BarCode）。
#                                   golden 呼叫點：DoReadLastData main.cpp:9365（開機 :9993、換配方 :25722／:25770；移植樹都走
#                                   tools/wb_serve.cpp 的 W906_DoReadLastData）、FormShow :216、spbSaveClick :2187。
#   fOCR_DoIniDataToForm（:2247）＝ IniConfig → 元件（替身）＋三個分頁的 TabVisible（INSTALL_OCR 與 CosFunction.bTrayOCR）。
#                                   golden 只從 FormShow :217 呼叫（DoReadLastData 沒有呼叫它）。
#   FormShow（:211）             ＝ 開頁：bShow、fOCR_ReadFile、fOCR_DoIniDataToForm、兩個勾選框的客戶別 Visible。沒有頁面 → 目前沒有呼叫者；
#                                   轉它是為了存檔前能照 golden 先把替身灌成檔案值（見 spbSaveClick 的 ⚠）。
#   spbSaveClick（:2151）        ＝ 存檔鈕：A02（Operator 權限不能存）→ WriteIniData 逐鍵寫 AOI.Data [OCR SETTING] 19 鍵 → fOCR_ReadFile →
#                                   fMain->BackupSetupFile()（移植樹 offline no-op，forms/fMain.h:270）。
#                                   ⚠ 目前沒有觸發點（沒有頁面；FileRW/IniConfig_OCR.cpp 的 FileRW_IniConfig_OCR_spbSaveClick() 沒有呼叫者）。
#
# 不轉：建構子（:72，sOCR_Send／sOCR_Recv 協定字串、Tester 參數、ListOCR*、模擬 UI 指標；沒有讀寫檔）、FormClose、sbtExitClick、
#   *Click 小鍵盤、edSetBlue/RedLightChange（送光源值，設備）、spbResetComClick／RS232Init（COM 口，設備）、socket／OCR 流程全部（Jimmy）。
#
# ⚠ golden 看起來錯（照翻，不修；交件報告列為決策題）：
#   (1) V912 OCR.dfm 沒有 rgOCRTriggerMode 物件（V899 OCR.dfm:479-490 有：'Switch Trigger'／'Command Trigger(SE8)'、ItemIndex=0），
#       OCR.h:81 有宣告、OCR.cpp:2181／:2278 有用 ⇒ V912 執行期這個指標是 NULL：開 OCR 表單（FormShow → fOCR_DoIniDataToForm :2278）
#       與按存檔（:2181）都會存取違規 —— VCL 攔下、跳 Access violation 錯誤框，該事件後半不執行：開頁時 :2279-2282 四個勾選框
#       沒顯示、ShowOCRImg／Visible 沒跑；存檔時前 14 鍵已寫、後 5 鍵沒寫、沒有重讀、沒有備份。D:\HT9045 的 V912 工作樹與 V910_HT9050 樹同樣缺
#       （20260926 grep）。移植樹的具名替身一定存在，所以不會出錯，
#       ItemIndex 預設 0（vclcompat Controls.h:454）＝ V899 DFM 的 ItemIndex=0；V912 DFM 沒有 Items → 替身沒有種 Items（照 912）。
#   (2) iOCRPort 讀 Gerneral.ini（CheckAndReadIniData，缺鍵補寫 24），畫面 IPPort2 顯示，但本存檔鈕不寫它；golden 全樹 grep "OCR Port"／
#       iOCRPort（20260926）只有這一處讀、:1509 ClientSocket2->Port 用、:1958／:1967 印字 ⇒ 畫面上改 IPPort2 不會存（只有缺鍵時補寫預設 24）。
#   (3) 存檔寫的是元件字串原文（edOCRSkip->Text 等，沒有數值檢查）；讀回時 ReadIniData(int) 解析，非數字 → 預設值。
#   (4) 每次開 OCR 表單（FormShow → fOCR_ReadFile）都會再送一次 MSG_CMD_Enable/DisableBarCode、Enable/DisablePin1Function 給 GPIB 橋。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'OCR.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfOCR'

METHODS = ['fOCR_ReadFile', 'fOCR_DoIniDataToForm', 'FormShow', 'spbSaveClick']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfOCR::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('IniConfig_OCR.py: span %s' % meth)


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
    raise SystemExit('IniConfig_OCR.py: L(%s, %r) not found' % (meth, text))


def _golden_const(name):
    """golden OCR.cpp 檔案層級的 `const int <name>=<v>;`（:63-64，AI(ht9045-v899) 20260522）逐字搬成本 TU 的 static。"""
    hit = [(i, l) for i, l in enumerate(_cpp) if re.match(r'^const\s+int\s+' + name + r'\s*=\s*\d+\s*;', l)]
    if len(hit) != 1:
        raise SystemExit('IniConfig_OCR.py: golden const %s: %d hits' % (name, len(hit)))
    i, l = hit[0]
    return 'static ' + l.strip() + '   // golden OCR.cpp:%d（逐字；forms/fOCR.cpp 另有一份給 IsOCRCommandTrigger，const 是內部連結，不衝突）' % (i + 1)


# ---- FormShow（golden :211）
_FS_SIM = L('FormShow', 'CreateOCRSimulationControls();')
_FS_IMG1 = L('FormShow', 'ShowOCRImg();')
_FS_IMG2 = L('FormShow', 'ShowMatchImg();')
_FS_BEVEL = L('FormShow', 'btSend->BevelInner=bvRaised;')

# ---- spbSaveClick（golden :2151）
_SV_CLOSE = L('spbSaveClick', 'Close();')
_SV_WR1 = L('spbSaveClick', 'WriteIniData(szDir, "OCR SETTING", "iOCRSkip",')

REPLACE = [
    # ---- FormShow
    ('FormShow', _FS_SIM, _FS_SIM,
     'CreateOCRSimulationControls()（AI(ht9045-v899) 20260505）：執行期 new 一個「OCR Simulation」分頁與十幾個元件（TTabSheet／TButton／'
     'TTimer…）＋ ckWordCount 改掛 tsOCR_Cognex_Setting —— 純畫面＋OCR 模擬流程（Jimmy），沒有讀寫檔',
     ';'),
    ('FormShow', _FS_IMG1, _FS_IMG1,
     'ShowOCRImg()（:885）：imgOCR 載入 _ImgPath+"OCR.bmp"（TImage，純畫面）', ';'),
    ('FormShow', _FS_IMG2, _FS_IMG2,
     'ShowMatchImg()（:900）：imgMatch 載入 _ImgPath+"Match.bmp"（TImage，純畫面）', ';'),
    ('FormShow', _FS_BEVEL, _FS_BEVEL,
     'btSend（TPanel）的 BevelInner 外觀：純畫面，vclcompat TPanel 沒有 BevelInner', ';'),
    # ---- spbSaveClick
    ('spbSaveClick', _SV_CLOSE, _SV_CLOSE,
     'Close()：golden A02（Operator 權限）時關表單（下一行 return）；網頁端記 closed，沒寫檔',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', _SV_WR1, _SV_WR1,
     '存檔標記：golden 第一個 WriteIniData 就落地 —— 前面只有 A02 會 return（沒寫檔）',
     'filerw::ELMark("OC_WriteIniData"); WriteIniData(szDir, "OCR SETTING", "iOCRSkip",        '
     'EL<TEdit>("TfOCR", "edOCRSkip")->Text);'),
]

STRUCT = {
    'struct': 'IniConfig_OCR',
    'prefix': 'OC',
    'class': _F,
    'cpp': 'OCR.cpp',
    'h': 'OCR.h',
    'files': ['<DataPath>\\<配方>\\AOI.Data [OCR SETTING] 19 鍵（讀 fOCR_ReadFile：ReadIniData；寫 spbSaveClick：WriteIniData）',
              'system\\Gerneral.ini [OCR SETTING] "OCR Port"（fOCR_ReadFile：CheckAndReadIniData，缺鍵補寫 24；存檔不寫）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'FormShow': '', 'spbSaveClick': ''},
    'members': [
        'bool bShow=false;   // golden OCR.h:292（建構子 :208 設 false、FormShow :213 設 true、FormClose :228 設 false）；golden 全樹沒有表單外的讀者',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'cprod.h', 'Config.h', 'common.h', 'MachineType.h', 'CosFunction.h', 'MessageDef.h', 'forms/fMain.h'],
    # cmydef.h：INSTALL_OCR／AccessLevel／CUSTOMER_CODE；cprod.h：TestIF_File（bOcrFunction／b2DUsePinInspection）；Config.h：IniConfig；
    # common.h：DataPath／GetLastOpenFN／asGeneralPath／ReadIniData／CheckAndReadIniData／WriteIniData；MachineType.h：eocrUninstal、CC_KYEC_LEE；
    # CosFunction.h：CosFunction.bTrayOCR；MessageDef.h：MSG_CMD_Enable/DisableBarCode、MSG_CMD_Enable/DisablePin1Function；
    # forms/fMain.h：fMain->SendMSG_CMD（offline no-op，forms/fMain.cpp:446）／BackupSetupFile（offline no-op，forms/fMain.h:270）
    'decls': [_golden_const('OCR_TRIGGER_MODE_SWITCH'), _golden_const('OCR_TRIGGER_MODE_COMMAND')],
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
_E030Q78_G = open(os.path.join(r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy', 'OCR.cpp'), 'rb').read() \
    .decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')


def _e030q78_expect(gl, text):
    """golden V912 OCR.cpp 第 gl 行（去掉 // 註解與頭尾空白）要等於 text，否則中止。"""
    s = _E030Q78_G[gl - 1].split('//')[0].strip()
    if s != text:
        raise SystemExit('IniConfig_OCR.py (E030-Q78): golden V912 OCR.cpp:%d is %r, expected %r' % (gl, s, text))
    return gl


_e030q78_expect(2153, 'if(IniConfig.bA02DisableSaveParsWhenSwitchToOp==true &&')
_e030q78_expect(2157, 'Close();')
_E030Q78_WHY = ('AI(W906-E030-Q78) 原文照留（取代碼＝golden 原文 return;，產生的程式不變，只加這段註解）。'
                '906 OCR.cpp:2153-2158：A02 分支只有 Close()（:2157），後面沒有 return，存檔照跑；主選單 main.cpp:28836 用 Show 開（非對話框），Close() 當場跑 FormClose（:225-229：StopOCRSimulation()、bShow=false，不重讀畫面）⇒ 操作員改的值照樣寫進檔。'
                'V912 OCR.cpp:2157-2158：Close() 後多一句 `return;` ⇒ 頁面關掉、不寫檔（A02 分支本身兩版都有，RogerYang 20260305 [A01_2]）。'
                'Kept: #20 exception (Steven 1003 Q78, keep V912)')
STRUCT.setdefault('replace', []).append(
    ('spbSaveClick', _e030q78_expect(2158, 'return;'), 2158, _E030Q78_WHY, 'return;'))
