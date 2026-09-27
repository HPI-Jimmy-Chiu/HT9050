# -*- coding: utf-8 -*-
# tools/editlist/Winway.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only Winway
#
# //AI(W906-FRW-S109) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S109）。golden V912 TfWinway（ATC\WinWaySetting.cpp，238 行，cp950）
#   的「讀寫檔那一半」：D:\HT9045\config\ATCWinWay.ini（ATCWinWayPath，golden WinWaySetting.h:15；4 段 [COMPort1..4]）。
#   沒有頁面（golden 主畫面 sbATC 鈕，ATC_SYSTEM==eWinWay 才開 fWinway->ShowModal()，main.cpp:29607-29609）→ 比照 Rotate：產生
#   gen.inc＋FileRW/Winway.cpp 的開機／頁面入口，不登記 PageDesc。
#
# 結構名 Winway：表單沒有 cprod.h 的主結構，讀寫的是 4 個 ATC_WinWay 物件（arrATC_Site[]）的 COM 設定與設定溫度，同 GroundMan 用表單名。
#
# 轉的 golden 方法：
#   建構子（:12）             ＝ CreateForm（HT9045.cpp:272）：edtSetTemp="0"、iWinwayATCIndex=0、bShow=false、
#                                new 4 個 ATC_WinWay(WinWayATCComm1..4) → 每站 LoadCommData 再 SaveCommData（**每次開機都讀、都寫**，
#                                不看 ATC_SYSTEM；檔不在會照預設值建出來）。
#   LoadCommData（:116）      ＝ [COMPort<n>] CommName（預設 COM<n+14>）／BaudRate／ByteSize／StopBits／Parity → WinwayCOM（TComm 欄位）＋
#                                sWinwayCommName；SetTemperature → ATC_WinWay::SetST()。⚠ SetST 只在 bCommConnect 時才送 Modbus 寫溫度指令
#                                （ATC/ATC_WinWay.cpp:192-195：先存 fSetTemperature，`if(!bCommConnect) return;`）；開機時 4 站都還沒開 COM
#                                （golden 只有 uLotInfo.cpp:7161 在 ATC_SYSTEM==eWinWay＆Temperature.bATCActiveCooling 時 OpenCommPort），所以這裡只存值。
#   SaveCommData（:135）      ＝ 反向，6 鍵（SetTemperature 用 WriteFloat＝WriteString(FloatToStr)，BCB6 IniFiles.pas 的本體）。
#   FormShow（:34）／ShowCommData（:43）／cbbWinwayATCIndexChange（:150）＝ 開頁／切換站別：arrATC_Site[i] → 元件。
#   btnUpdateClick（:78）     ＝ Update 鈕：元件 → arrATC_Site[i]／WinwayCOM → SaveCommData。
#
# 不轉（通訊，交 Jimmy；見 FileRW/Winway.cpp 檔頭）：btnUpdateClick 結尾 CloseCommPort()／OpenCommPort()（:111-112，用新設定重開 COM）、
#   ShowCommData 的 lblShowPT=GetPT()（:74，連線時送 Modbus 01 03 讀溫度）、OpenCommPort（:156／:164）、CloseCommPort（:176）、
#   SetST（:170）、btnSendTempClick（:184）／SetTemprature（:190）／SetTempratureAll（:195）（送設定溫度）、btnGetPVClick（:203）、
#   WinWayATCComm1ReceiveData（:208，收 PV）、edtSetTempClick（:235，小鍵盤 → 頁面）。
#
# 狀態共用：arrATC_Site[]、iWinwayATCIndex 用 #define 接到門面 forms/fWinway.h 的 fWinway（全樹一份；bthermo.cpp:4609 起、
#   forms/fLotInfo.cpp:4274 等 golden 讀者目前都在 #if 0，之後解閘時讀到的就是這裡建的 4 個物件）。edtSetTemp／cbbWinwayATCIndex 門面也有
#   （`new TEdit()` 寫法，產生器 'adopt' 只認 `new vclcompat::T()`）→ FileRW/Winway.cpp 開機一開始 ELKeep 收養。
#   4 個 TComm（golden DFM 元件 WinWayATCComm1..4）＝本 TU 的 WW_Comm[4]（門面沒有；ATC_WinWay::WinwayCOM 指向它們）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'ATC', 'WinWaySetting.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfWinway'

METHODS = ['TfWinway', 'FormShow', 'ShowCommData', 'btnUpdateClick', 'LoadCommData', 'SaveCommData', 'cbbWinwayATCIndexChange']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\b' + _F + r'::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('Winway.py: span %s' % meth)


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
    raise SystemExit('Winway.py: L(%s, %r) not found' % (meth, text))


REPLACE = [
    # ---- 建構子（golden :12，CreateForm HT9045.cpp:272）
    ('TfWinway', L('TfWinway', 'Width =330;'), L('TfWinway', 'Height=518;'), 'Width／Height：視窗大小（HTML 不用；門面 forms/fWinway.cpp:74-75 已存值）', ';'),
    ('TfWinway', L('TfWinway', 'arrATC_Site[0]=new ATC_WinWay(WinWayATCComm1);'), L('TfWinway', 'arrATC_Site[3]=new ATC_WinWay(WinWayATCComm4);'),
     'WinWayATCComm1..4：golden DFM 的 TComm 元件；替身是 TControl、不能當 TComm* 傳 → 本 TU 的 WW_Comm[0..3]（其餘照 golden）',
     'arrATC_Site[0]=new ATC_WinWay(WW_Comm[0]); arrATC_Site[1]=new ATC_WinWay(WW_Comm[1]); '
     'arrATC_Site[2]=new ATC_WinWay(WW_Comm[2]); arrATC_Site[3]=new ATC_WinWay(WW_Comm[3]);'),
    # ---- FormShow（golden :34）
    ('FormShow', L('FormShow', 'Left=(1024-Width)/2;'), L('FormShow', 'Top =(768-Height)/2;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
    # ---- ShowCommData（golden :43）
    ('ShowCommData', L('ShowCommData', 'lblShowPT->Caption=FloatToStr(tempATC_WinWay->GetPT());'),
     L('ShowCommData', 'lblShowPT->Caption=FloatToStr(tempATC_WinWay->GetPT());'),
     'GetPT()：連線時送 Modbus 01 03 讀溫度（ATC/ATC_WinWay.cpp GetPT），沒連線回 0 —— 通訊那一半，交 Jimmy',
     'filerw::ELTodo("golden ATC/WinWaySetting.cpp:74 lblShowPT=GetPT(): ATC_WinWay::GetPT sends Modbus 01 03 when connected -- communication half not ported (Jimmy)");'),
    # ---- btnUpdateClick（golden :78）
    ('btnUpdateClick', L('btnUpdateClick', 'tempATC_WinWay->CloseCommPort();'), L('btnUpdateClick', 'tempATC_WinWay->OpenCommPort();'),
     'CloseCommPort／OpenCommPort：用新設定重開這一站的 COM（ATC_WinWay::OpenCommPort → CreateFile＋StartComm）—— 通訊那一半，交 Jimmy；'
     '檔已在上一行 SaveCommData 寫好',
     'filerw::ELTodo("golden ATC/WinWaySetting.cpp:111-112 CloseCommPort()/OpenCommPort(): reopen the WinWay COM port with the new settings -- communication half not ported (Jimmy)");'),
    ('btnUpdateClick', L('btnUpdateClick', 'ShowMessage('), L('btnUpdateClick', 'ShowMessage('),
     'ShowMessage（VCL 訊息框）→ filerw::ELMessage（進 ack.session.messages）', 'filerw::ELMessage("【Update】 Success");'),
]

STRUCT = {
    'struct': 'Winway',
    'prefix': 'WW',
    'class': _F,
    'cpp': 'ATC/WinWaySetting.cpp',   # 斜線：產生器會把它寫進註解與字串常值（同 TestIF_File_Cleaning.py 的理由）
    'h': 'ATC/WinWaySetting.h',
    'files': ['D:\\HT9045\\config\\ATCWinWay.ini [COMPort1..4] CommName／BaudRate／ByteSize／StopBits／Parity／SetTemperature'
              '（開機：建構子每站 LoadCommData＋SaveCommData；開頁讀記憶體；Update 鈕寫該站）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['btnUpdateClick', 'SaveCommData'],
    'params': {'TfWinway': '', 'FormShow': '', 'btnUpdateClick': '', 'cbbWinwayATCIndexChange': ''},
    'rettype': {'LoadCommData': 'bool', 'SaveCommData': 'bool'},
    'members': [
        '#define arrATC_Site (fWinway->arrATC_Site)             // golden WinWaySetting.h:52 —— 門面 forms/fWinway.h 那一份（bthermo.cpp／fLotInfo.cpp 的 golden 讀者用同一個）',
        '#define iWinwayATCIndex (fWinway->iWinwayATCIndex)     // golden WinWaySetting.h:51 —— 門面的那一個',
        'bool bShow=false;                                      // golden WinWaySetting.h:53 —— 本 TU 自己的（網頁沒有關頁事件；同 GroundMan 的 fShow）',
        'TComm* WW_Comm[SiteNum]={new TComm(nullptr), new TComm(nullptr), new TComm(nullptr), new TComm(nullptr)};   '
        '// golden WinWaySetting.h:44-47 WinWayATCComm1..4（DFM 的 TComm 元件；vclcompat TComm 建構不碰硬體）',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['forms/fWinway.h', 'ATC/ATC_WinWay.h', 'vclcompat/IniFiles.h', 'vclcompat/SysUtils.h', 'cmydef.h'],
    # forms/fWinway.h：門面 fWinway（arrATC_Site／iWinwayATCIndex／edtSetTemp／cbbWinwayATCIndex）、SiteNum；ATC/ATC_WinWay.h：ATC_WinWay 本體（ht9045_comms）；
    # vclcompat/IniFiles.h：TIniFile；vclcompat/SysUtils.h：FloatToStr
    'decls': [
        r'#define ATCWinWayPath    "D:\\HT9045\\config\\ATCWinWay.ini"   // golden ATC/WinWaySetting.h:15（寫死字面值，--dry 蓋不到）',
        '// golden SaveCommData :145 IniFile->WriteFloat：vclcompat::TIniFile 沒有 WriteFloat；BCB6 TCustomIniFile.WriteFloat 的本體就是',
        '// WriteString(Section, Name, FloatToStr(Value))（同 ProductionInfo/uPAT_Function.cpp:621 的 G17 巨集）。放在所有 #include 之後。',
        '#define WriteFloat(_sec_,_key_,_val_) WriteString(_sec_,_key_,FloatToStr(_val_))',
    ],
    'overrides': [],
}
