// ===========================================================================
//  FileRW/GroundMan.cpp -- golden TfGroundMan（GroundMan\GroundMan.cpp，V912）的 C 路入口：Status.GroundMan.html 的「讀寫段」。
//  D:\HT9045\system\GroundMan.ini（系統檔，不跟配方）。頁面補件：web/page/ht9045_groundman_c.js。
//
//  //AI(W906-CRT-GroundMan) 20260926: 新檔（Steven 團隊）。設定：tools/editlist/GroundMan.py（每條 replace 附原因）。
//
//  golden 方法由 tools/gen_editlist.py 轉成 GroundMan.gen.inc（元件改具名替身，名稱＝頁面元件 id）：
//    建構子（:24）          ＝ 開機（CreateForm HT9045.cpp:262）：成員初值、iUseGndBoard（HSys.iGroundManScanPoint）、bOpenClose、
//                              asShowName → :136 ReadGroundOffset()。
//    FormShow（:139）       ＝ 開頁：依 ScanPoint 顯示 gbBoard0..3／labBoard*、labCH_* 名稱、KYEC_LEE 停用兩個輸入框、
//                              PeiXing＋HonPrec 權限才顯示 btnMaintenanceMode。golden FormShow 不讀檔 —— 輸入框的值是最近一次
//                              ReadGroundOffset（開機建構子或上一次存檔）留下的。
//    spbSaveClick（:1418）  ＝ 存檔鈕：A02 → ContinuousTime/Occurrences≥5 → KYEC_LEE 固定 15／2 → WriteIniData ×2 → ReadGroundOffset。
//                              golden 沒有 YES/NO 確認框、沒有 Insufficient 權限檢查（只有 A02）。
//    ReadGroundOffset（:1573）＝ 讀檔器：[System] UseOffset／UseResetByStart／Alarm_Continuous_Time／Alarm_Occurrences、
//                              [Board_<n>_Offset] <asShowName>（n＝1..iUseGndBoard）。CheckAndReadIniData 缺鍵照 golden 補寫預設值。
//  沒有 HTEditList：存檔流程讀的替身（edContinuous_Time、edOccurrences）全部是 mustSend。
//  savedMark＝"GM_WriteIniData"：golden 第一個 WriteIniData（:1466）之前；A02 與 5 秒保護都在它前面 return（沒寫檔）。
//  reload＝golden FormShow：golden FormClose（:228）不重讀檔，而且會 ReStart()（RS232 重開，機台動作）→ 不用它。
//    ⇒ golden 存檔被「必須大於 5」擋下時，輸入框留著被擋的值（VCL TEdit 的 Text 不會自己變回去），機台仍用舊值；
//      頁面重讀看到的也是被擋的值 —— 與 golden 相同（見報告「偏離 golden」一節）。
//
//  ---- 開機與換配方（golden 呼叫點）-----------------------------------------------------------------------------
//    開機：golden CreateForm(TfGroundMan)（HT9045.cpp:262，TfBarCode :243 之後）→ 建構子 :136 ReadGroundOffset()
//          → FileRW_GroundMan_Boot()（tools/wb_serve.cpp，FileRW_BarCode_Boot() 之後；要在 LoadMachineConfig 之後：
//          建構子讀 HSys.iGroundManScanPoint、ReadGroundOffset／FormShow 讀 CUSTOMER_CODE）。
//    TfMain::FormShow main.cpp:11533-11549 `#ifndef SOFT_SIMULTE if(USE_GROUND_MAN>0) fGroundMan->Init_GM_RS232();`
//          → Init_GM_RS232（:259）開頭 :266 也呼叫 ReadGroundOffset（同一個檔，建構子已讀過）＋開 COM：RS232，屬 S48，不接。
//    換配方：不讀（GroundMan.ini 不跟配方；golden DoReadLastData／ChangeSetUpFile 沒有 fGroundMan）。
//
//  ---- 狀態放哪（Steven 團隊 20260926）------------------------------------------------------------------------
//    golden TfGroundMan 的非元件成員（dOffset、bUseOffset、Alarm_* 等）在本 TU（gen.inc 的 static）；只有門面
//    forms/fGroundMan.h 已經有的兩個執行期狀態 iGroundMasterTask／bRs232Ok 用 #define 接到 fGroundMan（全樹一份，
//    csystem.cpp 的 fGroundMan->ReStart() 與門面 Timer1Timer 讀同一個）。⚠ gen.inc 之後這兩個名字是巨集：
//    本檔不要寫 fGroundMan->iGroundMasterTask（會展開成 fGroundMan->(fGroundMan->…)），直接寫 iGroundMasterTask。
//    S48 翻 RS232 監測（DoGroundMasterMonitor／comGMReceiveData／Init_GM_RS232）時，建議加進 tools/editlist/GroundMan.py
//    的 methods，與 ReadGroundOffset 讀進來的 dOffset／Alarm_* 在同一個 TU。
//
//  ---- 不在本檔（機台動作／RS232，S48；網頁停用，見 ht9045_groundman_c.js）-------------------------------------------
//    spbStartComClick（:247 comGM->StopComm＋Init_GM_RS232）、spbStopComClick（:253 comGM->StopComm）、Init_GM_RS232（:259）、
//    SetGroundMaster（:457 WriteCommData）、comGMReceiveData（:303，寫 slGroundManLog）、Timer1Timer（:551）、
//    DoGroundMasterMonitor（:577，警報時 :1295 StopAllMotor＋SystemStart=false）、ShowGroundManLog（:1480，寫 D:\HT9045_Log\GroundManLog）、
//    FormClose（:228 ReStart）、sbtExitClick（:1472 Close → FormClose）、ReStart（:1636）、btnMaintenanceMode（DFM 無 OnClick；
//    Down 狀態給 DoGroundMasterMonitor :1289／:1389 壓掉 PeiXing 的警報）。
// ===========================================================================
#include "FileRW/GroundMan.gen.inc"

#include <cstdio>
#include <string>

#include "FileRW/_EditPage.h"
#include "WebBridge/JsonWriter.h"

namespace {
bool g_booted = false;
bool Booted() { return g_booted; }

// PageDesc::reload：golden 沒寫檔的路徑（A02、必須大於 5）＝ golden 表單仍開著、輸入框留著操作員的值；
// golden FormClose 不重讀檔（而且會 RS232 ReStart）→ 用 FormShow（同 FileRW/ShuttleMove.cpp 的選擇）。FormShow 不碰兩個輸入框。
void Reload() { GM_FormShow(); }

// PageDesc::extraJson：通用 proxies 帶不到的東西。
//   * labStatus：golden Timer1Timer（:572）每 30 ms 寫 labStatus->Caption=(AnsiString)iGroundMasterTask —— 這裡是開頁當下的值
//     （Timer1Timer／RS232 監測未移植，S48；bRs232Ok 一直是 false，所以 golden 在這台也只會顯示初值 1）。
//   * useGroundMan／scanPoint／comPort／alarmOhm：Gerneral.ini [Ground_Man]（HSys，golden database.cpp:1420-1423），頁面顯示選配狀態。
//   * machine：ReadGroundOffset 讀進記憶體的值（RS232 監測用的那一份；golden 畫面不顯示 offset，這裡給探針／除錯核對）。
//   * runtime：執行期顯示（labValue_*／led_*／labCount_*／labBoardOhrm*／labBoardVersion*／mmGroundManLog）沒有接 —— 寫它們的
//     comGMReceiveData／DoGroundMasterMonitor／ShowGroundManLog 是 RS232 監測本體（S48）。
std::string ExtraJson()
{
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("labStatus").String(AnsiString(iGroundMasterTask).c_str());
    w.Key("rs232Ok").Bool(bRs232Ok);
    w.Key("useGroundMan").Number((wb_int64)USE_GROUND_MAN);
    w.Key("scanPoint").Number((wb_int64)HSys.iGroundManScanPoint);
    w.Key("useGndBoard").Number((wb_int64)iUseGndBoard);
    w.Key("comPort").String(HSys.asGroundManComPort.c_str());
    w.Key("alarmOhm").Number((wb_int64)HSys.iGroundManAlarmOhm);
    w.Key("machine").BeginObject();
    w.Key("useOffset").Bool(bUseOffset);
    w.Key("useResetByStart").Bool(bGroundManResetByStart);
    w.Key("alarmContinuousTime").Number((wb_int64)Alarm_Continuous_Time);
    w.Key("alarmOccurrences").Number((wb_int64)Alarm_Occurrences);
    w.Key("offsets").BeginArray();
    for (int i = 0; i < iUseGndBoard && i < 4; ++i) {
        w.BeginObject();
        w.Key("section").String(AnsiString().sprintf("Board_%d_Offset", i + 1).c_str());
        w.Key("keys").BeginArray();
        for (int j = 0; j < 8; ++j) w.String(asShowName[i][j].c_str());
        w.EndArray();
        w.Key("values").BeginArray();
        for (int j = 0; j < 8; ++j) w.Number(dOffset[i][j]);
        w.EndArray();
        w.EndObject();
    }
    w.EndArray();
    w.EndObject();
    w.Key("runtime").BeginObject();
    w.Key("wired").Bool(false);
    w.Key("why").String("golden Timer1Timer :551 / DoGroundMasterMonitor :577 / comGMReceiveData :303 / ShowGroundManLog :1480 "
                        "are the RS232 monitor (S48, not ported): labValue_*, led_*, labCount_*, labBoardOhrm*, labBoardVersion*, "
                        "mmGroundManLog have no writer in the port");
    w.EndObject();
    w.EndObject();
    return w.Str();
}

const filerw::PageDesc kPage = {
    "GroundMan", "TfGroundMan", "Status.GroundMan.html",
    nullptr, nullptr, 0,
    kGM_SaveReads, (int)(sizeof(kGM_SaveReads) / sizeof(kGM_SaveReads[0])),
    &GM_FormShow, &GM_spbSaveClick, "GM_WriteIniData", &Reload, &Booted,
    nullptr, &ExtraJson,
};
filerw::PageRegistrar g_reg(&kPage);
}  // namespace

// golden TfGroundMan 建構（HT9045.cpp:262 CreateForm）：DFM 設計期狀態 → 建構子（:24，內含 :136 ReadGroundOffset 讀
// system\GroundMan.ini）→ 存檔流程讀的替身 → 容器替身與父子。冪等。
// 前提：LoadMachineConfig 之後（HSys.iGroundManScanPoint、CUSTOMER_CODE）。
// ⚠ 會讀、缺鍵時會寫 D:\HT9045\system\GroundMan.ini（golden CheckAndReadIniData 補預設值；golden 每台機台開機都這樣，
//   不看 USE_GROUND_MAN）。鍵都在的檔不會被改。
void FileRW_GroundMan_Boot()
{
    if (g_booted) return;
    GM_DfmItems();
    GM_DfmState();
    GM_TfGroundMan();
    GM_CreateSaveProxies();
    GM_CreateContainerProxies();
    std::printf("FileRW GroundMan: TfGroundMan proxies ready (%d save reads) -- golden GroundMan.cpp ctor :24 -> ReadGroundOffset :1573 "
                "(system\\GroundMan.ini): USE_GROUND_MAN=%d ScanPoint=%d boards=%d UseOffset=%d UseResetByStart=%d "
                "Alarm_Continuous_Time=%d Alarm_Occurrences=%d\n",
                (int)(sizeof(kGM_SaveReads) / sizeof(kGM_SaveReads[0])), USE_GROUND_MAN, HSys.iGroundManScanPoint, iUseGndBoard,
                (int)bUseOffset, (int)bGroundManResetByStart, Alarm_Continuous_Time, Alarm_Occurrences);
    g_booted = true;
}
