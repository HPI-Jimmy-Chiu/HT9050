// ===========================================================================
//  tests/test_st02_w205_citems.cpp -- W-205 (W-195 (2) general items of the 37 file-scope rows, golden 913).
//  AI(W906-W195) 20261009 (St02-E).  One part per item (one MR per item): L05 now; L03 / L10 are added by their MRs.
//
//  Memory only: the motors stay unconstructed (MOT[].Motor == NULL, so the golden NULL guard skips every card read), the EventLog
//  goes to the ctest log scratch (the W906_*_ROOT redirects of every ctest, re-checked below), argv[1] = the tree root (read only).
//
//    L05  golden 913 acarry.cpp:3484-3515 W906_ShtMoveTimeoutDiag_St02: the exact golden-format interlock snapshot row
//         ("ShtMoveTimeout Diag SHT<n> : ...") through the 2-arg MyDBIProcess adapter; source pins: in DoShtMoveTimeoutHandle the
//         call sits after the latch-dump MyDBIProcess and before the first ShowMyMessage (golden: "必須在 ShowMyMessage 之前"),
//         and the operator text is golden 913 :3520.
// ===========================================================================
#include "MachineDefine.h"
#include "acarry.h"
#include "aHotPlateSubstrate.h"     // W906_MyDBIProcess_LastS1 / _LastS2 / _Reset
#include "Motor/mymotor.h"
#include "cmydef.h"
#include "atester.h"
#include "common.h"
#include "forms/fTemp_Set.h"        // the W-195 rule: create fTemp_Set first (see main)

#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

extern bool IndexZCanMove[2];
extern int  iPickFromShuttle1Task, iPickFromShuttle2Task;
void W906_ShtMoveTimeoutDiag_St02(int iSelSHT);

static int g_fail = 0;
static int g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static bool Forbidden(const char* p)
{
    std::string s = p ? p : "";
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    if (s.find("\\machine_log_scratch") != std::string::npos) return false;   // AI(W906-W205) 20261009 (laptop, batch 146 integration): the ctest redirect counts as contained wherever the build dir lives -- the laptop's gate builds under D:\HT9045\.claude\worktrees\..., so the bare d:\ht9045 prefix test below refused to run (same containment rule as test_c10_hana_rms.cpp:254)
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}

static void PartL05(const char* src)
{
    std::printf("\n-- L05: Shuttle Move Timeout interlock snapshot --\n");
    check(MOT[MTestY1].Motor == NULL && MOT[MTestZ1].Motor == NULL && MOT[MTestZ2].Motor == NULL && MOT[MTestY2].Motor == NULL,
          "precondition: no motor objects (the golden NULL guard skips every card read)");
    const bool savZ0 = IndexZCanMove[0], savZ1 = IndexZCanMove[1];
    IndexZCanMove[0] = true; IndexZCanMove[1] = false;
    iTestTask = 1234; iTestHeadMotorTask = 567; iTestYTask = 89;
    fFrontNeedTest = true; fFrontNeedSuck = false; fFrontNeedSuckIC = false;
    fRearNeedSuck = true; fRearNeedSuckIC = false; fRearNeedDestroy = false;
    AutoSHT1Task = 11; AutoSHT2Task = 120; iPickFromShuttle1Task = 7; iPickFromShuttle2Task = 42;
    MOT[MInShuttle2].fCanMove = false; MOT[MInShuttle2].fCanMoveL = true; MOT[MInShuttle2].fCanMoveR = false; MOT[MInShuttle2].fCanMoveM = false;

    W906_MyDBIProcess_Reset();
    W906_ShtMoveTimeoutDiag_St02(1);
    const std::string want =
        "ShtMoveTimeout Diag SHT2 : CanMove=0 L=1 R=0 M=0 | IdxZCanMove=1/0 | "
        "FrontNeedTest=1 FrontNeedSuck=0 FrontNeedSuckIC=0 RearNeedSuck=1 RearNeedSuckIC=0 RearNeedDestroy=0 | "
        "TestTask=1234 HeadMotTask=567 TestYTask=89 SHT1Task=11 SHT2Task=120 PickSht1=7 PickSht2=42 | "
        "Y1 0/0 Y2 0/0 Z1 0/0 Z2 0/0 (cmd/enc)";
    const std::string got = W906_MyDBIProcess_LastS2.c_str();
    std::printf("     row: %s\n", got.c_str());
    check(std::string(W906_MyDBIProcess_LastS1.c_str()) == "Message" && got == want,
          "L05: one EventLog row \"Message\", exactly golden 913 :3503-3514's format with the seeded flags / tasks / positions");
    IndexZCanMove[0] = savZ0; IndexZCanMove[1] = savZ1;
    check(savZ0 == false && savZ1 == false, "L05: V906 IndexZCanMove is {false,false} (GATE W4G-1) -> a real row logs 0/0 (accepted, W-205)");

    if (src) {
        const std::string a = Read(std::string(src) + "/acarry.cpp");
        const size_t fn = a.find("\nvoid DoShtMoveTimeoutHandle(int iSelSHT)");
        const size_t end = a.find("\n}", fn == std::string::npos ? 0 : fn);
        const std::string body = (fn == std::string::npos || end == std::string::npos) ? "" : a.substr(fn, end - fn);
        const size_t dump = body.find("MyDBIProcess(\"Message\", sLog);");
        const size_t diag = body.find("{ void W906_ShtMoveTimeoutDiag_St02(int iSelSHT); W906_ShtMoveTimeoutDiag_St02(iSelSHT); }");
        const size_t msg = body.find("ShowMyMessage(");
        check(dump != std::string::npos && diag != std::string::npos && msg != std::string::npos && dump < diag && diag < msg,
              "L05 source: the snapshot call is after the latch dump and before the operator message (golden 913 :3482 < :3484-3515 < :3524)");
        check(body.find("\"Press OK to Home, then do STATE RECORD before START.\",") != std::string::npos &&
              body.find("then START to resume.\",") == std::string::npos,
              "L05 source: the operator text is golden 913 :3520");
    } else
        check(false, "argv[1] = the tree root");
}

int main(int argc, char** argv)
{
    const char* roots[] = { as9045LogPath.c_str(), asSaveEventLogPath.c_str() };
    for (size_t i = 0; i < sizeof(roots) / sizeof(roots[0]); ++i)
        if (Forbidden(roots[i])) {
            std::printf("STOP: log root %s is a machine path -- the ctest W906_*_ROOT redirect is missing; nothing called\n", roots[i]);
            return 1;
        }
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136)
    GetTimeInfo();

    PartL05(argc > 1 ? argv[1] : 0);

    std::printf("test_st02_w205_citems: %d/%d passed\n", g_total - g_fail, g_total);
    return g_fail == 0 ? 0 : 1;
}
