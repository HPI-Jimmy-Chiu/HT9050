# -*- coding: utf-8 -*-
# tools/editlist/Monitor.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only Monitor
#
# //AI(W906-FRW-S110) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S110）。golden V912 TfMonitor（Monitor\MonitorInterface.cpp，436 行，cp950）
#   的「讀寫檔那一半」：D:\HT9045\system\MVData.ini（系統檔，不跟配方）。沒有頁面（golden 主畫面 sbMonitorView 鈕，
#   IniConfig.bC11UseMonitorView 才顯示，main.cpp:24392／:33471）→ 比照 Rotate：產生 gen.inc＋FileRW/Monitor.cpp 的開機／頁面入口，不登記 PageDesc。
#
# 結構名 Monitor：表單沒有 cprod.h 的主結構，讀寫的是表單自己的元件＋sADDRESS／iPORT，同 GroundMan／ShuttleMove 用表單名（去 Tf）。
#
# 轉的 golden 方法：
#   FormShow（:47）           ＝ 開頁：視窗大小位置（HTML 不用）→ bShow=true → LoadTCPIPParament()
#   LoadTCPIPParament（:79）  ＝ 讀 MVData.ini [Setup] IP／Port、[Specific] HD_Space／HD_Space_Low／HD_Space_Low_Prompt／HD_Closed_Wait／
#                                HD_Closed_Wait_Time → sADDRESS／iPORT＋7 個元件（只讀，TIniFile::Read*，不補寫）
#   SaveTCPIPParament（:98）  ＝ 7 個元件 → sADDRESS／iPORT → 寫同一檔 7 鍵（TIniFile::Write*，立即落地）
#   sbMVUpdateClick（:123）   ＝ Update 鈕 → SaveTCPIPParament()
# 建構子（:23）不轉：它的 :26-29、:35-44（bShow、bCommandReady、MVPageControl、sComData、iHDSpace、iStatus、iDisconnectCount、
#   MonitorTimer）移植樹門面 forms/fMonitor.cpp:32 在 static init 已照做；:30-31、:33 是 MVCtrl（MonitorTCPIP，TCP 連線，交 Jimmy）；
#   剩下唯一的讀檔 :32 LoadTCPIPParament() 由 FileRW/Monitor.cpp 的 FileRW_Monitor_Boot() 在 CreateForm(TfMonitor)（HT9045.cpp:247）的時機呼叫。
#
# 不轉（通訊／機台動作，交 Jimmy／Steven02；見 FileRW/Monitor.cpp 檔頭）：LoadTCPIPParament／SaveTCPIPParament 結尾的
#   MVCtrl->InitialSocket(sADDRESS, iPORT)（:94／:119）、sbMVConnectClick :128、sbMVDisconnectClick :133、MonitorTimerTimer :143
#   （HD 容量警報讀 cbWhenHDFullAlarm／edLowHDSpace／edWhenHDFullPrompt、錄影關閉等待讀 edAfterHandler…WaitTime —— 讀的是本檔讀進來的
#   同一批元件，見 adopt）、sbSendCommandClick、OpenMonitorVedio／StopMonitorVedio／GetMonitorVedioState／GetMonitorHDSpec。
#
# 元件共用：門面 forms/fMonitor.h 已有 7 個同名同型別元件（`TEdit *edMVAddress = new TEdit();` 那種寫法，產生器的 adopt 只認
#   `new vclcompat::T()`，所以不用 'adopt' 欄位）→ FileRW/Monitor.cpp 的 MN_AdoptPortWidgets() 在開機一開始用 ELKeep 登記成替身，
#   golden 轉出來的程式與門面（之後翻 MonitorTimerTimer 時）讀同一份值。sADDRESS／iPORT 用 #define 接到門面（fMonitor->sADDRESS）。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'Monitor', 'MonitorInterface.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfMonitor'

METHODS = ['FormShow', 'LoadTCPIPParament', 'SaveTCPIPParament', 'sbMVUpdateClick']


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
    raise SystemExit('Monitor.py: span %s' % meth)


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
    raise SystemExit('Monitor.py: L(%s, %r) not found' % (meth, text))


_SOCK = 'MVCtrl->InitialSocket(sADDRESS, iPORT)：MonitorTCPIP 的連線位址（TCP，golden 建構子 :30 new 的 MVCtrl）—— 通訊那一半，交 Jimmy；' \
        '移植樹門面的 MVCtrl 一直是 NULL（forms/fMonitor.h GATE M-1）。讀進來的 sADDRESS／iPORT 已在門面 fMonitor->sADDRESS／iPORT，接上連線時直接用'

REPLACE = [
    ('FormShow', L('FormShow', 'Width   =610;'), L('FormShow', 'Top     =(768-Height)/2;'), 'Width／Height／Left／Top：視窗大小與位置（HTML 不用）', ';'),
    ('LoadTCPIPParament', L('LoadTCPIPParament', 'MVCtrl->InitialSocket(sADDRESS, iPORT);'),
     L('LoadTCPIPParament', 'MVCtrl->InitialSocket(sADDRESS, iPORT);'), _SOCK, ';'),
    ('SaveTCPIPParament', L('SaveTCPIPParament', 'MVCtrl->InitialSocket(sADDRESS, iPORT);'),
     L('SaveTCPIPParament', 'MVCtrl->InitialSocket(sADDRESS, iPORT);'), _SOCK, ';'),
]

STRUCT = {
    'struct': 'Monitor',
    'prefix': 'MN',
    'class': _F,
    'cpp': 'Monitor/MonitorInterface.cpp',   # 斜線：產生器會把它寫進註解與字串常值（同 TestIF_File_Cleaning.py 的理由）
    'h': 'Monitor/MonitorInterface.h',
    'files': ['D:\\HT9045\\system\\MVData.ini [Setup] IP／Port、[Specific] HD_Space／HD_Space_Low／HD_Space_Low_Prompt／HD_Closed_Wait／HD_Closed_Wait_Time'
              '（讀：LoadTCPIPParament，開機＋開頁；寫：SaveTCPIPParament，Update 鈕）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['sbMVUpdateClick', 'SaveTCPIPParament'],
    'params': {'FormShow': '', 'sbMVUpdateClick': ''},
    'members': [
        'bool bShow=false;                        // golden MonitorInterface.h:82（private）—— 本 TU 自己的：網頁沒有關頁事件（golden FormClose :74 設回 false），'
        '寫門面的 fMonitor->bShow 會讓 golden MonitorTimerTimer :230「bShow==false 才自動重連」永遠不成立（同 GroundMan 的 fShow）',
        '#define sADDRESS (fMonitor->sADDRESS)   // golden MonitorInterface.h:89 —— 門面 forms/fMonitor.h:260 那一份（MVCtrl 連線用，交 Jimmy 接）',
        '#define iPORT (fMonitor->iPORT)         // golden MonitorInterface.h:90 —— 門面 forms/fMonitor.h:261',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['forms/fMonitor.h', 'vclcompat/IniFiles.h', 'cmydef.h'],
    # forms/fMonitor.h：門面 fMonitor（sADDRESS／iPORT、7 個元件）；vclcompat/IniFiles.h：TIniFile（golden inifiles.hpp）
    'decls': [],
    'overrides': [],
}
