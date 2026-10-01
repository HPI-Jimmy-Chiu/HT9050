// AI(W906-PAUSE-FWD / W906-SECS-RSTART / W906-W7C1-INI) 20261001: batch 18 (RULINGS_20261001 #0 / #5).
//   Part 1 (behaviour): the base TfMain::Pause (forms/fMain.cpp:246) -- the one the ~40 state-machine sites reach through
//     fMain->Pause("...") -- forwards to the W906_RemoteRun seat (forms/fMain.h:1440-1447): not installed -> false and
//     nothing else; installed -> the seat gets the same Func and its answer comes back; the two counters still move on
//     every call (other tests count calls into this base).  In wb_serve the seat is TfMainWeb::PauseFromWeb = golden
//     TfMain::Pause (main.cpp:6325-6379, WebStart.cpp:3810).
//   Part 2 (the source, argv[1] = source root, // comments stripped because they quote golden on purpose):
//     SECSGEM/uHGemHT9045.cpp RCMD REMOTE_START calls W906_RemoteRunStart twice and fMain->Start no more (golden
//     :5811 / :5816 equivalents); csystem.cpp no longer carries the W7C1 INI seams (GetLastOpenFN / DataPath /
//     WriteIniData / fContact->ReadFile) nor the empty g4 AuthPath.
//   NOT COVERED: the real writes those seams hid (rest-mode Contact.Data restore -- unreachable until atester.cpp:4221's
//   [I49] raise is un-gated; [I06] config.ini at the end of DoHomeProcess; CC_ASE_KaohSiung's HandlerCondition.Data) --
//   the gate's sysguard over system / config / IniData is the check that no test reaches them.  Memory only.
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "forms/fMain.h"

static int g_fail = 0;
static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static int g_calls = 0;
static std::string g_func;
static bool StubPauseYes(AnsiString f) { ++g_calls; g_func = f.c_str(); return true; }
static bool StubPauseNo(AnsiString f) { ++g_calls; g_func = f.c_str(); return false; }

static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string Code(const std::string& text)          // the text without its // comments
{
    std::string code;
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
        const std::size_t c = line.find("//");
        code += (c == std::string::npos ? line : line.substr(0, c)) + "\n";
    }
    return code;
}

static int Count(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (std::size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size()))
        ++n;
    return n;
}

int main(int argc, char** argv)
{
    // ---- Part 1: the base TfMain::Pause forwards to the seat
    check(W906_RemoteRun.Pause == 0 && W906_RemoteRun.Start == 0, "the seat starts empty (only wb_serve installs it)");
    const int c0 = fMain->W906_PauseCallCount;
    check(fMain->Pause("NotInstalled") == false, "seat not installed -> Pause returns false (no pause), as before");
    check(fMain->W906_PauseCallCount == c0 + 1 && fMain->W906_PauseLastFunc == AnsiString("NotInstalled"),
          "the call counter and the last Func still record the call");

    W906_RemoteRun.Pause = &StubPauseYes;
    check(fMain->Pause("DoShakeShuttle") == true && g_calls == 1 && g_func == "DoShakeShuttle",
          "seat installed -> the same Func reaches it (golden ainarm2.cpp DoShakeShuttle site) and its true comes back");
    check(fMain->W906_PauseCallCount == c0 + 2 && fMain->W906_PauseLastFunc == AnsiString("DoShakeShuttle"),
          "the counter moves on a forwarded call too");
    TfMain* asBase = fMain;
    check(asBase->Pause("CheckOutArmSuckICFallDown") == true && g_calls == 2 && g_func == "CheckOutArmSuckICFallDown",
          "through a TfMain* (how the state machine calls it) -> forwarded");

    W906_RemoteRun.Pause = &StubPauseNo;
    check(fMain->Pause("RCMD:PAUSE") == false && g_calls == 3 && g_func == "RCMD:PAUSE",
          "the seat's own answer passes through unchanged (false here)");

    W906_RemoteRun.Pause = 0;
    check(fMain->Pause("Uninstalled") == false && g_calls == 3, "seat removed again -> false, the old seat is not called");
    check(W906_RemoteRun.Start == 0, "Pause never touches the Start seat");

    // ---- Part 2: the source
    const std::string src = argc > 1 ? argv[1] : ".";
    const std::string fm = Code(Read(src + "/forms/fMain.cpp"));
    const std::string gem = Code(Read(src + "/SECSGEM/uHGemHT9045.cpp"));
    const std::string cs = Code(Read(src + "/csystem.cpp"));
    check(!fm.empty() && fm.find("bool TfMain::Pause(AnsiString Func) { W906_PauseCallCount++; W906_PauseLastFunc = Func; return W906_RemoteRunPause(Func); }") != std::string::npos,
          "forms/fMain.cpp: the base Pause body is the forward");
    check(!gem.empty() && Count(gem, "fMain->Start(\"SECS GEM RCMD : REMOTE_START\")") == 0 &&
          Count(gem, "W906_RemoteRunStart(\"SECS GEM RCMD : REMOTE_START\");") == 2,
          "SECSGEM/uHGemHT9045.cpp: RCMD REMOTE_START calls W906_RemoteRunStart in both arms, the no-op fMain->Start in none");
    check(!cs.empty() && Count(cs, "W7C1_GetLastOpenFN") == 0 && Count(cs, "W7C1_DataPath_seam") == 0 &&
          Count(cs, "W7C1_WriteIniData") == 0 && Count(cs, "W7C1_FCONTACT_READFILE") == 0,
          "csystem.cpp: no W7C1 INI seam is left in the code (GetLastOpenFN / DataPath / WriteIniData / fContact->ReadFile)");
    check(cs.find("static AnsiString W906G4_AuthPath(){ return AuthPath; }") != std::string::npos,
          "csystem.cpp: the g4 AuthPath seam returns the real AuthPath");
    check(cs.find("fContactForm->ReadFile();") != std::string::npos,
          "csystem.cpp: the rest-mode restore re-reads the Contact form through fContactForm (golden fContact->ReadFile())");

    std::printf("%s PauseForward (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
