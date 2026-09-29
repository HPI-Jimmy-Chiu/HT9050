// ===========================================================================
//  JsonBridge/actions/MainTesterConnect.cpp
//
//  AI(W906-GB-P2e) 20260926.  act.main.testerConnect —— golden TfMain::imgTesterClick 的翻譯＋網頁回報。
//  說明在 .h 檔頭。golden：912 main.cpp:29732-29794（906 :28766 起，程式碼相同）。
//
//  gate 登記（每個都在程式旁邊有 `#if 0 // TODO(W906-GB-P2e)`）：
//    E1 :29751  DoPassword() -- TfMain 門面沒有這個成員（FileRW/TestIF_File.cpp:2576 同樣當成「密碼失敗」）。
//               只影響 CC_ASE_CL：確認框按「是」之後照 golden 要密碼，這裡一律當失敗、不切換（fail-safe）。
//    E2 :29777 / :29783  fLotInfo->labTCPIPSimulate->Visible -- TfLotInfo 沒有這個成員（TesterComm/Tcp/TcpPump.cpp
//               的 P5 gate 同一個）。同一段的 TimerTCPIPConnect／TimerProcessTCPData 開關與 ClientSocket_TCPIP->Close()
//               由 P5 的 TcpPump 每圈依 golden 條件（TCP_IP_MODE && ON_LINE）自己做，這裡不重複。
//
//  AI(W906-GB-P2e-EN) 20260928 (St02-E)（St01 15:3x 查到）：golden 的圖示本身有致能閘——ChangeLevelAttr
//    `imgTester->Enabled=authMainForm[7];`（906_0625_Steven main.cpp:12539；912 :13060，就在 imgRunMode 那行前面），
//    是 golden 唯一設 imgTester->Enabled 的地方。不致能的圖示按不到，imgTesterClick 根本不會跑 ⇒ 這裡在翻譯本體之前先擋，
//    回 guard "disabled"（跟 St01 主畫面其他圖示同一種拒絕），不呼叫 ChangeTesterConnect。D1（ChangeTesterConnect 裡面的
//    權限檢查）照舊。authMainForm 由開機的 GetMainAuth（cAuthority.cpp:460，Security_new.def [Main]）填，跟 golden 一樣。
// ===========================================================================
#include "JsonBridge/actions/MainTesterConnect.h"

#include <string>

#include "JsonBridge/EventLog.h"   // LogAppend（SKILL §4.7 規則 1：每個 act.* 在本體前記一筆）
#include "WebBridge/JsonWriter.h"
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"           // CC_ASE_CL / CC_KYEC_XILINX / ON_LINE / OFF_LINE / TCP_IP_MODE / eTrayCount
#include "cmydef.h"                // SystemStart, CUSTOMER_CODE, sStackBinTemp[]
#include "cprod.h"                 // TestIF_File
#include "LastSet.h"               // LastSet.iTester
#include "Config.h"                // IniConfig.bEnable_SECS_GEM
#include "CosFunction.h"           // CosFunction.bOffLineBin
#include "canary_support.h"        // ShowMyMessageBox_YES_NO
#include "aHotPlateSubstrate.h"    // SetRunStartMode（RunStartMode.cpp）
#include "forms/fMain.h"           // fMain->ChangeTesterConnect
#include "forms/fSecurity.h"       // fSecurity->Insufficient
#include "cAuthority.h"            // authMainForm[7]（golden ChangeLevelAttr :12539）  AI(W906-GB-P2e-EN) 20260928 (St02-E)
#include "forms/fShowBinSelect.h"  // fShowBinSelect->ShowBinSel / InitShowBinDigital
#include "BarCode/BarCode.h"       // fBarCode->ReadFile
#include "SECSGEM/SecsEventType.h" // SECS_EVENT.SwitchTesterMode
#include "SECSGEM/SecsEventReport.h"  // EventReport

namespace ht9045 {
namespace sjson {

namespace {

const char* const kGolden = "main.cpp:29732-29794 imgTesterClick -> ChangeTesterConnect(10)";

// imgTesterClick 在哪一步停下（golden 是一路 return，這裡記下來給網頁看）。
enum TcStop { kTcDone = 0, kTcSystemStart, kTcNoAuthority, kTcAseClNo, kTcAseClPassword };

struct TcResult
{
    TcStop stop;
    int    changeRet;        // ChangeTesterConnect 的回傳（有呼叫才有意義）
    bool   offLineBin;       // 有沒有跑 bOffLineBin 那段
    bool   barCodeReread;    // 有沒有跑 CC_KYEC_XILINX 那段
    bool   secsReported;     // 有沒有送 SECS 事件
};

//******************************************************************************
// golden 912 main.cpp:29732-29794 TfMain::imgTesterClick，照翻。TfMain 成員改寫成 fMain->X；其餘是 golden 原文。
TcResult W906_ImgTesterClick()
{
    TcResult r = { kTcDone, 0, false, false, false };
    int ret=0;

    if(SystemStart)                                                             //JerryYang 20170412 (Steven) QUALCOMM先取消此功能
    {
        r.stop=kTcSystemStart;
        return r;
    }

    if(fSecurity->Insufficient(8)==false)                                       //jou 981207 權限控制
    {
        r.stop=kTcNoAuthority;
        return r;
    }

    if(CUSTOMER_CODE==CC_ASE_CL)
    {
        ret=ShowMyMessageBox_YES_NO("Sure to change the tester connect stauts?", "確定要切換測試機連線狀態?");
        if(ret==2)
        {
            r.stop=kTcAseClNo;
            return r;
        }
        else
        {
#if 0 // TODO(W906-GB-P2e): E1 DoPassword() is not a TfMain facade member (password dialog; FileRW/TestIF_File.cpp:2576 treats it as failed too) -- golden main.cpp:29751
            if(fMain->DoPassword()==false)                                      //Steven 20101124
            {
                return;
            }
#else
            r.stop=kTcAseClPassword;                                            //AI(W906-GB-P2e) 20260926: E1 -- no password dialog = password failed (fail-safe, no switch)
            return r;
#endif
        }
    }

    r.changeRet=fMain->ChangeTesterConnect(10);                                 //AI(W906-GB-P2e) 20260926: TfMain member -> fMain->; golden ignores the return

    if(CosFunction.bOffLineBin)                                                 //ChungHung 20111110
    {
        SetRunStartMode();
        fShowBinSelect->ShowBinSel();
        fShowBinSelect->InitShowBinDigital();
        r.offLineBin=true;
    }

    if(CUSTOMER_CODE==CC_KYEC_XILINX)                                           //Alick 20170331 (wei) 連線模式變換時，重新讀檔
    {
       fBarCode->ReadFile();
       r.barCodeReread=true;
    }

    if(TestIF_File.iTestType==TCP_IP_MODE)
    {
        //AI(W906-GB-P2e) 20260926: golden :29772-29784 switches fTesterTCP's two timers and closes ClientSocket_TCPIP.
        //   In V906 the P5 TcpPump (TesterComm/Tcp/TcpPump.cpp) does exactly that on every tick from the same golden
        //   condition (TCP_IP_MODE && LastSet.iTester==ON_LINE), so nothing is repeated here.
#if 0 // TODO(W906-GB-P2e): E2 fLotInfo->labTCPIPSimulate is not a TfLotInfo member (same gate as the P5 TcpPump) -- golden main.cpp:29777 / :29783
        if(LastSet.iTester==OFF_LINE)                                           //wei 20211027 open short TCP/IP
            fLotInfo->labTCPIPSimulate->Visible=true;
        else
            fLotInfo->labTCPIPSimulate->Visible=false;
#endif
    }

    for(int i=0; i<eTrayCount; i++)                                             //JerryYang 20231218 : P53防混功能
    {
        sStackBinTemp[i]="";
    }

    if(IniConfig.bEnable_SECS_GEM==true)                                        //Steven 20140528 : Secs Gem
    {
        EventReport(SECS_EVENT.SwitchTesterMode);                               //10     切換 Tester Online / Offline Mode
        r.secsReported=true;
    }
    return r;
}

const char* TesterName(int iTester)
{
    if (iTester == ON_LINE)  return "ON_LINE";
    if (iTester == OFF_LINE) return "OFF_LINE";
    if (iTester == _2D_SORT) return "2D_SORT";
    return "OTHER";
}

}  // namespace

std::string DoTesterConnectAction(const std::string& payloadJson)
{
    (void)payloadJson;                          // golden 的按鈕沒有參數（切換）
    webbridge::JsonWriter w;
    if (!authMainForm[7]) {                     // AI(W906-GB-P2e-EN) 20260928 (St02-E): golden ChangeLevelAttr main.cpp:12539 imgTester->Enabled=authMainForm[7]
        LogAppend(kLogProcess, "act.main.testerConnect refused: disabled (authMainForm[7] is false)", "", "", "act");
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("disabled");
        w.Key("detail").String("golden main.cpp:12539 imgTester->Enabled=authMainForm[7] is false (Security_new.def [Main]) -- "
                               "the operator cannot click it, so imgTesterClick never runs");
        w.Key("golden").String(kGolden);
        w.Key("before").String(TesterName(LastSet.iTester));
        w.Key("after").String(TesterName(LastSet.iTester));
        w.Key("modeChanged").Bool(false);
        w.EndObject();
        return w.Str();
    }
    if (fMain == 0 || fSecurity == 0 || fShowBinSelect == 0 || fBarCode == 0) {
        w.BeginObject();
        w.Key("executed").Bool(false);
        w.Key("guard").String("forms-not-created");
        w.Key("detail").String("fMain／fSecurity／fShowBinSelect／fBarCode 有一個還是 NULL");
        w.EndObject();
        return w.Str();
    }

    LogAppend(kLogProcess, "act.main.testerConnect pressed", "", "", "act");

    const int before = LastSet.iTester;
    const TcResult r = W906_ImgTesterClick();
    const int after = LastSet.iTester;

    w.BeginObject();
    if (r.stop == kTcSystemStart) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("system-running");
        w.Key("detail").String("SystemStart：golden 在機台運轉中直接 return（main.cpp:29736）");
    } else if (r.stop == kTcNoAuthority) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("not-authorized");
        w.Key("detail").String("fSecurity->Insufficient(8)==false：權限項目 [08] Main - Tester On/Off line（main.cpp:29739）");
    } else if (r.stop == kTcAseClNo) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("cancelled");
        w.Key("detail").String("CC_ASE_CL：確認框按了「否」（main.cpp:29744-29747）");
    } else if (r.stop == kTcAseClPassword) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("password-not-ported");
        w.Key("detail").String("CC_ASE_CL：golden 接著要 DoPassword()（main.cpp:29751），移植樹沒有密碼框，當成密碼失敗、不切換");
    } else {
        w.Key("executed").Bool(true);
        w.Key("guard").String("");
    }
    w.Key("golden").String(kGolden);
    w.Key("before").String(TesterName(before));
    w.Key("after").String(TesterName(after));
    w.Key("modeChanged").Bool(before != after);
    if (r.stop == kTcDone) {
        w.Key("changeTesterConnectReturn").Number((wb_int64)r.changeRet);
        if (before == after)
            w.Key("note").String("模式沒變。ChangeTesterConnect 本體（golden :12581-12778）還沒翻（P2d）時，"
                                 "forms/fMain.cpp:510 的替身不改任何東西；翻好之後，沒變代表 golden 自己擋下"
                                 "（例如機台裡還有 IC 且非 SOFT_SIMULTE、權限、或 bOneCycleOperateChangeON_line）。");
        w.Key("offLineBinRefresh").Bool(r.offLineBin);
        w.Key("barCodeReread").Bool(r.barCodeReread);
        w.Key("secsEventSwitchTesterMode").Bool(r.secsReported);
    }
    w.EndObject();
    return w.Str();
}

void WriteTesterConnectActionSchema(webbridge::JsonWriter& w)
{
    w.BeginObject();
    w.Key("cmd").String("act.main.testerConnect");
    w.Key("golden").String(kGolden);
    w.Key("goldenBody").String("main.cpp:12581-12778 ChangeTesterConnect（P2d；翻好之前是 forms/fMain.cpp:510 的替身）");
    w.Key("args").BeginObject();
    w.EndObject();
    w.Key("toggle").String("沒有參數：golden 的按鈕是切換，ChangeTesterConnect(10) 依目前模式決定下一個模式");
    w.Key("guards").BeginArray();
    w.String("disabled -- golden ChangeLevelAttr :12539 imgTester->Enabled=authMainForm[7]（圖示按不到）");
    w.String("forms-not-created -- 移植樹自己的狀態");
    w.String("system-running -- golden :29736");
    w.String("not-authorized -- golden :29739 fSecurity->Insufficient(8)");
    w.String("cancelled -- golden :29745（CC_ASE_CL 確認框按否）");
    w.String("password-not-ported -- golden :29751 DoPassword（移植樹沒有，當成失敗）");
    w.EndArray();
    w.EndObject();
}

}  // namespace sjson
}  // namespace ht9045
