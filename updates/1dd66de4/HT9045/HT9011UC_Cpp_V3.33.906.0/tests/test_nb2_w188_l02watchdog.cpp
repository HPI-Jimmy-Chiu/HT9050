// =============================================================================
//  AI(W906-W188) 20261009 (NB2-1): W-188 #4 L02 -- DoFrontTestSuckIC / DoRearTestSuckIC case 306 watchdog = golden 913
//  (GitLab honprec/rd/rd5/ht9045_913 main e9908638; RogerYang 20260908):
//    atester_front.cpp :932 TQPF_Timer hFrontPickErrWaitFR, :1747 SetSecAndOn(30) before Task=306,
//    :1755-1756 `if(FRCarryKit.UseSiteNoIC() || InSHT1InLF())`, :1765-1768 `else if(hFrontPickErrWaitFR.Off()) Task=320`
//    atester_rear.cpp  :952 / :1752 / :1760-1761 / :1770-1773 (BR, InSHT2InLF, hRearPickErrWaitBR).
//  Before: with [D43] on and FR/BR still holding tested IC, case 306 waited forever (silent hang).
//  Now: after 30 s it goes to case 320 with bShuttleXMoveToLeft false -> case 321 raises JAM0301/0302.
//  [1] behaviour (one call per step, cursor parked again afterwards):
//      FR has IC, shuttle not left, timer running -> stays 306; timer expired -> 320, bShuttle1MoveToLeft stays false;
//      FR empty -> 320 with bShuttle1MoveToLeft=true (0618 path unchanged).  Same for Rear.
//  [2] source ratchets on aTester_Front.cpp / aTester_Rear.cpp (argv[1] = source root).
//  Use: only through ctest (NB2_W188L02Watchdog).
// =============================================================================
#include "aTester_Front.h"
#include "aTester_Rear.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cpublic.h"
#include "cmydef.h"
#include "csystem.h"
#include "acarry.h"
#include "canary_support.h"
#include "FormsFacade.h"
#include "atester_shims.h"
#include "myTimer.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern TQPF_Timer hFrontPickErrWaitFR;
extern TQPF_Timer hRearPickErrWaitBR;

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::vector<std::string> ReadLines(const std::string& path)
{
    std::vector<std::string> v;
    std::ifstream f(path.c_str(), std::ios::binary);
    std::string s;
    while (std::getline(f, s)) {
        if (!s.empty() && s[s.size() - 1] == '\r') s.erase(s.size() - 1);
        v.push_back(s);
    }
    return v;
}
static int CountLines(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}
static int FindLine(const std::vector<std::string>& L, const std::string& n, int from = 0)
{
    for (int i = from; i < (int)L.size(); ++i) if (L[i].find(n) != std::string::npos) return i;
    return -1;
}

struct Side {
    const char* name;
    int* task;
    bool (*step)();
    TMyKitSuck* outKit;     // FR / BR
    bool* moveToLeft;       // bShuttle1MoveToLeft / bShuttle2MoveToLeft
    bool* shtLeftFlag;      // b1ShuttleMoveToLeft / b2ShuttleMoveToLeft (InSHTxInLF returns false when this is false)
    TQPF_Timer* wd;
};

static void RunSide(const Side& s)
{
    std::printf("-- [1] %s case 306 --\n", s.name);
    const bool saveD43 = IniConfig.bD43IndexDropErrorCanRetryandSkip;
    const bool saveMove = *s.moveToLeft, saveSht = *s.shtLeftFlag;
    const int saveTask = *s.task;
    IniConfig.bD43IndexDropErrorCanRetryandSkip = true;
    *s.shtLeftFlag = false;                 // shuttle not pulled left -> InSHTxInLF() false
    *s.moveToLeft = false;

    s.outKit->SetAll(HAS_IC);
    const bool pre = !s.outKit->UseSiteNoIC();
    CHECK(pre, "precondition: the out-shuttle kit holds tested IC (UseSiteNoIC false)");

    char msg[160];
    s.wd->SetSecAndOn(30);
    *s.task = 306;
    s.step();
    std::snprintf(msg, sizeof msg, "%s: kit still holds IC, watchdog running -> stays in 306 (Task=%d)", s.name, *s.task);
    CHECK(*s.task == 306, msg);

    s.wd->SetMSAndOn(1);
    ::Sleep(20);
    *s.task = 306;
    s.step();
    std::snprintf(msg, sizeof msg, "%s: 30 s watchdog expired -> 320 (Task=%d)", s.name, *s.task);
    CHECK(*s.task == 320, msg);
    CHECK(*s.moveToLeft == false, "watchdog path keeps bShuttleXMoveToLeft false (case 320 will not wait, case 321 raises JAM030x)");

    s.outKit->SetAllToNullIC();
    s.wd->SetSecAndOn(30);
    *s.task = 306;
    s.step();
    CHECK(*s.task == 320 && *s.moveToLeft == true, "kit empty -> 320 with bShuttleXMoveToLeft=true (0618 [D43] path unchanged)");

    *s.task = saveTask;
    *s.moveToLeft = saveMove; *s.shtLeftFlag = saveSht;
    IniConfig.bD43IndexDropErrorCanRetryandSkip = saveD43;
    s.outKit->SetAllToNullIC();
}

static void Ratchet(const std::string& root, const char* file, const char* tm, const char* kit, const char* lf, const char* delay)
{
    const std::vector<std::string> L = ReadLines(root + "/" + file);
    std::string decl = std::string("TQPF_Timer ") + tm + ";   TQPF_Timer " + delay + ";";
    std::string set  = std::string(tm) + ".SetSecAndOn(30);   Task=306;";
    std::string cond = std::string("if(") + kit + ".UseSiteNoIC() || " + lf + ")";
    std::string els  = std::string("} else if(") + tm + ".Off()) { Task=320; }";
    char msg[200];
    std::snprintf(msg, sizeof msg, "[2] %s: timer declared once on the %s line", file, delay);
    CHECK(CountLines(L, decl) == 1, msg);
    std::snprintf(msg, sizeof msg, "[2] %s: SetSecAndOn(30) right before the only Task=306", file);
    CHECK(CountLines(L, set) == 1 && CountLines(L, "Task=306;") == 1, msg);
    const int c = FindLine(L, cond), e = FindLine(L, els);
    std::snprintf(msg, sizeof msg, "[2] %s: case-306 condition has || %s and the watchdog else-if follows it", file, lf);
    CHECK(c > 0 && e > c && e - c <= 9 && FindLine(L, "case 306:") < c, msg);
}

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188L02Watchdog", rt)) return 2;
    std::printf("==== W-188 #4 L02 case-306 watchdog -> golden 913 ====\n");

    Side front = { "Front", &iFrontTestSuckICTask, DoFrontTestSuckIC, &FRCarryKit, &bShuttle1MoveToLeft, &b1ShuttleMoveToLeft, &hFrontPickErrWaitFR };
    Side rear  = { "Rear",  &iRearTestSuckICTask,  DoRearTestSuckIC,  &BRCarryKit, &bShuttle2MoveToLeft, &b2ShuttleMoveToLeft, &hRearPickErrWaitBR };
    RunSide(front);
    RunSide(rear);

    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    Ratchet(root, "aTester_Front.cpp", "hFrontPickErrWaitFR", "FRCarryKit", "InSHT1InLF()", "hDoFrontTestSuckICdelay");
    Ratchet(root, "aTester_Rear.cpp",  "hRearPickErrWaitBR",  "BRCarryKit", "InSHT2InLF()", "hDoRearTestSuckICdelay");

    std::printf("==== W-188 #4: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
