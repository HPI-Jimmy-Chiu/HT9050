// ===========================================================================
//  FileRW/Monitor.cpp -- golden TfMonitor（Monitor\MonitorInterface.cpp）的 C 路入口：D:\HT9045\system\MVData.ini 的讀寫段。
//  沒有頁面（golden 主畫面 sbMonitorView 鈕只在 IniConfig.bC11UseMonitorView 時顯示，main.cpp:23684／:32378（V912 :24392／:33501；
//  舊寫的 :33471 是 c2f6c75a 之前的號碼）fMonitor->Show()）。
//  //AI(W906-E031) 20261003 [W906] (St01)：產生器對本結構改讀 golden 906 0618（E-031 全面切換第 1 批，tools/golden_root.py）；
//    MonitorInterface.cpp／.h／.dfm 0618 與 V912 逐位元組相同 ⇒ 本檔的 :N 兩棵都對；main.cpp／HT9045.cpp 改成 0618（括號 V912）。
//
//  //AI(W906-FRW-S110) 20260926: 新檔（Steven 團隊，RULINGS_20260926 S110）。設定：tools/editlist/Monitor.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 Monitor.gen.inc（元件改具名替身，名稱＝golden 元件名）：
//    LoadTCPIPParament（:79）  ＝ 讀 MVData.ini [Setup] IP／Port、[Specific] 5 鍵 → sADDRESS／iPORT＋7 個元件。只讀（TIniFile::Read* 不補寫）。
//    SaveTCPIPParament（:98）  ＝ 7 個元件 → 寫同一檔 7 鍵（TIniFile::Write*，每一鍵立即落地；檔不在會建）。
//    FormShow（:47）           ＝ 開頁：bShow=true → LoadTCPIPParament()。
//    sbMVUpdateClick（:123）   ＝ Update 鈕 → SaveTCPIPParament()。
//
//  ---- golden 讀寫時機（本檔提供函式，呼叫點由整合者插進 tools/wb_serve.cpp，片段見交件報告）--------------------------
//    開機：CreateForm(TfMonitor)（HT9045.cpp:246，TfBarCode :242 之後；V912 :247／:243）→ 建構子 :32 LoadTCPIPParament() → FileRW_Monitor_Boot()。
//          golden 每台機台開機都讀（不看 IniConfig.bC11UseMonitorView）；檔不在就是預設值（127.0.0.1／7000／false／0…），不建檔。
//          建構子其餘的行：:26-29、:35-44 門面 forms/fMonitor.cpp:32 在 static init 已照做；:30-31、:33 是 MVCtrl（TCP），不做。
//    換配方：不讀（MVData.ini 不跟配方；golden DoReadLastData／ChangeSetUpFile 沒有 fMonitor）。
//    開頁：FileRW_Monitor_FormShow()（golden FormShow :54 再讀一次）；存檔：FileRW_Monitor_sbMVUpdateClick()。⚠ 兩支目前都沒有呼叫者。
//
//  ---- 狀態放哪 ------------------------------------------------------------------------------------------------
//    7 個元件（edMVAddress、edMVPort、cbWhenHDFullAlarm、edLowHDSpace、edWhenHDFullPrompt、cbAfterHandlerTrayFeedMonitor1ClosedVideo、
//    edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime）＝門面 forms/fMonitor.h 的那 7 個物件（MN_AdoptPortWidgets 開機最先登記）：
//    golden MonitorTimerTimer（:143，HD 容量警報、錄影關閉等待）讀的就是這幾個元件，之後翻它時讀到的是本檔讀進來的值。
//    sADDRESS／iPORT：門面 fMonitor->sADDRESS／iPORT（Monitor.gen.inc 用 #define 接過去 —— ⚠ gen.inc 之後這兩個名字是巨集，
//    本檔直接寫名字，不要寫 fMonitor->sADDRESS）。bShow：本 TU 自己的（理由見 tools/editlist/Monitor.py members）。
//
//  ---- 不在本檔（通訊／機台動作，交 Jimmy）--------------------------------------------------------------------------
//    MVCtrl（MonitorTCPIP，建構子 :30-31／:33）與 Load／Save 結尾的 MVCtrl->InitialSocket(sADDRESS, iPORT)（:94／:119）、
//    sbMVConnectClick :128／sbMVDisconnectClick :133（Connect／Disconnect）、MonitorTimerTimer :143（自動重連、HD 容量警報 ShowMyMessage、
//    錄影開關）、sbSendCommandClick、sbMonitor1OpenClick 等錄影鈕、OpenMonitorVedio／StopMonitorVedio（門面 GATE M-4）、GetMonitorHDSpec。
// ===========================================================================
#include "FileRW/Monitor.gen.inc"

#include <cstdio>

namespace {
bool g_booted = false;

// 門面 forms/fMonitor.h 已有的同名同型別元件 → 直接當替身（產生器 'adopt' 的手寫版：門面寫的是 `new TEdit()`，產生器只認
// `new vclcompat::TEdit()`）。要在任何 EL<>("TfMonitor", …) 之前（DfmState／CreateSaveProxies 會建）。
void MN_AdoptPortWidgets()
{
    filerw::ELKeep("TfMonitor", "edMVAddress", fMonitor->edMVAddress);
    filerw::ELKeep("TfMonitor", "edMVPort", fMonitor->edMVPort);
    filerw::ELKeep("TfMonitor", "cbWhenHDFullAlarm", fMonitor->cbWhenHDFullAlarm);
    filerw::ELKeep("TfMonitor", "edLowHDSpace", fMonitor->edLowHDSpace);
    filerw::ELKeep("TfMonitor", "edWhenHDFullPrompt", fMonitor->edWhenHDFullPrompt);
    filerw::ELKeep("TfMonitor", "cbAfterHandlerTrayFeedMonitor1ClosedVideo", fMonitor->cbAfterHandlerTrayFeedMonitor1ClosedVideo);
    filerw::ELKeep("TfMonitor", "edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime", fMonitor->edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime);
}
}  // namespace

// golden TfMonitor 建構（HT9045.cpp:246 CreateForm，V912 :247）：收養門面元件 → DFM 設計期狀態（Text 初值）→ 存檔流程讀的替身 → 容器替身與父子
// → 建構子 :32 LoadTCPIPParament()（讀 D:\HT9045\system\MVData.ini；檔不在＝預設值，不建檔、不寫檔）。冪等。
void FileRW_Monitor_Boot()
{
    if (g_booted) return;
    MN_AdoptPortWidgets();
    MN_DfmItems();
    MN_DfmState();
    MN_CreateSaveProxies();
    MN_CreateContainerProxies();
    MN_LoadTCPIPParament();   // golden MonitorInterface.cpp:32（建構子）
    g_booted = true;
    std::printf("FileRW Monitor: TfMonitor proxies ready (%d save reads) -- golden MonitorInterface.cpp ctor :32 LoadTCPIPParament "
                "(system\\MVData.ini): IP=%s Port=%d HD_Space=%d HD_Space_Low=%s HD_Space_Low_Prompt=%s HD_Closed_Wait=%d "
                "HD_Closed_Wait_Time=%s (MVCtrl/TCP not ported)\n",
                (int)(sizeof(kMN_SaveReads) / sizeof(kMN_SaveReads[0])), sADDRESS.c_str(), iPORT,
                (int)fMonitor->cbWhenHDFullAlarm->Checked, fMonitor->edLowHDSpace->Text.c_str(),
                fMonitor->edWhenHDFullPrompt->Text.c_str(), (int)fMonitor->cbAfterHandlerTrayFeedMonitor1ClosedVideo->Checked,
                fMonitor->edAfterHandlerTrayFeedMonitor1ClosedVideoWaitTime->Text.c_str());
}

// golden TfMonitor::FormShow（:47）：開頁（會再讀一次 MVData.ini）。目前沒有呼叫者（沒有頁面）。
void FileRW_Monitor_FormShow()
{
    if (!g_booted) FileRW_Monitor_Boot();
    MN_FormShow();
}

// golden TfMonitor::sbMVUpdateClick（:123）→ SaveTCPIPParament（:98）：寫 D:\HT9045\system\MVData.ini 7 鍵。
// 目前沒有呼叫者（沒有頁面、沒有 WS 指令）；給之後的頁面用（頁面值先套到替身，見 kMN_SaveReads）。
void FileRW_Monitor_sbMVUpdateClick()
{
    if (!g_booted) FileRW_Monitor_Boot();
    MN_sbMVUpdateClick();
}
