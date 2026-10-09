// ===========================================================================
//  tests/test_st02_w195_atk.cpp -- W-195 (3) ATK (MR-ATK-1 + 1b): golden 913 (= 912, RogerYang AI(ht9045-atk-amr-flow)
//  20260820-0825) CEID 288 x Auto1-3 at Final Lot End, the one "Final Track-Out" CEID 8 and the ATK SV / EC re-points.
//  AI(W906-W195) 20261009 (St02-E).  依 W-195 卡，S25 不適用（Steven 1009 13:2x）.
//
//  Memory only: no file is written (SetLotState's GPIB arm goes through the tester-seat test hooks, as test_setlotstate.cpp;
//  EventReport is the sim counter; the SV table is built in a local THGem).  argv[1] = the tree root, read only (part 5).
//
//    1. fMain->SetLotState(10) with ATK on (CC_AMKOR_Korea + USE_COVER_TRAYID=tCID_NFC + [A65] + SECS + lot started):
//       3x CEID 288 with each lane's port 1-3 / bundle ID / bin code (trailing ',' cut) / trays / units (W906_AtkCeid288Hook),
//       an unused lane sends empty values, no CEID 8, bATK_AMR_DoLotEndSent untouched, then the restore
//       (golden 913 main.cpp:15922-15954).
//    2. controls: CC_PTI (HT9050's customer), SECS off, lot not started -> nothing sent, no hook call.
//    3. W906_AtkFinalTrackOutCeid8: SV 38316 "Final Track-Out", one CEID 8, the flag set (golden 913 csystem.cpp:19315-19317).
//    4. AddSV under CC_AMKOR_Korea vs CC_PTI: SV 1102 / 38314 name and value follow the customer (golden 913
//       uHGemHT9045_SV.cpp:159-162 / :864-867), one registration each; SV 38316 "Track Out Type" for every customer (:869).
//    5. source pins: SetLotState's ATK arm, csystem.cpp DoTrayFeed (golden :8126-8137, inside IsATK_AMR) and MainProc
//       (:19315-19317), fLotInfo.cpp SetLotEnd (uLotInfo.cpp:2079-2083 live), EC 37007 one-line if/else (EC.cpp:1657-1660).
// ===========================================================================
#include "Automation/AtkAmr_St02.h"
#include "Automation/AGV_PortScan.h"
#include "forms/fMain.h"
#include "forms/fSCKART.h"
#include "forms/fAGV.h"
#include "forms/fTemp_Set.h"   // the W-195 rule: create fTemp_Set first (see main)
#include "SECSGEM/uHGemEquipment.h"
#include "SECSGEM/uHGemHT9045.h"
#include "SECSGEM/SecsSvEcRegistration.h"
#include "SECSGEM/SecsSvRead.h"
#include "SECSGEM/SecsEventReport.h"
#include "SECSGEM/SecsEventType.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "MessageDef.h"
#include "TesterComm/TesterWndSeat.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_fail = 0;
static int g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

// ---- the tester seat (as test_setlotstate.cpp) --------------------------------
static bool BridgeFound() { return true; }
static HWND BridgeWnd() { return reinterpret_cast<HWND>(1); }
static void SendToBridge(COPYDATASTRUCT*) {}

// ---- the CEID 288 hook ----------------------------------------------------------
struct Lane { int port; std::string bundle, bin; int trays, units; };
static std::vector<Lane> g_lanes;
static void Ceid288(int iPortNo, const char* sBundleID, const char* sBinCode, int iTrays, int iUnits)
{
    Lane l;
    l.port = iPortNo; l.bundle = sBundleID ? sBundleID : ""; l.bin = sBinCode ? sBinCode : ""; l.trays = iTrays; l.units = iUnits;
    g_lanes.push_back(l);
}
static bool LaneIs(size_t i, int port, const char* bundle, const char* bin, int trays, int units)
{
    if (i >= g_lanes.size()) return false;
    const Lane& l = g_lanes[i];
    const bool ok = l.port == port && l.bundle == bundle && l.bin == bin && l.trays == trays && l.units == units;
    if (!ok) std::printf("     lane %u: port %d bundle [%s] bin [%s] trays %d units %d\n", (unsigned)i, l.port, l.bundle.c_str(), l.bin.c_str(), l.trays, l.units);
    return ok;
}

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// the lines of a file that the compiler sees (outside `#if 0`; a lifted gate is a `//#if 0` comment), as catalogue's ScanSvSource
static std::vector<std::string> LiveLines(const std::string& text)
{
    std::vector<std::string> out;
    std::vector<bool> dead;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') line.erase(line.size() - 1);
        size_t p = line.find_first_not_of(" \t");
        if (p != std::string::npos && line[p] == '#') {
            size_t q = line.find_first_not_of(" \t", p + 1);
            const std::string d = q == std::string::npos ? "" : line.substr(q);
            if (d.compare(0, 2, "if") == 0) {
                const bool plainIf = d.size() > 2 && (d[2] == ' ' || d[2] == '\t');
                size_t r = d.find_first_not_of(" \t", 2);
                dead.push_back(plainIf && r != std::string::npos && d[r] == '0' &&
                               (r + 1 == d.size() || d[r + 1] == ' ' || d[r + 1] == '\t' || d[r + 1] == '/'));
            } else if (d.compare(0, 5, "endif") == 0) { if (!dead.empty()) dead.pop_back(); }
            else if (d.compare(0, 4, "else") == 0) { if (!dead.empty()) dead.back() = !dead.back(); }
            continue;
        }
        bool inDead = false;
        for (size_t i = 0; i < dead.size(); ++i) inDead = inDead || dead[i];
        if (!inDead) out.push_back(line);
    }
    return out;
}

// index of the first live line containing `what` at or after `from` (code only: the text before any `//`), -1 when none
static int FindLive(const std::vector<std::string>& v, const char* what, int from = 0)
{
    for (int i = from < 0 ? 0 : from; i < (int)v.size(); ++i) {
        const size_t c = v[i].find("//");
        const std::string code = c == std::string::npos ? v[i] : v[i].substr(0, c);
        if (code.find(what) != std::string::npos) return i;
    }
    return -1;
}

static void SetAtk(bool on)
{
    CUSTOMER_CODE = on ? CC_AMKOR_Korea : CC_PTI;
    USE_COVER_TRAYID = on ? tCID_NFC : 0;
    IniConfig.bA65_BundleIDList = on;
}

static void SeedLanes(bool bAuto3Used)
{
    const char* bundles[3] = { "BUNDLE-A1", "BUNDLE-A2", "BUNDLE-A3" };
    const char* bins[3] = { "1,2,", "3", "4," };
    for (int i = eAuto1; i <= eAuto3; ++i) {
        Prod.iTrayType[i] = (i == eAuto3 && !bAuto3Used) ? tNotUse : tNotUse + 1;
        asBundleTrayID[i + ePortAuto1] = bundles[i - eAuto1];
        sSVBinAssign[i] = bins[i - eAuto1];
        LastSet.iUnloaderTrayCount_ART[i] = 5 + (i - eAuto1);
        LastSet.BinCT[0][e3Auto1 + i] = 100 * (1 + i - eAuto1);
    }
    LastSet.iLoaderTotalTray = 42;
    RunInfo.iUnloadCount = 999;
    iATKPortTrayCount = -1;
    iATKPortUnitCount = -1;
}

int main(int argc, char** argv)
{
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136): golden boot creates TfTemp_Set
    W906_TesterForward.BridgeFound = &BridgeFound;
    W906_TesterBridgeWndHook = &BridgeWnd;
    W906_TesterSendToBridgeHook = &SendToBridge;
    TestIF_File.iTestType = 0;                 // not TCP/IP: SetLotState's GPIB arm (fMain_SetLotState.cpp:78-84)
    TestIF.iTestType = GPIB_MODE;
    CosFunction.bAutoRetestGPIBmode = true;
    CosFunction.bUseSCKART = false; CosFunction.bGPIBLotEnd = false; CosFunction.iAutoRetestTCPmode = 0;
    CosFunction.bART_SECSGEM_93K = false; IniConfig.bA37LotStartLotEnd = false; IniConfig.bA10_AutoReTest = false;
    TestIF_File.bRENESAS_EnableFTCT = false;
    W906_AtkCeid288Hook = &Ceid288;

    // ---- 1. Final Lot End with ATK on ------------------------------------------
    SetAtk(true);
    check(fAGV->IsATK_AMR(), "precondition: CC_AMKOR_Korea + tCID_NFC + [A65] = IsATK_AMR()");
    IniConfig.bEnable_SECS_GEM = true; RunInfo.bLotStart = true;
    SeedLanes(false);
    fAGV->bATK_AMR_DoLotEndSent = false;
    g_lanes.clear(); ResetSimEventReport();
    fMain->SetLotState(10);
    check(fSCKART->iCurrent93KARTStep == 11, "final lot end (10): SCK ART step 11 as before (golden :15920)");
    check(g_SimEventReportCount == 3 && g_SimLastEventReportCeid == SECS_EVENT.MaximumOutputPortReport,
          "3 events, the last CEID 288 MaximumOutputPortReport (golden :15950); no CEID 8 any more");
    check(g_lanes.size() == 3, "the hook saw 3 lanes");
    check(LaneIs(0, 1, "BUNDLE-A1", "1,2", 5, 100), "Auto1: port 1, bundle, bin code without the trailing ',', trays, units (:15938-15947)");
    check(LaneIs(1, 2, "BUNDLE-A2", "3", 6, 200), "Auto2: port 2, bin code without a trailing ',' kept as is");
    check(LaneIs(2, 3, "", "", 0, 0), "Auto3 unused (tNotUse): empty bundle / bin and 0 / 0 are still sent (:15929-15935)");
    check(fAGV->bATK_AMR_DoLotEndSent == false, "SetLotState no longer sets bATK_AMR_DoLotEndSent (912/913 moved it to csystem.cpp)");
    check(iATKPortTrayCount == 42 && iATKPortUnitCount == 999, "after the loop: restored to LastSet.iLoaderTotalTray / RunInfo.iUnloadCount (:15952-15953)");
    check(iThisPortNo == 3 && sUnloadBundleID == "" && sOutputBinCode == "", "left behind: the last lane's port 3 and its empty values (golden)");

    SeedLanes(true);
    g_lanes.clear(); ResetSimEventReport();
    fMain->SetLotState(10);
    check(g_SimEventReportCount == 3 && LaneIs(2, 3, "BUNDLE-A3", "4", 7, 300),
          "Auto3 used: port 3 with its bundle, bin \"4,\" -> \"4\", 7 trays, 300 units");
    check(sUnloadBundleID == "BUNDLE-A3" && sOutputBinCode == "4" && iATKPortTrayCount == 42 && iATKPortUnitCount == 999,
          "left behind: Auto3's bundle / bin, the counts restored");

    // ---- 2. controls -----------------------------------------------------------
    SetAtk(false);
    SeedLanes(true);
    g_lanes.clear(); ResetSimEventReport();
    fMain->SetLotState(10);
    check(g_SimEventReportCount == 0 && g_lanes.empty() && iATKPortTrayCount == -1, "CC_PTI (HT9050's customer): nothing sent, nothing touched");
    SetAtk(true);
    IniConfig.bEnable_SECS_GEM = false;
    g_lanes.clear(); ResetSimEventReport();
    fMain->SetLotState(10);
    check(g_SimEventReportCount == 0 && g_lanes.empty(), "ATK with SECS off: nothing sent");
    IniConfig.bEnable_SECS_GEM = true; RunInfo.bLotStart = false;
    fMain->SetLotState(10);
    check(g_SimEventReportCount == 0 && g_lanes.empty(), "ATK with no lot started: nothing sent");
    RunInfo.bLotStart = true;

    // ---- 3. the one CEID 8 ---------------------------------------------------
    sTrackOutType_ATK = ""; fAGV->bATK_AMR_DoLotEndSent = false; ResetSimEventReport();
    W906_AtkFinalTrackOutCeid8();
    check(sTrackOutType_ATK == "Final Track-Out" && g_SimEventReportCount == 1 && g_SimLastEventReportCeid == SECS_EVENT.DoLotEnd &&
          fAGV->bATK_AMR_DoLotEndSent == true,
          "W906_AtkFinalTrackOutCeid8: SV 38316 \"Final Track-Out\", one CEID 8, the flag set (golden 913 csystem.cpp:19315-19317)");
    fAGV->bATK_AMR_DoLotEndSent = false;

    // ---- 4. the SV table follows the customer ----------------------------------
    const bool savedSecs = CosFunction.bEnable_SECS_GEM;
    THGem* const savedGem = HGem;
    for (int pass = 0; pass < 2; ++pass) {
        const bool atk = pass == 0;
        SetAtk(atk);
        THGem gem;
        HGem = &gem;
        CosFunction.bEnable_SECS_GEM = true;
        HT9045Gem h(AnsiString(""), &gem);
        { extern void W906_BootCreateTrayAssignment(); W906_BootCreateTrayAssignment(); }   // as SecsCatalogue (test_secs_catalogue.cpp)
        h.AddSV();
        TStringList* ids = gem.SvEcReg.SV_ID;
        int n1102 = 0, n38314 = 0, n38316 = 0;
        for (int i = 0; i < ids->Count; ++i) {
            const AnsiString s = ids->Strings[i];
            if (s == "1102") ++n1102;
            if (s == "38314") ++n38314;
            if (s == "38316") ++n38316;
        }
        check(n1102 == 1 && n38314 == 1 && n38316 == 1, atk ? "ATK: SV 1102 / 38314 / 38316 registered once each"
                                                            : "CC_PTI: SV 1102 / 38314 / 38316 registered once each");
        iATKPortUnitCount = 4321; RunInfo.iUnloadCount = 1234;
        sOutputBinCode = "7,8"; PC_NAME = "PC-W195";
        sTrackOutType_ATK = "Final Track-Out";
        const int i1102 = ids->IndexOf("1102"), i38314 = ids->IndexOf("38314"), i38316 = ids->IndexOf("38316");
        AnsiString v1102, v38314, v38316, why;
        const bool r1 = i1102 >= 0 && ht9045::SecsSvReadValue(gem.SvEcReg, i1102, v1102, why);
        const bool r2 = i38314 >= 0 && ht9045::SecsSvReadValue(gem.SvEcReg, i38314, v38314, why);
        const bool r3 = i38316 >= 0 && ht9045::SecsSvReadValue(gem.SvEcReg, i38316, v38316, why);
        const AnsiString n1 = i1102 >= 0 ? gem.SvEcReg.SV_NAME->Strings[i1102] : AnsiString("");
        const AnsiString n2 = i38314 >= 0 ? gem.SvEcReg.SV_NAME->Strings[i38314] : AnsiString("");
        const AnsiString n3 = i38316 >= 0 ? gem.SvEcReg.SV_NAME->Strings[i38316] : AnsiString("");
        std::printf("     %s: 1102 [%s] = %s | 38314 [%s] = %s | 38316 [%s] = %s\n", atk ? "ATK" : "PTI", n1.c_str(), v1102.c_str(),
                    n2.c_str(), v38314.c_str(), n3.c_str(), v38316.c_str());
        if (atk) {
            check(r1 && n1 == "Output Port Unit Count" && v1102 == "4321", "ATK: SV 1102 = iATKPortUnitCount \"Output Port Unit Count\" (golden 913 SV.cpp:160)");
            check(r2 && n2 == "Output BIN Code" && v38314 == "7,8", "ATK: SV 38314 = sOutputBinCode \"Output BIN Code\" (SV.cpp:865)");
        } else {
            check(r1 && n1 == "Output Total Count" && v1102 == "1234", "CC_PTI: SV 1102 = RunInfo.iUnloadCount \"Output Total Count\" as before (SV.cpp:162)");
            check(r2 && n2 == "PC_NAME" && v38314 == "PC-W195", "CC_PTI: SV 38314 = PC_NAME as before (SV.cpp:867)");
        }
        check(r3 && n3 == "Track Out Type" && v38316 == "Final Track-Out", "SV 38316 \"Track Out Type\" = sTrackOutType_ATK, every customer (SV.cpp:869)");
    }
    HGem = savedGem;
    CosFunction.bEnable_SECS_GEM = savedSecs;

    // ---- 5. source pins ------------------------------------------------------
    if (argc > 1) {
        const std::string src = argv[1];
        const std::vector<std::string> sl = LiveLines(Read(src + "/forms/fMain_SetLotState.cpp"));
        const int s10 = FindLive(sl, "else if(iState==10)");
        const int sAtk = FindLive(sl, "if(fAGV->IsATK_AMR()==true &&", s10);
        const int sCall = FindLive(sl, "W906_AtkFinalLotEndCeid288();", sAtk);
        check(s10 >= 0 && sAtk > s10 && sCall > sAtk && sCall - sAtk <= 3 && FindLive(sl, "EventReport(SECS_EVENT.DoLotEnd)") < 0 &&
              FindLive(sl, "bATK_AMR_DoLotEndSent=true") < 0,
              "fMain_SetLotState.cpp: the ATK arm calls W906_AtkFinalLotEndCeid288; no live CEID 8 / flag set left there");

        const std::vector<std::string> cs = LiveLines(Read(src + "/csystem.cpp"));
        int fFeed = -1;                                     // the definition (csystem.cpp also declares it at file scope, :7594)
        for (int i = 0; i < (int)cs.size() && fFeed < 0; ++i)
            if (cs[i] == "bool DoTrayFeed()") fFeed = i;
        const int fAtk = FindLive(cs, "if(fAGV->IsATK_AMR())", fFeed);
        const int fLine = FindLive(cs, "{ bool bSortingWrapUp=(LastSet.iUnloadFixTray==eAtkTfFeedFix); LastSet.iUnloadFixTray=eAtkTfNormalFeed; "
                                       "if(IniConfig.bEnable_SECS_GEM==true && RunInfo.bLotStart==true && bSortingWrapUp==false) W906_AtkFinalTrackOutCeid8(); }", fAtk);
        check(fFeed >= 0 && fAtk > fFeed && fLine > fAtk && fLine - fAtk < 30,
              "csystem.cpp DoTrayFeed: FeedFix read before the overwrite, then the one CEID 8 when no sorting (golden 913 :8126-8137)");
        const int mRestore = FindLive(cs, "W906_AtkFinalTrackOutCeid8();", fLine + 1);
        check(mRestore > fLine && cs[mRestore].find("//Send CEID 8 with correct data") != std::string::npos &&
              FindLive(cs, "EventReport(SECS_EVENT.DoLotEnd);") < 0,
              "csystem.cpp MainProc: the restore sends its CEID 8 through W906_AtkFinalTrackOutCeid8 (golden 913 :19315-19317); no bare DoLotEnd left");

        const std::vector<std::string> li = LiveLines(Read(src + "/forms/fLotInfo.cpp"));
        const int lGuard = FindLive(li, "if(fAGV->bATK_AMR_DoLotEndSent==false)");
        const int lMark = FindLive(li, "sTrackOutType_ATK=\"Final Track-Out\";", lGuard);
        const int lSend = FindLive(li, "EventReport(SECS_EVENT.DoLotEnd);", lMark);
        const int lReset = FindLive(li, "fAGV->bATK_AMR_DoLotEndSent=false;", lSend);
        check(lGuard >= 0 && lMark > lGuard && lSend > lMark && lReset > lSend && lReset - lGuard < 12,
              "fLotInfo.cpp SetLotEnd: skip when sent, mark \"Final Track-Out\" (ATK), send, reset the flag -- all live (golden 913 uLotInfo.cpp:2077-2083)");
        check(FindLive(li, "fAGV->bATK_AMR_DoHostLotStart=false;") < 0, "fLotInfo.cpp: the :2084-2085 member resets stay gated (TfAGV lacks them)");

        const std::vector<std::string> ec = LiveLines(Read(src + "/SECSGEM/uHGemHT9045_EC.cpp"));
        const int e7 = FindLive(ec, "if(CUSTOMER_CODE==CC_AMKOR_Korea) HGemPtr->SetECDataPointer(37007,");
        check(e7 >= 0 && ec[e7].find("&iATKPortTrayCount") != std::string::npos && ec[e7].find("else HGemPtr->SetECDataPointer(37007,") != std::string::npos &&
              ec[e7].find("&LastSet.iLoaderTotalTray") != std::string::npos && FindLive(ec, "SetECDataPointer(37007,", e7 + 1) < 0,
              "uHGemHT9045_EC.cpp: EC 37007 = iATKPortTrayCount for ATK, LastSet.iLoaderTotalTray otherwise, on one line (golden 913 :1657-1660)");
    } else
        check(argc > 1, "argv[1] = the tree root (part 5)");

    W906_AtkCeid288Hook = 0;
    std::printf("test_st02_w195_atk: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
