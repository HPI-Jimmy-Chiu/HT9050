// =============================================================================
//  LotInfo_E020.cpp -- todo E-020 LI-6 / LI-11 / LI-13 (St01): golden TfLotInfo pieces the port did not have, and their web route
//
//  //AI(W906-E020-LI6) 20261002 [W906] (St01): new file (ht9045_sm, CMakeLists.txt:2499 same line). Jimmy RULINGS_20261001 #0
//    (translate as golden, ask only where golden has no answer); Steven "follow BCB, fewer questions".
//  golden = V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\uLotInfo.cpp (Big5; read as cp950), uLotInfo.h, uLotInfo.dfm.
//
//  LI-6  RTC change file.  enum eWaitRtcDeleteTask :5357-5363, file-scope TQPF_Timer WaitRtcDeleteDelay :5364,
//        TfLotInfo::Timer1Timer :5366-5453, TfLotInfo::RTCChangeFile(bool bNeedDelete=true) :5455-5475.
//        Timer1 = TTimer (dfm :14621-14626 Enabled=False Interval=100 OnTimer=Timer1Timer).
//    Golden callers (V912): cSetUp.cpp:2846 / :2854 (setup read; port cSetUp.cpp:1084 / :1094), uhome.cpp:1946 (homing; port
//      uhome.cpp:1554, reader :1956 bRTCChangeFileFinish), main.cpp:11017 (boot, REAL_TIME_CCD block; that FormShow part is not in
//      the port), cContact.cpp:14673 (ROI learning; fContact is a shim, not ported).
//    The state machine and every flag run as golden. Every COM2 RTC-vision statement is GATEd: the port's COM2 is TCOM2Shim
//      (atester_shims.h:373-462), whose own note says the RTC vision half (sRealTimeCom_Send / SendCommToVision /
//      bRealTimeCom_ReceiveOK / rt* channels / InitRealTimeCCDPara) is not ported -- same gate as WebStart.cpp:1224,
//      csystem.cpp:7733 / :31352, atester.cpp:6454. Each gated statement keeps golden's text under #if 0 and logs one skip
//      (W906_E020_RtcSkip: printf + the list in LotInfo_E020.h) with the string golden would have sent.
//    Consequence, as golden with a dead RTC: the DELETE reply never comes, wrdWaitReply ends on golden's own 10 s timeout (:5434),
//      wrdEndTask sets bSendRealCCDSendStart=true and bRTCChangeFileFinish=true. So once Timer1 is ticked, homing on an RTC
//      machine (uhome.cpp:1954-1962) goes on without the vision having received @FILE / @SITE / MODEL (human-review A).
//    Nobody ticks Timer1 yet (todo X-1 / E-025, Jimmy's timer table): W906_TfLotInfo_Timer1Timer() below is the OnTimer for a
//      future timer card (VCL rule: only while Timer1->Enabled). Until then RTCChangeFile arms Timer1 and the SM waits -- the
//      homing wait at uhome.cpp:1956 stays as it is today ("RTC Change File TimeOut" every 10 s).
//  LI-11 btChangeFileClick :10394-10418 (dfm :4641-4648; Visible = FormShow :572, port forms/fLotInfo.cpp:6135).
//    The branch is chosen as golden; none of the three is ported (fBarCode has none of btBarcodeChangeFileDisConnect / Connect,
//    InitialBarcodeScanChangeFile, TimerBarcodeChangeFile; DoBarcodeChangeFile / iInitialChangeFileTask are not ported --
//    cStateRecord.cpp:1938 GATE, WebRecipeChange.cpp:537 gap). Each branch refuses with whyNot "GATE (W906-E020-LI11-n)";
//    bBarcodeConnect is not set alone (it would make no sense without the scan init + timer, and nothing reads it in the port).
//  LI-13 palSecsGemMouseDown :10532-10606 (dfm palSecsGem :314-323 OnMouseDown). The body is jimmychiu's facade copy
//    forms/fLotInfo.cpp:4794 (golden as is); this file routes the web mouse-down to it. Its sixth click calls SetLotID
//    ("0123456789"), which writes AuthPath config.ini [Lot Info] (seam W906_AUTH_PATH, common.cpp:31 / :168).
//
//  WEB.  act.lotInfo.changeFile           value {}                          -> golden btChangeFileClick
//        act.lotInfo.palSecsGemMouseDown  value {"button":"left|right|middle","seq":n} -> golden palSecsGemMouseDown
//    JsonBridge/ChanAction.cpp:348 (same line) -> ht9045::sjson::W906_LotInfoE020Act. Like every act.*: operator token, WebCmdGuard
//    (same cmd + value within 400 ms -> busy:; the page sends a click serial "seq" so each mouse-down is its own command -- golden
//    counts every mouse-down), no FormLock (E-021 precedent; ChanAction.cpp header), refused while the program closes.
//    C++ re-checks what the operator could reach (the page decides nothing): changeFile = tsBarCode->TabVisible and
//    btChangeFile->Visible after W906_RefreshTabVisible (golden FormShow :571-572); palSecsGemMouseDown = tsLotID->TabVisible
//    (W906_RefreshTabVisible) and W906_LotEndPanelVisible (palSecsGem->Visible, FormShow :367-371 / :407-412).
//    Page: D:\HT9045\web\page\ht9045_lotinfo_e020.js.
//  ctest: E020_LotInfoRTC / E020_LotInfoActs (tests/test_e020_lotinfo_e020.cpp), E020_LotInfoPage (node).
// =============================================================================
#include "LotInfo_E020.h"

#include "forms/fLotInfo.h"
#include "forms/fSetup.h"         // fSetup->fShow (golden csetup.h:261)
#include "forms/fMain.h"          // fMain->cbSetupFileName->Text
#include "cmydef.h"               // InitialOK, CUSTOMER_CODE, BAR_CODE_INSTALL, bUseTwoArm32Site, bDoROILearning, bSendRealCCDSendStart, SystemStart, AccessLevel
#include "cprod.h"                // TestIF_File (sRtcFileName, iTestMode, bEnableBarCode, b2DUseSubJob)
#include "MachineType.h"          // CC_*, eTestMode, ebct*
#include "Config.h"               // IniConfig.bD31RTCChangeRecipeNeedreCreateModel
#include "CosFunction.h"          // CosFunction.b2DUseSubJobFunction
#include "atester_shims.h"        // COM2 (TCOM2Shim: bCCDDummyRum only on the RTC side)
#include "myTimer.h"              // TQPF_Timer
#include "W906FormShowing.h"      // W906_FormShowing (tools/fshow_audit.py rule)
#include "vclcompat/ShiftState.h" // TMouseButton / TShiftState
#include "Public/cJSON.h"         // act value parsing (ht9045_public)

#include <cstdio>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------------------------------------------------------
//  LI-6 skip log (port-only bookkeeping, not golden)
// ---------------------------------------------------------------------------------------------------------------------------
namespace {
std::vector<W906E020RtcSkip> g_w906RtcSkips;
long g_w906RtcSkipTotal = 0;
const size_t kW906RtcSkipKeep = 200;

void W906_E020_RtcSkip(const char* gate, const char* golden, const AnsiString& value)
{
    ++g_w906RtcSkipTotal;
    W906E020RtcSkip s;
    s.gate = gate;
    s.golden = golden;
    s.value = value.c_str();
    if (g_w906RtcSkips.size() >= kW906RtcSkipKeep) g_w906RtcSkips.erase(g_w906RtcSkips.begin());
    g_w906RtcSkips.push_back(s);
    std::printf("[E020-LI6] GATE (%s) skipped: %s%s%s%s -- TCOM2Shim has no RTC vision half (atester_shims.h:366-462)\n",
                gate, golden, value.IsEmpty() ? "" : " \"", value.c_str(), value.IsEmpty() ? "" : "\"");
    std::fflush(stdout);
}
}  // namespace

const std::vector<W906E020RtcSkip>& W906_E020_RtcSkips() { return g_w906RtcSkips; }
long W906_E020_RtcSkipTotal() { return g_w906RtcSkipTotal; }
void W906_E020_RtcSkipsClear() { g_w906RtcSkips.clear(); }

// ---------------------------------------------------------------------------------------------------------------------------
//  LI-6 golden V912 uLotInfo.cpp:5356-5475
// ---------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------
enum eWaitRtcDeleteTask
{
    wrdInitial=1,
    wrdSendComm,
    wrdWaitReply,
    wrdEndTask
};
TQPF_Timer WaitRtcDeleteDelay;                                                  // golden :5364 (file scope, external linkage as golden)
//---------------------------------------------------------------------------
void TfLotInfo::Timer1Timer()                                                   // golden :5366 (TObject *Sender dropped, unused)
{
    if(InitialOK==false)                                                        //Steven 20160912 : Add InitialOK in Timer
        return;

    AnsiString str;
    AnsiString strxxx;
    int &Task=iWaitRtcDeleteTask;
    switch(Task)
    {
        case wrdInitial:
            if(CUSTOMER_CODE==CC_ASE_KaohSiung)                                 //kevin 20211122 RTC FileName
            {                                                                   // [W906] braces: one golden statement became gate + log
#if 0 // GATE (W906-E020-LI6-1): TCOM2Shim has no sRealTimeCom_Send[] / rtFileOK (RTC vision half unported, atester_shims.h:366-462)
                COM2->sRealTimeCom_Send[COM2->rtFileOK] = str.sprintf("@FILE%s+" , TestIF_File.sRtcFileName);
#endif
                W906_E020_RtcSkip("W906-E020-LI6-1", "uLotInfo.cpp:5378 COM2->sRealTimeCom_Send[COM2->rtFileOK]=", str.sprintf("@FILE%s+" , TestIF_File.sRtcFileName));
            }
            else
            {
#if 0 // GATE (W906-E020-LI6-1): as above
                COM2->sRealTimeCom_Send[COM2->rtFileOK] = str.sprintf("@FILE%s+" , fMain->cbSetupFileName->Text);
#endif
                W906_E020_RtcSkip("W906-E020-LI6-1", "uLotInfo.cpp:5380 COM2->sRealTimeCom_Send[COM2->rtFileOK]=", str.sprintf("@FILE%s+" , fMain->cbSetupFileName->Text));
            }

            //AI(W906-E020-LI6) golden oddity kept: both arms of if(fSetup->fShow) build the same @SITE strings (:5382-5419).
            if(W906_FormShowing("fSetup", fSetup->fShow))                       // golden :5382 if(fSetup->fShow) (page-table rule, tools/fshow_audit.py)
            {
                if(TestIF_File.iTestMode==DualSite    ||
                   TestIF_File.iTestMode==SingleSite ||
                   TestIF_File.iTestMode==QualSite1X4 ||
                   TestIF_File.iTestMode==_8Site1X4                             //ChungHung 20150528 add for 海思 _8Site1x4
                   )                                                            //Steven 20120316 : 只分一條跟兩條?l
                {
                    strxxx.sprintf("@SITE2+");                                  // golden :5390 COM2->sRealTimeCom_Send[COM2->rtSite]   = strxxx.sprintf("@SITE2+");  (GATE LI6-2, logged below)
                }
                else if(bUseTwoArm32Site==true)
                {
                    strxxx.sprintf("@SITE10+");                                 // golden :5394 (GATE LI6-2)
                }
                else
                {
                    strxxx.sprintf("@SITE5+");                                  // golden :5398 (GATE LI6-2)
                }
            }
            else
            {
                if(TestIF_File.iTestMode==DualSite    ||
                   TestIF_File.iTestMode==SingleSite ||
                   TestIF_File.iTestMode==QualSite1X4 ||
                   TestIF_File.iTestMode==_8Site1X4                             //ChungHung 20150528 add for 海思 _8Site1x4
                   )                                                            //Steven 20120316 : 只分一條跟兩條?l
                {
                    strxxx.sprintf("@SITE2+");                                  // golden :5409 (GATE LI6-2)
                }
                else if(bUseTwoArm32Site==true)
                {
                    strxxx.sprintf("@SITE10+");                                 // golden :5413 (GATE LI6-2)
                }
                else
                {
                    strxxx.sprintf("@SITE5+");                                  // golden :5417 (GATE LI6-2)
                }
            }
#if 0 // GATE (W906-E020-LI6-2): TCOM2Shim has no sRealTimeCom_Send[] / rtSite -- golden assigns strxxx to it in each arm above
            COM2->sRealTimeCom_Send[COM2->rtSite]   = strxxx;
#endif
            W906_E020_RtcSkip("W906-E020-LI6-2", "uLotInfo.cpp:5390-5417 COM2->sRealTimeCom_Send[COM2->rtSite]=", strxxx);

#if 0 // GATE (W906-E020-LI6-3): TCOM2Shim has no SendCommToVision / rtFileOK / rtSite
            COM2->SendCommToVision(COM2->rtFileOK, true);
            COM2->SendCommToVision(COM2->rtSite,   true);
#endif
            W906_E020_RtcSkip("W906-E020-LI6-3", "uLotInfo.cpp:5421 COM2->SendCommToVision(COM2->rtFileOK, true)", str);
            W906_E020_RtcSkip("W906-E020-LI6-3", "uLotInfo.cpp:5422 COM2->SendCommToVision(COM2->rtSite, true)", strxxx);
            if(bNeedToDeleteFile)
                Task=wrdSendComm;
            else
                Task=wrdEndTask;
            break;
        case wrdSendComm:                                                       //需要砍檔案的話就要進來
#if 0 // GATE (W906-E020-LI6-4): TCOM2Shim has no SendCommToVision / rtDELETE
            COM2->SendCommToVision(COM2->rtDELETE, true);
#endif
            W906_E020_RtcSkip("W906-E020-LI6-4", "uLotInfo.cpp:5429 COM2->SendCommToVision(COM2->rtDELETE, true)", AnsiString());
            W906_E020_RtcSkip("W906-E020-LI6-5", "uLotInfo.cpp:5439 COM2->bRealTimeCom_ReceiveOK[COM2->rtDELETE] (no link: read as false; wrdWaitReply ends on the 10 s timeout :5434)", AnsiString());
            WaitRtcDeleteDelay.SetSecAndOn(10);
            Task=wrdWaitReply;
            break;
        case wrdWaitReply:
            if(WaitRtcDeleteDelay.Off())
            {
                Task=wrdEndTask;
            }

#if 0 // GATE (W906-E020-LI6-5): TCOM2Shim has no bRealTimeCom_ReceiveOK[] / rtDELETE -- no RTC link, so no reply (logged once in wrdSendComm)
            if(COM2->bRealTimeCom_ReceiveOK[COM2->rtDELETE]==true)
            {
                Task=wrdEndTask;
            }
#endif
            break;
        case wrdEndTask:
            //Steven 20110825 : 取full view image
#if 0 // GATE (W906-E020-LI6-6): TCOM2Shim has no SendCommToVision / rtMODEL / rtInspEnd
            COM2->SendCommToVision(COM2->rtMODEL, true);
            COM2->SendCommToVision(COM2->rtInspEnd, false);
#endif
            W906_E020_RtcSkip("W906-E020-LI6-6", "uLotInfo.cpp:5446 COM2->SendCommToVision(COM2->rtMODEL, true)", AnsiString());
            W906_E020_RtcSkip("W906-E020-LI6-6", "uLotInfo.cpp:5447 COM2->SendCommToVision(COM2->rtInspEnd, false)", AnsiString());
            bSendRealCCDSendStart=true;                                         //Ifor 20240919 add:避免RTC 變手動模式
            bRTCChangeFileFinish=true;                                          //ChungHung 20140514 fix in homeing and contact make time out
            Timer1->Enabled=false;
            break;
    }
}
//---------------------------------------------------------------------------
void TfLotInfo::RTCChangeFile(bool bNeedDelete)                                 // golden :5455 (default bNeedDelete=true: forms/fLotInfo.h:2333, golden uLotInfo.h)
{
    if(!COM2->bCCDDummyRum && InitialOK==true && bDoROILearning==false)
    {
        if(Timer1->Enabled==false)
        {
            iWaitRtcDeleteTask=1;
            bRTCChangeFileFinish=false;                                         //ChungHung 20140514 fix in homeing and contact make time out
            //jou 2012-03-01 [D31] RTC Change Recipe Need reCreate RTC Model
            if(IniConfig.bD31RTCChangeRecipeNeedreCreateModel==true)
            {
                bNeedToDeleteFile=bNeedDelete;
            }
            else
            {
                bNeedToDeleteFile=false;
            }
            Timer1->Enabled=true;
        }
    }
}
//---------------------------------------------------------------------------

// The OnTimer of golden's Timer1 (Interval=100 ms) for a future timer card: VCL runs OnTimer only while the timer is Enabled.
bool W906_TfLotInfo_Timer1Timer()
{
    if (fLotInfo == 0 || fLotInfo->Timer1 == 0 || fLotInfo->Timer1->Enabled == false)
        return false;
    fLotInfo->Timer1Timer();
    return true;
}

// ---------------------------------------------------------------------------------------------------------------------------
//  LI-11 golden V912 uLotInfo.cpp:10394-10418 TfLotInfo::btChangeFileClick -- branch choice as golden, every branch GATEd
// ---------------------------------------------------------------------------------------------------------------------------
int W906_E020_btChangeFileClick(std::string* whyNot)
{
    std::string dummy;
    std::string& why = whyNot ? *whyNot : dummy;
    why.clear();
    if(TestIF_File.bEnableBarCode)                                              // golden :10396
    {
        if(BAR_CODE_INSTALL==ebctInShtIntel)                                    // golden :10398
        {
#if 0 // GATE (W906-E020-LI11-1): fBarCode has no btBarcodeChangeFileDisConnect / btBarcodeChangeFileConnect (golden BarCode.cpp:6141-6160 ClientSocket_BarcodeChangeFile)
            fBarCode->btBarcodeChangeFileDisConnect->Click();                   //wei 20160728 Barcode File切換
            fBarCode->btBarcodeChangeFileConnect->Click();
#endif
            why = "GATE (W906-E020-LI11-1): golden uLotInfo.cpp:10400-10401 fBarCode->btBarcodeChangeFileDisConnect->Click(); "
                  "btBarcodeChangeFileConnect->Click(); -- fBarCode has neither button (V912 BarCode.cpp:6141-6160, the change-file "
                  "socket to the in-shuttle Intel reader) nor the reconnect chain behind it; nothing done";
            return 1;
        }
        else if(BAR_CODE_INSTALL==ebctEtherNetCCD)                              // golden :10403
        {
#if 0 // GATE (W906-E020-LI11-2): fBarCode has no InitialBarcodeScanChangeFile / TimerBarcodeChangeFile (DoBarcodeChangeFile unported)
            bBarcodeConnect=true;
            fBarCode->InitialBarcodeScanChangeFile();
            fBarCode->TimerBarcodeChangeFile->Enabled=true;
#endif
            why = "GATE (W906-E020-LI11-2): golden uLotInfo.cpp:10405-10407 bBarcodeConnect=true; fBarCode->InitialBarcodeScanChangeFile(); "
                  "fBarCode->TimerBarcodeChangeFile->Enabled=true; -- the Cognex EtherNet change-file chain (V912 BarCode.cpp:6217-6233 "
                  "TimerBarcodeChangeFileTimer -> DoBarcodeChangeFile :6233-~6700) is not ported; bBarcodeConnect not set alone; nothing done";
            return 2;
        }
        else if(BAR_CODE_INSTALL==ebctUseCCDMode &&                             // golden :10409
                CosFunction.b2DUseSubJobFunction==true &&
                TestIF_File.b2DUseSubJob==true)
        {
#if 0 // GATE (W906-E020-LI11-3): as LI11-2
            bBarcodeConnect=true;
            fBarCode->InitialBarcodeScanChangeFile();
            fBarCode->TimerBarcodeChangeFile->Enabled=true;
#endif
            why = "GATE (W906-E020-LI11-3): golden uLotInfo.cpp:10413-10415 (CCD mode + 2D sub job) bBarcodeConnect=true; "
                  "fBarCode->InitialBarcodeScanChangeFile(); fBarCode->TimerBarcodeChangeFile->Enabled=true; -- the change-file chain "
                  "(V912 BarCode.cpp:6217-~6700 DoBarcodeChangeFile) is not ported; bBarcodeConnect not set alone; nothing done";
            return 3;
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------------------------------------------------------
//  web: act.lotInfo.*
// ---------------------------------------------------------------------------------------------------------------------------
namespace {

std::string E020_Q(const std::string& s)
{
    std::string o = "\"";
    for (unsigned char c : s) {
        switch (c) {
        case '"':  o += "\\\""; break;
        case '\\': o += "\\\\"; break;
        case '\n': o += "\\n";  break;
        case '\r': o += "\\r";  break;
        case '\t': o += "\\t";  break;
        default:
            if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", (unsigned)c); o += b; }
            else o += (char)c;
        }
    }
    return o + "\"";
}
std::string E020_B(bool b) { return b ? "true" : "false"; }

std::string E020_Refuse(const std::string& op, const char* guard, const char* golden, const std::string& detail)
{
    std::printf("[E020] act.lotInfo.%s refused: %s -- %s\n", op.c_str(), guard, detail.c_str());
    std::fflush(stdout);
    return "{\"executed\":false,\"op\":" + E020_Q(op) + ",\"guard\":" + E020_Q(guard) + ",\"goldenLine\":" + E020_Q(golden) +
           ",\"detail\":" + E020_Q(detail) + "}";
}

const char* const kE020ChangeFileGolden = "V912 uLotInfo.cpp:10394-10418 btChangeFileClick (dfm :4641-4648)";
const char* const kE020PalGolden = "V912 uLotInfo.cpp:10532-10606 palSecsGemMouseDown (dfm palSecsGem :314-323)";

std::string E020_ChangeFile(TfLotInfo* f)
{
    const std::string op = "changeFile";
    f->W906_RefreshTabVisible();                                                // golden FormShow :571-572 (forms/fLotInfo.cpp:6134-6135)
    if (!f->tsBarCode->TabVisible)
        return E020_Refuse(op, "tab-hidden", "V912 uLotInfo.cpp:571 tsBarCode->TabVisible",
                           "golden shows the BarCode tab only for BAR_CODE_INSTALL = ebctUseCCDMode / ebctInShtIntel / ebctEtherNetCCD: the operator cannot reach Change File");
    if (!f->btChangeFile->Visible)
        return E020_Refuse(op, "button-hidden", "V912 uLotInfo.cpp:572 btChangeFile->Visible",
                           "golden hides Change File unless ebctInShtIntel / ebctEtherNetCCD / (ebctUseCCDMode and CosFunction.b2DUseSubJobFunction)");
    std::string why;
    const int branch = W906_E020_btChangeFileClick(&why);
    if (branch == 0) {
        std::printf("[E020] act.lotInfo.changeFile: golden takes no branch (bEnableBarCode=%d BAR_CODE_INSTALL=%d) -- nothing to do, as golden\n",
                    TestIF_File.bEnableBarCode ? 1 : 0, BAR_CODE_INSTALL);
        std::fflush(stdout);
        return "{\"executed\":true,\"op\":\"changeFile\",\"branch\":0,\"goldenLine\":" + E020_Q(kE020ChangeFileGolden) +
               ",\"result\":" + E020_Q(TestIF_File.bEnableBarCode
                                       ? "golden takes no branch (no matching BAR_CODE_INSTALL case, :10398-10411): nothing done"
                                       : "TestIF_File.bEnableBarCode is off (:10396): golden does nothing") + "}";
    }
    std::printf("[E020] act.lotInfo.changeFile: branch %d -- %s\n", branch, why.c_str());
    std::fflush(stdout);
    char gate[32];
    std::snprintf(gate, sizeof(gate), "W906-E020-LI11-%d", branch);
    return "{\"executed\":false,\"op\":\"changeFile\",\"guard\":\"gated\",\"branch\":" + std::to_string(branch) +
           ",\"gate\":" + E020_Q(gate) + ",\"goldenLine\":" + E020_Q(kE020ChangeFileGolden) + ",\"whyNot\":" + E020_Q(why) + "}";
}

std::string E020_PalSecsGem(TfLotInfo* f, const std::string& payloadJson)
{
    const std::string op = "palSecsGemMouseDown";
    cJSON* root = cJSON_Parse(payloadJson.empty() ? "{}" : payloadJson.c_str());
    if (root == 0 || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        return E020_Refuse(op, "bad-payload", kE020PalGolden, "value must be a JSON object string {\"button\":\"left|right|middle\"}");
    }
    const cJSON* jb = cJSON_GetObjectItemCaseSensitive(root, "button");
    const std::string b = (jb && cJSON_IsString(jb)) ? jb->valuestring : "";
    const cJSON* js = cJSON_GetObjectItemCaseSensitive(root, "seq");
    const bool hasSeq = js && cJSON_IsNumber(js);
    const double seq = hasSeq ? js->valuedouble : 0.0;
    cJSON_Delete(root);
    TMouseButton Button;
    if (b == "left") Button = mbLeft;
    else if (b == "right") Button = mbRight;
    else if (b == "middle") Button = mbMiddle;
    else return E020_Refuse(op, "bad-payload", kE020PalGolden, "button must be \"left\", \"right\" or \"middle\" (got \"" + b + "\")");

    f->W906_RefreshTabVisible();                                                // tsLotID (golden hides it only for CC_PANTHER :392 / CC_GIGAS :1035)
    if (!f->tsLotID->TabVisible)
        return E020_Refuse(op, "tab-hidden", "V912 uLotInfo.cpp:392 / :1035 tsLotID->TabVisible",
                           "golden hides the Lot tab on this machine (CC_PANTHER / CC_GIGAS): the operator cannot reach palSecsGem");
    if (!f->W906_LotEndPanelVisible())
        return E020_Refuse(op, "panel-hidden", "V912 uLotInfo.cpp:367-371 / :407-412 palSecsGem->Visible",
                           "CC_MTI without SECS/GEM or ATP lock: golden hides palSecsGem");

    // Read-only preview of golden's own early return (:10537-10540); golden itself decides inside the body.
    const char* ret = SystemStart ? "SystemStart" : (AccessLevel < iDefHonPrecLevel ? "AccessLevel<iDefHonPrecLevel"
                                                                                       : (CUSTOMER_CODE == CC_KYEC_LEE ? "CC_KYEC_LEE" : ""));
    const std::string lot0 = f->edtSysLotID->Text.c_str(), opr0 = f->edtSysOperatorID->Text.c_str();
    f->palSecsGemMouseDown(nullptr, Button, TShiftState(), 0, 0);               // golden body: forms/fLotInfo.cpp:4794 (jimmychiu)
    const std::string lot1 = f->edtSysLotID->Text.c_str(), opr1 = f->edtSysOperatorID->Text.c_str();
    const bool filled = lot1 != lot0 || opr1 != opr0;
    std::printf("[E020] act.lotInfo.palSecsGemMouseDown %s (seq %.0f)%s%s%s\n", b.c_str(), seq,
                *ret ? " -- golden returns at :10537-10540: " : "", ret, filled ? " -- Lot ID / Operator ID filled (golden :10597-10601)" : "");
    std::fflush(stdout);
    std::string j = "{\"executed\":true,\"op\":\"palSecsGemMouseDown\",\"button\":" + E020_Q(b);
    if (hasSeq) { char s[32]; std::snprintf(s, sizeof(s), "%.0f", seq); j += ",\"seq\":" + std::string(s); }
    j += ",\"goldenLine\":" + E020_Q(kE020PalGolden) + ",\"goldenReturn\":" + E020_Q(ret) + ",\"filled\":" + E020_B(filled) +
         ",\"lotId\":" + E020_Q(lot1) + ",\"operatorId\":" + E020_Q(opr1) + "}";
    return j;
}

}  // namespace

namespace ht9045 {
namespace sjson {
std::string W906_LotInfoE020Act(const std::string& cmd, const std::string& payloadJson)
{
    const std::string op = cmd.size() > 12 ? cmd.substr(12) : std::string();   // after "act.lotInfo."
    if (op != "changeFile" && op != "palSecsGemMouseDown")
        return E020_Refuse(op, "unknown-action", "", "act.lotInfo.* has: changeFile, palSecsGemMouseDown");
    TfLotInfo* f = fLotInfo;
    if (f == 0)
        return E020_Refuse(op, "no-form", "", "fLotInfo is NULL");
    if (op == "changeFile") return E020_ChangeFile(f);
    return E020_PalSecsGem(f, payloadJson);
}
}  // namespace sjson
}  // namespace ht9045
