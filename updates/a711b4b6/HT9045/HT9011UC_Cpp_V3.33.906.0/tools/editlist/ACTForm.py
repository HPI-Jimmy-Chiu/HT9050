# -*- coding: utf-8 -*-
# tools/editlist/ACTForm.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only ACTForm
#
# //AI(W906-FRW-S108) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S108）。golden V912 TACTForm（AutoTemperature.cpp，1530 行，cp950；
#   Steven 20210510 自動 K 溫，量測器 Agilent34970A／DeltaDTB4824）的「讀寫檔那一半」：D:\HT9045\System\AutoTemperature.ini（FilePath）。
#   表單不在移植樹（只有 docs/web-fw-legacy 的 layout.json）；沒有頁面（golden 主畫面 sbAutoTemp 鈕，CosFunction.bAutoKTemp 才顯示，
#   main.cpp:24389／:29883-29890 ACTForm->Show()）→ 比照 Rotate：產生 gen.inc＋FileRW/ACTForm.cpp 的開機／頁面入口，不登記 PageDesc。
#
# 為什麼走 C 路而不是手寫讀寫函式：no-page 的表單讀寫一律照 golden 表單轉（Rotate／AutoAlignment／FixAICCD／Magazine… 同一個做法），
#   golden 讀寫段（LoadACTData／SaveACTData／LoadCommData／SaveCommData／btnUpdateClick）都是讀寫元件＋TIniFile，產生器可以原樣轉；
#   之後要做頁面時 PageDesc 與 kACT_SaveReads（必送）已經在，不用重翻。
#
# 轉的 golden 方法：
#   建構子（:289）         ＝ CreateForm（HT9045.cpp:221）。⚠ golden :297 LoadACTData(ACTData) 在 :398 FilePath="D:\\HT9045\\System\\AutoTemperature.ini"
#                            **之前** —— 那一刻 FilePath 是空字串，TIniFile("") 讀不到任何鍵，ACTData 全部是預設值（iThermoCtrlType=1＝DeltaDTB4824…）。
#                            照 golden 順序留著（檔頭「看起來像錯」一條）。其餘：量測點面板 myATPal（TMyATPanel，GUI）不建；iGroupSet／iTotalGroupCount 照算。
#   FormShow（:419）       ＝ 開頁：LoadACTData（真的讀檔）→ 11 個設定元件；LoadCommData（同檔 [COMPort]）→ 5 個 COM 元件；溫度基準顯示。
#   btnUpdateClick（:692） ＝ Update 鈕：元件 → ACTData → SaveACTData（[ACT] 10 鍵＋[Display] Colums）；元件 → ACTCom → SaveCommData（[COMPort] 5 鍵）。
#   LoadACTData（:860）／SaveACTData（:834）／LoadCommData（:1287）／SaveCommData（:1275）＝ 讀寫器。
#   palSaveLogClick（:1355）＝ 存 MemoOffset（K 溫過程的偏移紀錄）到 D:\HT9045_Log\AutoTempCalibration\<時間>.txt。
#
# 不轉（通訊／機台動作，交 Jimmy；見 FileRW/ACTForm.cpp 檔頭）：FormShow 的 OpenCommPort()（:476，開 ACTCom）、TimerACT->Enabled=true（:531，
#   TimerACTTimer :887 自動 K 溫：讀溫度計、算偏移、改 Temperature.fTempOffSet）、ChangeStartState（:794，START 鈕外觀＋tTempScanTimer）、
#   ShowDefaultPos／UpdateTemperatureData（量測點面板 GUI）、FormClose（:534，停計時器＋CloseCommPort）、ACTComReceiveData、SendCommand、
#   btnAutoStartClick、btnDutOnOffClick、DTB4824_ReadPV、TCP 那組（btConnect／btDisConnect／btACTTrigger／ClientSocketACTRead／ACTConnectTimerTimer）。
#
# 元件：ACTCom（golden DFM 的 TComm，AutoTemperature.dfm:994）替身是 TControl、不能當 TComm* 用 → 本 TU 的 ACT_Com（members），
#   用到它的行由下面 _auto() 逐行改寫（同 TestIF_File_Cleaning.py 的 _auto 做法：原文留在 #if 0）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'AutoTemperature.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_h = open(os.path.join(_G, 'AutoTemperature.h'), 'rb').read().decode('cp950', errors='replace')
_F = 'TACTForm'

METHODS = ['TACTForm', 'FormShow', 'btnUpdateClick', 'SaveACTData', 'LoadACTData', 'SaveCommData', 'LoadCommData', 'palSaveLogClick']

# golden header 的元件（同 gen_editlist.py widgets_of：vclcompat 有的型別照用，其餘 TControl）
_KNOWN = {'TEdit', 'TCheckBox', 'TComboBox', 'TRadioGroup', 'TLabel', 'TPanel', 'TGroupBox', 'TTabSheet',
          'TPageControl', 'TSpeedButton', 'TButton', 'TBitBtn', 'TMemo', 'TLabeledEdit', 'TRadioButton',
          'TListBox', 'TDateTimePicker'}
_W = {}
for _m in re.finditer(r'^\s*(T\w+)\s*\*\s*(\w+)\s*;', _h[re.search(r'\bclass\s+TACTForm\b[^;{]*\{', _h).end():], re.M):
    _W[_m.group(2)] = _m.group(1) if _m.group(1) in _KNOWN else 'TControl'
_WRE = re.compile(r'(?<![\w.>:])(' + '|'.join(sorted(map(re.escape, _W), key=len, reverse=True)) + r')\b(?!\s*::)')
_MRE = re.compile(r'(?<![\w.>:])(' + '|'.join(METHODS) + r')\s*\(')


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
    raise SystemExit('ACTForm.py: span %s' % meth)


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
    raise SystemExit('ACTForm.py: L(%s, %r) not found' % (meth, text))


def _code(gl):
    return _cpp[gl - 1].split('//')[0]


def _conv(code):
    """一行 golden 程式碼 → 本 TU 的寫法（同產生器：元件 → EL<>、方法 → ACT_、TComboBox 的 Text／ItemIndex 指派 → ELComboText／ELComboIndex），
    另把 ACTCom → ACT_Com。只給下面 _auto() 用（這些行不含字串裡的元件名）。"""
    t = code.rstrip()
    t = _MRE.sub(lambda m: 'ACT_' + m.group(1) + '(', t)
    t = re.sub(r'(?<![\w.>:])ACTCom\b', 'ACT_Com', t)
    t = _WRE.sub(lambda m: 'EL<%s>("TACTForm", "%s")' % (_W[m.group(1)], m.group(1)), t)
    t = re.sub(r'(EL<TComboBox>\("\w+", "\w+"\))\s*->Text\s*=(?!=)\s*([^;]+);', r'filerw::ELComboText(\1, \2);', t)
    t = re.sub(r'(EL<TComboBox>\("\w+", "\w+"\))\s*->ItemIndex\s*=(?!=)\s*([^;]+);', r'filerw::ELComboIndex(\1, \2);', t)
    lead = t[:len(t) - len(t.lstrip())]
    return lead[4:] + t.strip()          # 產生器在取代碼前面會補 4 格；保留 golden 原本多出來的縮排（if 的本體）


# ---- 手寫的取代（原因寫在每一條）
_loop2 = L('FormShow', 'for(int i=0; i<(int)myATPal.size(); i++)', 2)
_loop2_end = L('FormShow', 'myATPal[i]->gbMeasurePoint->Visible=true;') + 1
assert _cpp[_loop2_end - 1].strip() == '}', 'ACTForm.py: FormShow 第二個 myATPal 迴圈結尾不是 }（golden 改了？）'

MANUAL = [
    # ---- FormShow（golden :419）
    ('FormShow', L('FormShow', 'Top=4;'), L('FormShow', 'Left=4;'), 'Top／Left：視窗位置（HTML 不用）', ';'),
    ('FormShow', L('FormShow', 'ChangeStartState();'), L('FormShow', 'ChangeStartState();'),
     'ChangeStartState（:794）：START 鈕的字與顏色（bAutoStart=false → "START"）、true 時啟動 tTempScanTimer —— 自動 K 溫流程的 GUI，交 Jimmy', ';'),
    ('FormShow', L('FormShow', 'ShowDefaultPos();'), L('FormShow', 'ShowDefaultPos();'),
     'ShowDefaultPos（:1327）：量測點面板 myATPal（TMyATPanel）的排版 —— GUI，移植樹沒有這個類別', ';'),
    ('FormShow', L('FormShow', 'OpenCommPort();'), L('FormShow', 'OpenCommPort();'),
     'OpenCommPort（:616）：開 ACTCom（GetCOMPortStatus＋StartComm）—— 通訊那一半，交 Jimmy',
     'filerw::ELTodo("golden AutoTemperature.cpp:476 OpenCommPort(): opens the ACT thermometer COM port (ACTCom->StartComm) -- communication half not ported (Jimmy)");'),
    ('FormShow', _loop2, _loop2_end,
     '量測點面板 myATPal 的原始／目前偏移圖（ImageShowOriginalOffset／ImageShowCurrentOffset，讀 Temperature.fTempOffSet）—— GUI，移植樹沒有 TMyATPanel'
     '（迴圈拿掉後 :425 的 iGroup 只剩宣告 → (void)，同 forms/fConfiguration.h 的 (void)dCount 慣例）', '(void)iGroup;'),
    ('FormShow', L('FormShow', 'UpdateTemperatureData();'), L('FormShow', 'UpdateTemperatureData();'),
     'UpdateTemperatureData（:809）：量測點面板的溫度顯示 —— GUI', ';'),
    ('FormShow', L('FormShow', 'TimerACT->Enabled=true;'), L('FormShow', 'TimerACT->Enabled=true;'),
     'TimerACT：自動 K 溫的量測計時器（TimerACTTimer :887：讀溫度計、算偏移、寫 Temperature.fTempOffSet）—— 機台動作／通訊，交 Jimmy',
     'filerw::ELTodo("golden AutoTemperature.cpp:531 TimerACT->Enabled=true: TimerACTTimer (auto temperature calibration run) not ported (Jimmy)");'),
]


def _covered(meth, gl):
    return any(m == meth and a <= gl <= b for (m, a, b, *_r) in MANUAL)


def _auto():
    """建構子的 myATPal（GUI 面板）逐行 → 空敘述；FormShow／btnUpdateClick 用到 ACTCom 的行、以及 TComboBox 指派前有空白的行
    （產生器的 ELComboText 規則只認 `)->Text=`）→ _conv 改寫。"""
    out = []
    for m in ('TACTForm', 'FormShow'):     # FormShow :497-498 DTB4824 時把面板的 cbChannel 藏起來（第二個迴圈整段在 MANUAL）
        a, b = SPANS[m]
        for gl in range(a + 1, b):
            if 'myATPal' in _code(gl) and not _covered(m, gl):
                out.append((m, gl, gl, '量測點面板 myATPal（TMyATPanel，GUI 元件；網頁 hwidgets.js 自己畫）—— 移植樹沒有這個類別', ';'))
    for m in ('FormShow', 'btnUpdateClick'):
        a, b = SPANS[m]
        for gl in range(a + 1, b):
            c = _code(gl)
            if _covered(m, gl):
                continue
            if re.search(r'(?<![\w.>:])ACTCom\b', c) or \
               re.search(r'(?<![\w.>:])(cbDevice|cbBaudRate|cbByteSize|cbStopBit|cbParity|cbbThermoCtrlType)\s+->\s*(Text|ItemIndex)\s*=(?!=)', c):
                out.append((m, gl, gl, 'ACTCom（golden DFM 的 TComm）→ 本 TU 的 ACT_Com；TComboBox 指派 → ELComboText／ELComboIndex（見設定檔檔頭）', _conv(c)))
    return out


STRUCT = {
    'struct': 'ACTForm',
    'prefix': 'ACT',
    'class': _F,
    'cpp': 'AutoTemperature.cpp',
    'h': 'AutoTemperature.h',
    'files': ['D:\\HT9045\\System\\AutoTemperature.ini [ACT] iThermoCtrlType／CheckIntervalTime／CalibrationRange／dCalibrationRange[0..2]／ReadScanTime／'
              'OffsetLimit／SingleOffsetLimit／OffsetMethod、[Display] Colums、[COMPort] CommName／BaudRate／ByteSize／StopBits／Parity'
              '（讀：FormShow；寫：btnUpdateClick）',
              'D:\\HT9045_Log\\AutoTempCalibration\\<YYYY-MM-DD hh_mm_ss>.txt（寫：palSaveLogClick）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['btnUpdateClick', 'SaveACTData', 'SaveCommData'],
    'params': {'TACTForm': '', 'FormShow': '', 'btnUpdateClick': '', 'palSaveLogClick': ''},
    'rettype': {'SaveCommData': 'bool', 'LoadCommData': 'bool'},
    'members': [
        'bool bCommConnect=false;       // golden AutoTemperature.h:191（建構子 :291 設 false；只有 OpenCommPort／CloseCommPort 改它）',
        'bool bAutoStart=false;         // golden AutoTemperature.h:192',
        'ACT_DATA ACTData;              // golden AutoTemperature.h:194（golden 表單外沒有讀者：全樹只有 main.cpp:29889 ACTForm->Show()）',
        'TCheckBox*  _cbBase[3];        // golden AutoTemperature.h:196',
        'TPanel*     _pnlBaseTemp[3];   // golden AutoTemperature.h:197',
        'AnsiString  FilePath;          // golden AutoTemperature.h:198（建構子 :398 才設；:297 LoadACTData 時還是空字串，見設定檔檔頭）',
        'AnsiString CommName;           // golden AutoTemperature.h:178（private）',
        'int iTotalGroupCount=0;        // golden AutoTemperature.h:199（建構子依 iSocketBaseTempCount；只有 TimerACTTimer 用）',
        'int iGroupSet[ATC_MaxGroup];   // golden AutoTemperature.h:200（同上）',
        'bool bBaseFinish[5];           // golden AutoTemperature.h:206',
        'int iReceiveData=0;            // golden AutoTemperature.h:211',
        'int iAddr=0;                   // golden AutoTemperature.h:212',
        'TComm* ACT_Com=new TComm(nullptr);   // golden AutoTemperature.h:69 ACTCom（DFM 的 TComm，AutoTemperature.dfm:994；vclcompat TComm 建構不碰硬體，沒有人 StartComm）',
    ],
    'replace': MANUAL + _auto(),
    'blocks': [],
    'includes': ['cmydef.h', 'common.h', 'cprod.h', 'MachineType.h', 'vclcompat/IniFiles.h', 'vclcompat/SysUtils.h'],
    # cmydef.h：iSocketBaseTempCount、SystemYear…（palSaveLogClick）；common.h：MyForceDirectories／OffsetPath；cprod.h：Temperature（FormShow 基準溫度）；
    # MachineType.h：tcHotPlate1…／eDut2ea／eDut4ea、CheckRange；vclcompat/IniFiles.h：TIniFile；vclcompat/SysUtils.h：IntToStr／FloatToStr
    'decls': [
        '// golden AutoTemperature.h:18-32（本表單私用的型別；移植樹沒有 —— 20260926 grep ACT_DATA／Type_Agilent34970A／ATC_MaxGroup 0 筆）',
        '#define ATC_MaxGroup 20',
        'typedef struct {',
        '    int iThermoCtrlType;',
        '    int iCheckIntervalTime;',
        '    int iCalibrationRange;',
        '    double dCalibrationRange[3];',
        '    int iReadScanTime;',
        '    int iOffsetLimit;',
        '    int iSingleOffset;',
        '    int iDisplayColumn;',
        '    int iOffsetMethod;',
        '} ACT_DATA;',
        'enum eACTThermoType{Type_Agilent34970A=0, Type_DeltaDTB4824=1};  //Steven 20210510 : 自動K溫使用DTB4824',
        '// golden SaveACTData :849-851 IniFile1->WriteFloat：vclcompat::TIniFile 沒有 WriteFloat；BCB6 TCustomIniFile.WriteFloat 的本體就是',
        '// WriteString(Section, Name, FloatToStr(Value))（同 ProductionInfo/uPAT_Function.cpp:621 的 G17 巨集）。放在所有 #include 之後。',
        '#define WriteFloat(_sec_,_key_,_val_) WriteString(_sec_,_key_,FloatToStr(_val_))',
    ],
    'overrides': [],
}
