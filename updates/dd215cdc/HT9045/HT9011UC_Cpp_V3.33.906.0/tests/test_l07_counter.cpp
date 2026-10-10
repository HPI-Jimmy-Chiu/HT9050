// =============================================================================
//  test_l07_counter.cpp -- AI(W906-L07) 20261010 (Ifor01)
//
//  L07 step 3 (W-190 5; TO_IFOR 1010 11:2x B): golden 913 JerryYang 20260917 iATCRemoteChangeTempCnt ("遠端測試中變溫次數"),
//  the five writes St02-M checked on 1010 11:5x (FROM_STEVEN §3).  Its reader is POOL-13's ShowNewATCThermo / SetATCOffset (St02-E).
//    [1] DoCheckHasTestTempChange (atester.cpp; golden 913 :11951) clears the counter -- runs for real (its body is otherwise gated)
//    [2] source pins (argv[1] = port root, read only; code before // only): ++ twice in TfMain::WriteSetTestTempStatus (golden 913
//        Command.cpp:1748 temperature-changed branch, :1805 offset==0 branch before the gated SetAllTemp), =0 in GetTesterResult case 1
//        (atester.cpp:882) and in ProcessMotorHome after bDoingF16=false (uhome.cpp:1454)
// =============================================================================
#include "MachineDefine.h"
#include "cmydef.h"
#include "atester.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const char* msg) { if (ok) { std::printf("  PASS: %s\n", msg); ++g_pass; } else { std::printf("  FAIL: %s\n", msg); ++g_fail; } }
static std::string Slurp(const std::string& p) { std::ifstream f(p.c_str(), std::ios::binary); std::stringstream s; s << f.rdbuf(); return s.str(); }
// the code part (before //) of the lines between `head` and the next line that is exactly "}"
static std::string FuncCode(const std::string& src, const std::string& head)
{
    const size_t s = src.find(head);
    if (s == std::string::npos) return std::string();
    std::string out;
    std::istringstream in(src.substr(s));
    for (std::string l; std::getline(in, l); ) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        const size_t c = l.find("//");
        out += (c == std::string::npos ? l : l.substr(0, c)) + "\n";
        if (l == "}") break;
    }
    return out;
}
static int Count(const std::string& s, const std::string& k) { int n = 0; for (size_t p = s.find(k); p != std::string::npos; p = s.find(k, p + 1)) n++; return n; }

int main(int argc, char** argv)
{
    std::printf("L07_AtcRemoteChangeCount\n");
    iATCRemoteChangeTempCnt = 5;
    DoCheckHasTestTempChange();
    Check(iATCRemoteChangeTempCnt == 0, "1. DoCheckHasTestTempChange clears the count (golden 913 atester.cpp:11951)");

    if (argc > 1) {
        const std::string root = std::string(argv[1]) + "/";
        const std::string ws = FuncCode(Slurp(root + "Command.cpp"), "void TfMain::WriteSetTestTempStatus()");
        Check(Count(ws, "iATCRemoteChangeTempCnt++;") == 2 && ws.find("bChangeTest_TempAlarm=true;  iATCRemoteChangeTempCnt++;") != std::string::npos,
              "2a. WriteSetTestTempStatus counts twice: with bChangeTest_TempAlarm=true (golden 913 Command.cpp:1748) and in the else branch (:1805)");
        const std::string gt = FuncCode(Slurp(root + "atester.cpp"), "bool GetTesterResult(int Type)");
        Check(gt.find("bLBBoostTimeOut=false;  iATCRemoteChangeTempCnt=0;") != std::string::npos, "2b. GetTesterResult case 1 clears it (golden 913 atester.cpp:882)");
        const std::string uh = Slurp(root + "uhome.cpp");
        const size_t f16 = uh.find("            bDoingF16=false;"), nl = uh.find('\n', f16);
        const size_t z = uh.find("iATCRemoteChangeTempCnt=0;", nl);
        Check(f16 != std::string::npos && z != std::string::npos && uh.find('\n', nl + 1) > z,
              "2c. ProcessMotorHome clears it on the line after bDoingF16=false (golden 913 uhome.cpp:1454) -- code, not inside the // comment");
    } else std::printf("  (no argv[1]: source pins skipped)\n");

    std::printf("L07_AtcRemoteChangeCount: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
