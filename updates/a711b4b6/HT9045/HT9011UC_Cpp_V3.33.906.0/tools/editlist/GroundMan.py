# -*- coding: utf-8 -*-
# tools/editlist/GroundMan.py -- gen_editlist.py 的結構設定（C 形狀：具名替身，一個結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only GroundMan
#
# //AI(W906-CRT-GroundMan) 20260926: 新檔（Steven 團隊）。golden V912 TfGroundMan（GroundMan\GroundMan.cpp，1648 行，cp950）——
#   Status.GroundMan.html 的「讀寫段」。頁面補件 web/page/ht9045_groundman_c.js。一律照 V912（RULINGS 第 37 條）；
#   客戶專屬條件（第 25 條）：編得過就照 golden 留著（CC_KYEC_LEE、CC_SIGURD_PeiXing 都編得過，全部照留）。
#
# 結構名 GroundMan：這張表單沒有 cprod.h 的主結構，它讀寫的是 D:\HT9045\system\GroundMan.ini（系統檔，不跟配方），
#   同 ShuttleMove 的慣例用表單名（去掉 Tf）當 tag。
#
# 選配開關：Gerneral.ini [Ground_Man] USE_GROUND_MAN（golden database.cpp:1420，0／1；HandlerSys rgGroundMan 設定）——
#   golden 主畫面 GroundMan 鈕 spbGroundMan 只在 USE_GROUND_MAN 時顯示（main.cpp:24327），按下 fGroundMan->Show()（:34642）。
#   golden 的表單本身不看這個開關：建構子一律 ReadGroundOffset、存檔鈕一律寫檔；只有 RS232 監測（Timer1Timer :568）看它。
#   另外 HSys.iGroundManScanPoint（[Ground_Man] Ground_Man_ScanPoint，0／1／2＝8／22／28 點）決定用幾塊板（iUseGndBoard 1／3／4）。
#
# 轉的 golden 方法（讀寫段）：
#   建構子（:24）         ＝ CreateForm（HT9045.cpp:262）：成員初值、iUseGndBoard、bOpenClose、asShowName → :136 ReadGroundOffset()
#   FormShow（:139）      ＝ 開頁：依 ScanPoint 顯示板子、labCH_* 名稱、KYEC_LEE 停用兩個輸入框、PeiXing 維修鈕
#   spbSaveClick（:1418） ＝ 存檔鈕：A02 → 5 秒保護 → KYEC_LEE 固定值 → WriteIniData ×2（[System] Alarm_*）→ ReadGroundOffset
#   ReadGroundOffset（:1573）＝ 讀 GroundMan.ini：[System] UseOffset／UseResetByStart／Alarm_Continuous_Time／Alarm_Occurrences、
#                          [Board_<n>_Offset] <asShowName> → dOffset（CheckAndReadIniData：缺鍵照 golden 補寫預設值）
# golden 呼叫 ReadGroundOffset 的三個地方：建構子 :136（開機，本檔轉）、spbSaveClick :1468（本檔轉）、Init_GM_RS232 :266
#   （RS232 開 COM，屬 S48，不轉；golden 只在 TfMain::FormShow main.cpp:11533-11549 `#ifndef SOFT_SIMULTE if(USE_GROUND_MAN>0)` 呼叫，
#   讀的是同一個檔，開機值已由建構子那一次讀好）。換配方不讀（GroundMan.ini 不跟配方；golden DoReadLastData／ChangeSetUpFile 都沒有 fGroundMan）。
#
# 不轉（機台動作／RS232，S48 待辦；見 FileRW/GroundMan.cpp 檔頭）：spbStartComClick :247、spbStopComClick :253、Init_GM_RS232 :259、
#   comGMReceiveData :303、SetGroundMaster :457、Timer1Timer :551、DoGroundMasterMonitor :577（:1295 StopAllMotor）、
#   FormClose :228（ReStart → RS232 重開）、sbtExitClick :1472（Close → FormClose）、ShowGroundManLog :1480（寫 D:\HT9045_Log\GroundManLog）、
#   ReStart :1636、calc_crc :525（純計算，只給 RS232 用）。edOccurrencesMouseDown／edContinuous_TimeMouseDown（小鍵盤）→ 頁面 kb 表。
import os
import re

_G = r'D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy'
_cpp = open(os.path.join(_G, 'GroundMan', 'GroundMan.cpp'), 'rb').read().decode('cp950', errors='replace').replace('\r\n', '\n').split('\n')
_F = 'TfGroundMan'

METHODS = ['TfGroundMan', 'FormShow', 'spbSaveClick', 'ReadGroundOffset']


def _span(meth):
    for i, l in enumerate(_cpp):
        if re.match(r'^[^\n/]*\bTfGroundMan::' + meth + r'\s*\(', l):
            d, seen = 0, False
            for j in range(i, len(_cpp)):
                s = _cpp[j].split('//')[0]
                d += s.count('{') - s.count('}')
                seen = seen or '{' in s
                if seen and d == 0:
                    return i + 1, j + 1
    raise SystemExit('GroundMan.py: span %s' % meth)


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
    raise SystemExit('GroundMan.py: L(%s, %r) not found' % (meth, text))


_WR1 = L('spbSaveClick', 'WriteIniData(sFileName, "System",   "Alarm_Continuous_Time"')

REPLACE = [
    # ---- 建構子（golden :24，CreateForm HT9045.cpp:262）
    ('TfGroundMan', L('TfGroundMan', 'ZeroMemory(asRecordValueData, sizeof(asRecordValueData));'),
     L('TfGroundMan', 'ZeroMemory(asRecordValueData, sizeof(asRecordValueData));'),
     'ZeroMemory 對 AnsiString 陣列：標準 C++ 是未定義行為（會把字串物件內部清成 0）；本 TU 的 static 陣列本來就是空字串'
     '（同 forms/fGroundMan.h DEVIATION D-5）。bool 陣列那三行照 golden', ';'),
    ('TfGroundMan', L('TfGroundMan', 'ZeroMemory(asShowName, sizeof(asShowName));'),
     L('TfGroundMan', 'ZeroMemory(asShowName, sizeof(asShowName));'),
     '同上（asShowName 在下面 :86-124 逐格照 golden 設定）', ';'),
    ('TfGroundMan', L('TfGroundMan', 'Timer1->Interval=30;'), L('TfGroundMan', 'Timer1->Interval=30;'),
     'Timer1：RS232 監測的計時器（Timer1Timer :551，S48）；替身沒有 Interval。移植樹門面 forms/fGroundMan.cpp 建構子已設 '
     'fGroundMan->Timer1->Interval=30', ';'),
    # ---- FormShow（golden :139）
    ('FormShow', L('FormShow', 'Left=100;'), L('FormShow', 'Top=100;'), 'Left／Top：視窗位置（HTML 不用）', ';'),
    # ---- spbSaveClick（golden :1418）
    ('spbSaveClick', L('spbSaveClick', 'Close();'), L('spbSaveClick', 'Close();'),
     'Close()：golden A02（Operator 權限）時關表單 → 伺服器端記 closed，頁面要重新開頁才能再存；沒寫檔。'
     'golden Close() 還會觸發 FormClose :228 → ReStart()（RS232 重開，S48），網頁不做',
     'filerw::ELMark("closed");'),
    ('spbSaveClick', _WR1, _WR1,
     '存檔標記：golden 第一個 WriteIniData（:1466）就落地 —— 之前的 A02（:1420）與「ContinuousTime/Occurrences 必須大於 5」'
     '（:1444）都在它前面 return，沒寫檔（savedMark="GM_WriteIniData"）',
     'filerw::ELMark("GM_WriteIniData"); WriteIniData(sFileName, "System",   "Alarm_Continuous_Time",  iAlarm_Continuous_Time);     //Alarm 允許時間'),
]

STRUCT = {
    'struct': 'GroundMan',
    'prefix': 'GM',
    'class': _F,
    'cpp': 'GroundMan\\GroundMan.cpp',
    'h': 'GroundMan\\GroundMan.h',
    'files': ['D:\\HT9045\\system\\GroundMan.ini [System] Alarm_Continuous_Time／Alarm_Occurrences（寫，spbSaveClick）',
              '同檔 [System] UseOffset／UseResetByStart、[Board_<n>_Offset]（讀，ReadGroundOffset；CheckAndReadIniData 缺鍵補寫）'],
    'lists': [],
    'methods': METHODS,
    'save_methods': ['spbSaveClick'],
    'params': {'TfGroundMan': '', 'FormShow': '', 'spbSaveClick': ''},
    'globals': ['Alarm_Continuous_Time', 'Alarm_Occurrences'],   # golden GroundMan.cpp:21-22（檔案層級全域，只有本檔用）
    'members': [
        'bool fShow=false;                     // golden GroundMan.h:231 —— 本 TU 自己的（同 ShuttleMove：網頁關頁沒有回報，寫門面的 fGroundMan->fShow 會永遠是 true）',
        '#define iGroundMasterTask (fGroundMan->iGroundMasterTask)   // golden GroundMan.h:230：門面 forms/fGroundMan.h 的那一個（RS232 監測狀態；golden Timer1Timer :572 顯示在 labStatus）',
        '#define bRs232Ok (fGroundMan->bRs232Ok)                     // golden GroundMan.h:208：門面的那一個（唯一設 true 的 Init_GM_RS232 屬 S48，未移植 → 一直是 false）',
        'bool bReaderOK[4][6];                 // golden GroundMan.h:201（以下到 asGroundVaule 都是 golden 私有成員；本 TU 只做建構子的初值，',
        'AnsiString sCMD[6];                   // golden GroundMan.h:202  RS232 監測（S48）才讀 —— 移植時與監測放在同一個 TU，見 FileRW/GroundMan.cpp 檔頭）',
        'AnsiString asRecordValueData[4][8];   // golden GroundMan.h:203',
        'double dOffset[4][8];                 // golden GroundMan.h:206（ReadGroundOffset 讀 [Board_<n>_Offset]）',
        'bool bRecordAlarmData[4][8];          // golden GroundMan.h:207',
        'bool bOpenClose[4][8];                // golden GroundMan.h:209',
        'AnsiString asShowName[4][8];          // golden GroundMan.h:210（ini 鍵名＋labCH_* 名稱；建構子 :86-124）',
        'bool bGroundManReset;                 // golden GroundMan.h:213',
        'bool bGroundManResetByStart;          // golden GroundMan.h:214（[System] UseResetByStart）',
        'bool bUseOffset;                      // golden GroundMan.h:215（[System] UseOffset）',
        'int  iUseGndBoard;                    // golden GroundMan.h:217（建構子依 HSys.iGroundManScanPoint：1／3／4）',
        'bool bGroundManAlarm_FirstTime[4][8]; // golden GroundMan.h:220',
        'int  iGroundManAlarm_HappenCount[4][8];   // golden GroundMan.h:222',
        'bool bInternalOhm;                    // golden GroundMan.h:223',
        'bool bVersion;                        // golden GroundMan.h:224',
        'int  iMachineOhmRetry;                // golden GroundMan.h:225',
        'AnsiString asGroundVaule;             // golden GroundMan.h:233（public；golden cObserver.cpp:2036／uLotInfo.cpp:15604 讀，移植樹沒有讀者）',
    ],
    'replace': REPLACE,
    'blocks': [],
    'includes': ['cmydef.h', 'Config.h', 'common.h', 'MachineType.h', 'database.h', 'forms/fGroundMan.h'],
    # cmydef.h：CUSTOMER_CODE／AccessLevel／iDefHonPrecLevel／USE_GROUND_MAN；Config.h：IniConfig（A02）；common.h：CheckAndReadIniData／
    # WriteIniData／MyForceDirectories；MachineType.h：CC_*；database.h：HSys（iGroundManScanPoint）；forms/fGroundMan.h：門面 fGroundMan（#define 的兩個成員）
    'decls': [],
    'overrides': [],
}
