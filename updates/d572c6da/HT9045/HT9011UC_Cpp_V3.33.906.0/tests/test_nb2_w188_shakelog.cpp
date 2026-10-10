// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #12 -- DoShakeShuttle timeout log + ResetShakeShuttleTimeOut = golden 913 (GitLab
//  honprec/rd/rd5/ht9045_913 main e9908638; ht9045-v899 20260831): ainarm2.cpp:2362-2368 (ResetShakeShuttleTimeOut),
//  :2445-2448 (timeout message: str2 not str3; "fCanMoveM:%d" restored; "Shuttle:%d" added), ainarm2.h:190 (declaration),
//  ckernel.cpp:534-535 (called once START has really succeeded, so pause / alarm wall-clock time is not counted in the 10 s).
//  [1] ResetShakeShuttleTimeOut re-arms the 10 s timer only while iShakeShuttleTask==essSetFlag (Knock shares the timer).
//  [2] source ratchets: the message lines, the format has as many %d as arguments, the ckernel call sits after the safe-door check.
//  Use: only through ctest (NB2_W188ShakeLog).
// =============================================================================
#include "myTimer.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

extern TQPF_Timer ShakeShuttleDelay;      // ainarm2.cpp:102 (golden :2313)
extern int iShakeShuttleTask;             // ainarm2.cpp:6579 (golden :2311)
void ResetShakeShuttleTimeOut();          // ainarm2.cpp EOF (golden 913 :2364)

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
static int Find(const std::vector<std::string>& L, const std::string& n)
{
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) return (int)i;
    return -1;
}
static int CountSub(const std::string& s, const std::string& n)
{
    int c = 0;
    for (size_t p = s.find(n); p != std::string::npos; p = s.find(n, p + n.size())) ++c;
    return c;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, 0, _IONBF, 0);
    if (!W906TestRequireCtestRedirects("NB2_W188ShakeLog"))
        return 2;
    std::printf("==== W-188 #12 DoShakeShuttle timeout log + ResetShakeShuttleTimeOut -> golden 913 ====\n");

    // ---------------------------------------------------------------- [1] ResetShakeShuttleTimeOut
    std::printf("-- [1] ResetShakeShuttleTimeOut --\n");
    const int essSetFlag = 1, essMoveRight = 2;                          // ainarm2.cpp enum eShakeShuttle (golden ainarm2.h:180)
    iShakeShuttleTask = essSetFlag;
    ShakeShuttleDelay.SetMSAndOn(1);
    ::Sleep(40);
    CHECK(ShakeShuttleDelay.Off(), "precondition: a 1 ms timer has expired (as after a long pause)");
    ResetShakeShuttleTimeOut();
    CHECK(!ShakeShuttleDelay.Off(), "waiting for the interlock (essSetFlag): START re-arms the 10 s timeout");

    iShakeShuttleTask = essMoveRight;
    ShakeShuttleDelay.SetMSAndOn(1);
    ::Sleep(40);
    ResetShakeShuttleTimeOut();
    CHECK(ShakeShuttleDelay.Off(), "any other step (the timer may be Knock's): left alone");
    iShakeShuttleTask = 0;

    // ---------------------------------------------------------------- [2] source ratchets
    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> A = ReadLines(root + "/ainarm2.cpp");
    const std::vector<std::string> K = ReadLines(root + "/ckernel.cpp");
    CHECK(Find(A, "                str2.sprintf(\"ShuttleShake:%d  Fix3Cylinder:%d\", bShuttleShake, bUseFix3CylinderActive);") >= 0,
          "the short text goes to str2 (shown by ShowMyMessage(str1, str2))");
    const int f = Find(A, "str3.sprintf(\"Shake Shuttle Time Out!!, Shuttle:%d, ");
    char msg[200];
    if (f >= 0 && f + 1 < (int)A.size()) {
        const std::string fmt = A[f].substr(0, A[f].find("\","));
        const std::string args = A[f + 1].substr(0, A[f + 1].find(");"));
        const int nPct = CountSub(fmt, "%d");
        const int nArg = CountSub(args, ",") + 1;
        std::snprintf(msg, sizeof msg, "log format: %d x %%d == %d arguments, starts with iShuttle+1, no bare ':d'", nPct, nArg);
        CHECK(nPct == 10 && nArg == 10 && args.find("iShuttle+1, bShuttleShake") != std::string::npos && fmt.find("fCanMoveM:d") == std::string::npos, msg);
    } else {
        CHECK(false, "913 log format line present");
    }
    CHECK(Find(A, "void ResetShakeShuttleTimeOut()") >= 0, "ResetShakeShuttleTimeOut defined");
    const int kc = Find(K, "{ extern void ResetShakeShuttleTimeOut(); ResetShakeShuttleTimeOut(); }   }");
    int ks = -1;
    for (int i = kc - 1; kc > 0 && i >= kc - 10; --i) if (K[i].find("if(CheckSafeDoorIsClosed()==false)") != std::string::npos) { ks = i; break; }
    std::snprintf(msg, sizeof msg, "ckernel: called after the START safe-door check, at the end of the START branch (door :%d, call :%d)", ks + 1, kc + 1);
    CHECK(kc > ks && ks > 0 && K[kc + 1].find("else if(SoftStop==true)") != std::string::npos, msg);

    std::printf("==== W-188 #12: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
