// =============================================================================
//  test_st02_etm011_timer6.cpp -- E-TM-011 (POOL-3): golden TfMain::Timer6Timer, the SECS/GEM run-check wait after a remote
//  START (golden 913 main.cpp:32478-32727 = 0618 :31299-31548), bound to the timer table entry fMain->Timer6.
//
//  AI(W906-ETM011) 20261008 (St02-E).  Suite name (add_test): St02_Timer6RunCheck.  The entry's OnTimer is called directly (one
//  call = one 1000 ms tick), so no other timer runs and nothing sleeps.  Alarms are caught with W906_ShowErrorMessage_Hook;
//  RecordProcess writes only ctest's redirected roots (containment first).
//    [0] W906_St02TimersReset binds Timer6 (OnTimer set) and leaves it OFF -- golden main.dfm Timer6 Enabled = False.
//    [1] SECS on, RCMD on, PhysicalStart, N07 Run Check Alarm Time = 2: ticks 1-2 nothing; tick 3 -> WAR16110 (kcode 0,
//        "Main--Timer6"), PhysicalStart off, the key lock bSECSGEMAlarm off, Timer6 off (golden :32489-32500).
//    [2] the host answered first (SoftStart on, PhysicalStart cleared by the RCMD START, uHGemHT9045.cpp): next tick -> else,
//        the lock stays, no alarm, Timer6 off (golden :32714-32725).
//    [3] interrupted (PhysicalStart cleared, SoftStart / SystemStart off): unlock, Timer6 off, no alarm.
//    [4] InitialOK false: the tick does nothing (golden :32484-32485).
//    [5] (commit 2/2) WebStart.cpp: the openers golden 913 main.cpp:6469 / :6479 turn fMain->Timer6 on (two live lines -- fMain->
//        because StartFromWeb runs on wb_serve's own TfMainWeb); the N25 / N29 opener (:6504) stays MARKED.  argv[1] = port root.
//    [6] no blocking: Timer6 is tick-driven (one call, no wait loop) and its WAR16110 is a kcode-0 notice; wb_serve's
//        ForwardShowErrorMessage posts kcode 0 to the dialog mailbox and returns -- its AI(W906-Q30-KZERO) branch (tools/wb_serve.cpp,
//        `if (kcode == 0) {` -> DialogMailboxPostAlarm) comes before the wait loop.  Laptop condition (TO_STEVEN s4 1009 01:0x).
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "forms/fMain.h"
#include "st02_test_containment.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern int (*W906_ShowErrorMessage_Hook)(const char* Code, int KCode, int Pos);   // canary_support.h
extern AnsiString W906_ShowErrorMessage_LastErrPart;
namespace ht9045 {
void W906_St02TimersReset();                                   // MainTimersSt02.cpp (ctest only)
void W906_St02TimersCountsT6(unsigned long* t6, bool* on);    // MainTimersSt02.cpp (ctest only)
}

namespace {
int g_pass = 0, g_fail = 0;
void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
struct Shown { std::string code, part; int kcode; };
std::vector<Shown> g_shown;
int RecordShow(const char* code, int kcode, int)
{
    Shown s;
    s.code = code ? code : "";
    s.part = W906_ShowErrorMessage_LastErrPart.c_str();
    s.kcode = kcode;
    g_shown.push_back(s);
    return 0;
}
void Tick() { if (fMain->Timer6->OnTimer) fMain->Timer6->OnTimer(); }
bool On() { return (bool)fMain->Timer6->Enabled; }
std::string State()
{
    return std::string("PhysicalStart=") + (bPhysicalStart ? "1" : "0") + " lock=" + (bSECSGEMAlarm ? "1" : "0") + " Timer6=" + (On() ? "on" : "off") +
           " alarms=" + std::to_string(g_shown.size());
}
}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_Timer6RunCheck -- golden TfMain::Timer6Timer (913 main.cpp:32478-32727)\n");
    if (!W906TestInsideCtestRoots("St02_Timer6RunCheck"))
        return 2;
    if (fMain == 0) { std::printf("  ABORT: no fMain facade\n"); return 2; }

    // ---------------------------------------------------------------- [0]
    std::printf("[0] bound, off at start\n");
    ht9045::W906_St02TimersReset();
    unsigned long n0 = 0;
    bool on0 = true;
    ht9045::W906_St02TimersCountsT6(&n0, &on0);
    Check(fMain->Timer6->OnTimer != 0 && !on0 && !On(), "[0] fMain->Timer6 has an OnTimer and stays off after W906_St02TimersReset (golden main.dfm Enabled = False)");
    fMain->Timer6->Enabled = true;
    ht9045::W906_St02TimersReset();
    Check(!On(), "[0] a reset turns it off again even when it was on");

    W906_ShowErrorMessage_Hook = &RecordShow;
    InitialOK = true;
    SoftStart = false;
    SystemStart = false;
    IniConfig.bEnable_SECS_GEM = true;
    IniConfig.bRCMDStart = true;
    IniConfig.bN25_1_EnableStartControl = false;
    IniConfig.bN29_ParameterCheckForGMTest = false;
    IniConfig.iN07RunCheckAlarmTime = 2;

    // ---------------------------------------------------------------- [1]
    std::printf("[1] no answer from the host\n");
    bPhysicalStart = true;
    bSECSGEMAlarm = true;
    fMain->Timer6->Enabled = true;
    Tick();
    Tick();
    Check(bPhysicalStart && bSECSGEMAlarm && On() && g_shown.empty(), "[1] ticks 1-2: still waiting (" + State() + ")");
    Tick();
    const bool war = g_shown.size() == 1 && g_shown[0].code == "WAR16110" && g_shown[0].kcode == 0 && g_shown[0].part == "Main--Timer6";
    Check(war && !bPhysicalStart && !bSECSGEMAlarm && !On(), "[1] tick 3 (> 2): WAR16110 \"Main--Timer6\", PhysicalStart off, lock off, Timer6 off (" + State() + ")");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] the host answered in time\n");
    g_shown.clear();
    bPhysicalStart = true;
    bSECSGEMAlarm = true;
    fMain->Timer6->Enabled = true;
    Tick();                                   // count 1
    SoftStart = true;                         // RCMD START (uHGemHT9045.cpp) -> SoftStart, PhysicalStart cleared
    bPhysicalStart = false;
    Tick();
    Check(!On() && bSECSGEMAlarm && g_shown.empty(), "[2] else branch: Timer6 off, the lock left to the START, no alarm (" + State() + ")");
    Tick();
    Tick();
    bPhysicalStart = true;                    // count was reset by the else: a new wait starts from 0
    fMain->Timer6->Enabled = true;
    SoftStart = false;
    Tick();
    Tick();
    Check(g_shown.empty() && bPhysicalStart, "[2] a new wait starts from 0 (count reset by the else) (" + State() + ")");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] interrupted\n");
    g_shown.clear();
    bPhysicalStart = false;                   // e.g. Pause (WebStart.cpp PauseFromWeb clears it)
    bSECSGEMAlarm = true;
    SoftStart = false;
    SystemStart = false;
    fMain->Timer6->Enabled = true;
    Tick();
    Check(!bSECSGEMAlarm && !bPhysicalStart && !On() && g_shown.empty(), "[3] SoftStart / SystemStart off: lock off, PhysicalStart off, Timer6 off (" + State() + ")");

    // ---------------------------------------------------------------- [4]
    std::printf("[4] InitialOK false\n");
    InitialOK = false;
    bPhysicalStart = true;
    bSECSGEMAlarm = true;
    fMain->Timer6->Enabled = true;
    for (int i = 0; i < 5; ++i) Tick();
    Check(bPhysicalStart && bSECSGEMAlarm && On() && g_shown.empty(), "[4] nothing happens before InitialOK (" + State() + ")");

    // ---------------------------------------------------------------- [5]
    std::printf("[5] the openers in WebStart.cpp\n");
    {
        std::ifstream f((std::string(argc > 1 ? argv[1] : "") + "/WebStart.cpp").c_str(), std::ios::binary);
        std::string line;
        int live = 0, marked = 0;
        while (std::getline(f, line))
        {
            const size_t p = line.find_first_not_of(" \t");
            if (p == std::string::npos) continue;
            const std::string kLive = "fMain->Timer6->Enabled = true;", kMarked = "//MARKED(W906-ST-S3-B1) 20260918 Timer6->Enabled = true;";
            if (line.compare(p, kLive.size(), kLive) == 0) ++live;
            if (line.compare(p, kMarked.size(), kMarked) == 0 && line.find("golden :6217") != std::string::npos) ++marked;
        }
        Check(live == 2 && marked == 1, "[5] two live openers fMain->Timer6->Enabled = true (golden 913 :6469 / :6479), the N25/N29 one (:6504) still MARKED (live " +
                                            std::to_string(live) + ", marked " + std::to_string(marked) + ")");
    }

    // ---------------------------------------------------------------- [6]
    std::printf("[6] no blocking: kcode 0 notice, posted without the wait loop\n");
    {
        auto slurp = [](const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::ostringstream ss; ss << f.rdbuf(); return ss.str(); };
        const std::string root = argc > 1 ? argv[1] : "";
        const std::string t6 = slurp(root + "/MainTimer6_St02.cpp");
        Check(t6.find("ShowErrorMessage(\"WAR16110\", 0, MMSystem, false, \"Main--Timer6\");") != std::string::npos,
              "[6] MainTimer6_St02.cpp raises WAR16110 with kcode 0 (a notice, golden 913 main.cpp:32497)");
        const std::string ws = slurp(root + "/tools/wb_serve.cpp");
        const size_t kz = ws.find("AI(W906-Q30-KZERO)");
        const size_t br = kz == std::string::npos ? std::string::npos : ws.find("if (kcode == 0) {", kz);
        const size_t post = br == std::string::npos ? std::string::npos : ws.find("DialogMailboxPostAlarm(", br);
        Check(br != std::string::npos && post != std::string::npos && post - br < 200,
              "[6] tools/wb_serve.cpp: the kcode-0 branch posts to the dialog mailbox right away (W906-Q30-KZERO), no wait loop");
        InitialOK = true;
        IniConfig.bEnable_SECS_GEM = true;
        IniConfig.bRCMDStart = true;
        IniConfig.iN07RunCheckAlarmTime = 0;
        bPhysicalStart = true;
        W906_ShowErrorMessage_Hook = 0;       // the real ShowErrorMessage (no web forward in ctest): one call, it returns
        const unsigned long t0 = ::GetTickCount();
        Tick();
        const unsigned long dt = ::GetTickCount() - t0;
        Check(!bPhysicalStart && dt < 2000, "[6] the tick that raises WAR16110 returns at once (" + std::to_string(dt) + " ms)");
    }

    fMain->Timer6->Enabled = false;
    bPhysicalStart = false;
    bSECSGEMAlarm = false;
    IniConfig.bEnable_SECS_GEM = false;
    IniConfig.bRCMDStart = false;
    W906_ShowErrorMessage_Hook = 0;
    std::printf("St02_Timer6RunCheck: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
