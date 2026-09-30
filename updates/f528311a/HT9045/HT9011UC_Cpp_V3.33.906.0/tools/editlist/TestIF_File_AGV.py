# -*- coding: utf-8 -*-
# tools/editlist/TestIF_File_AGV.py -- gen_editlist.py 的結構設定（一個 C 形狀結構一個檔）。
# 欄位說明見 tools/editlist/README.md。改完跑：python tools/gen_editlist.py --only TestIF_File_AGV
#
# AI(W906-B8-AG1) 20260930 [W906] St01：B8 AG-1（D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\B8_RISK_20260930.md「AG-1」；
#   Steven 20260928「任何畫面的事件, 都是我們做」、20260929「請按照bcb的邏輯處理」⇒ 照 golden）。
# golden TfAGV（V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\Automation\AGV.cpp）的設定半邊：
#   D:\HT9045\config\AGV.ini [Configuration] 26 鍵 ⇄ TestIF_File（iE84TimeOut_K12[2][11]、iLoaderUnloaderTrayCount[0..2]、bEnableE84）。
#   結構名加後綴 _AGV：TestIF_File 已有 A 形狀 FileRW/TestIF_File.cpp 與多個 C 形狀 TestIF_File_<表單>（同 TestIF_File_QAMode.py）。
# 沒有 HTEditList：存檔鈕 spbSaveClick（:1184）＝ 26 個 WriteIniData（:1189-1221）→ ReadFile（:1223）；golden 不問 YES/NO、不查權限。
#   存檔流程讀的 26 個替身（25 格 TEdit＋cbEnableAGVFunction）全部是 mustSend。
# 開頁 FormShow（:1304）＝ ReadFile（:1227）→ DoIniDataToForm（:1264）→ fShow=true。FormClose（:1311）只 fShow=false（＝PageDesc::reload 不能用它，
#   見 FileRW/TestIF_File_AGV.cpp）。
#
# ⚠ E84 會因此動起來（golden 行為，照做、寫明）：ReadFile 把 AGV.ini 的 "E84 Enable" 讀進 TestIF_File.bEnableE84；移植樹
#   D:\HT9045\HT9011UC_Cpp_V3.33.906.0\csystem.cpp:30394 `if(USE_E84_Sensor==1 && TestIF_File.bEnableE84)`（MainProc，每一拍）就開始跑
#   E84 交握（DoE84LoaderScan／DoE84UnloaderScan ×3、E84StatusChange、DoE84Loader、DoE84Unloader —— Automation/AGV_E84.cpp、AGV_PortScan.cpp，
#   會開關 SW[SwE84_1_*]／SW[SwE84_2_*] 交握輸出）。USE_E84_Sensor＝Gerneral.ini [System] AGVModal（database.cpp:1655）。
#   所以 AGVModal=1、AGV.ini "E84 Enable"=1 的機台：網頁開一次 Setup.AGV（golden FormShow 讀檔）或存一次檔，E84 交握就開始跑 ——
#   跟 golden 開過 AGV 畫面一樣。開機那一次讀（golden main.cpp:9378）在另一個 patch（B）。
#
# AI(W906-B8-AG1) 20260930 [W906] St01 patch B（Steven 請看：會動真機的 E84 交握輸出）：
#   * 開機讀檔 golden TfMain::DoReadLastData main.cpp:9378 fAGV->ReadFile() → FileRW_AGV_ReadFile()（FileRW/TestIF_File_AGV.cpp；
#     呼叫在 tools/wb_serve.cpp W906_DoReadLastData、FrmAOI 那一行同一行）⇒ AGVModal=1、AGV.ini "E84 Enable"=1 的機台開機就開始跑 E84 交握。
#   * Initial Load／Initial Unload 兩顆鈕（WS form.event click）：golden btInitalLoadClick :1316-1322＝InitialE84LoadTask()（iE84LoadTask=1）、
#     InitialE84LoadSensor()（SW[SwE84_1_LREQ／UREQ／VA／READY／VS0／VS1].Off()，Automation/AGV_E84.cpp:189）、bE84LoaderActionflag[0..2]=false；
#     btInitalUnLoadClick :1324-1330＝同樣三步在 E84_2（iE84UnloadTask、SW[SwE84_2_*]、bE84UnloaderActionflag）。按下當下就關交握輸出、
#     把交握狀態機拉回第 1 步、放掉軌道互鎖旗標。golden 兩顆鈕自己沒有任何檢查；C++ 端的重查＝form.event 既有的一道（運轉中 SystemStart／
#     SoftStart 回 running，RULINGS_20260927 第 2 條第 7 題 A）＋開過頁（同一個 AccessLevel）＋點得到（DFM 父層）。
#     ⚠ golden 是非模態 fAGV->Show()（main.cpp:35780），運轉中開著也按得到；網頁運轉中送不進來（比 golden 嚴，同 OS-5 的寫法，交 ST01-E）。
#
# ⚠ 測試縫：golden 路徑是寫死的字面值 "D:\\HT9045\\config\\AGV.ini"（:1187、:1230）→ W906_AgvIniPath()（FileRW/TestIF_File_AGV.cpp；
#   環境變數 W906_AGVINI_PATH，沒設或空字串＝golden 字面值）。ctest 用它指到沙盒，不碰真的 AGV.ini。
#
# 沒轉（寫明）：
#   * 建構子 :30-34 只設 bATK_AMR_StartFromHostLotStart=false（ATK AMR 流程的成員，不是這張表單的讀寫；移植樹門面 forms/fAGV.h 沒有它）。
#   * sbtExitClick :1297（Down=false; Close(); fShow=false）：頁面自己的 .exitbtn 關視窗；golden 關窗只有 fShow=false，不寫檔。
#   * Timer2Timer :1632（警報框開著時代替主流程跑 E84 交握，B8 P-1 計時器拍子）、ShowE84Sensor 36 顆燈、mmE84Log：不在 AG-1。
#   * edE84_1_TP1MouseDown（:1445，N_INTEGER 0..300）／edAuto1CountMouseDown（:1439，0..20）：小鍵盤在頁面補件（web/page/ht9045_agv_c.js）。
_F = 'TfAGV'

STRUCT = {
    'struct': 'TestIF_File_AGV',
    'prefix': 'AG',
    'class': _F,
    'cpp': 'Automation/AGV.cpp',   # 斜線：產生器會把它寫進註解與字串常值（同 Winway.py）
    'h': 'Automation/AGV.h',
    'files': ['D:\\HT9045\\config\\AGV.ini [Configuration]（W906_AGVINI_PATH 測試縫）'],
    'lists': [],
    'methods': ['FormShow', 'FormClose', 'ReadFile', 'DoIniDataToForm', 'spbSaveClick',
                'btInitalLoadClick', 'btInitalUnLoadClick'],   # AI(W906-B8-AG1) 20260930 patch B：golden :1316／:1324
    'save_methods': ['spbSaveClick'],
    'params': {'FormShow': '', 'FormClose': '', 'spbSaveClick': '', 'btInitalLoadClick': '', 'btInitalUnLoadClick': ''},
    'members': ['bool fShow=false;   // golden Automation/AGV.h:176（本 TU 自己的；移植樹 forms/fAGV.h 門面沒有這個成員，golden 也沒有別的檔讀 fAGV->fShow）'],
    'replace': [
        ('spbSaveClick', 1187, 1187, '測試縫：golden 寫死的 D:\\HT9045\\config\\AGV.ini → W906_AgvIniPath()（沒設 W906_AGVINI_PATH＝golden 字面值）',
         'szDir=W906_AgvIniPath();'),
        ('ReadFile', 1230, 1230, '測試縫：同 :1187',
         'szDir=W906_AgvIniPath();'),
    ],
    'blocks': [],
    'includes': ['cprod.h', 'cmydef.h', 'common.h', 'vclcompat/SysUtils.h',
                 'Automation/AGV_E84.h'],   # AI(W906-B8-AG1) 20260930 patch B：InitialE84LoadTask／InitialE84LoadSensor／…（golden AGV.cpp:197-239，移植樹 AGV_E84.cpp:165-207）
    'decls': ['AnsiString W906_AgvIniPath();   // FileRW/TestIF_File_AGV.cpp（AI(W906-B8-AG1) 20260930：golden AGV.cpp:1187／:1230 的路徑，測試縫 W906_AGVINI_PATH）'],
    'overrides': [],
    # AI(W906-B8-AG1) 20260930 patch B：WS form.event（FileRW/TestIF_File_AGV.cpp 用 PageEventsRegistrar 註冊；頁面 web/page/ht9045_agv_c.js）
    'events': [('btInitalLoad', 'click', 'btInitalLoadClick'),
               ('btInitalUnLoad', 'click', 'btInitalUnLoadClick')],
}
