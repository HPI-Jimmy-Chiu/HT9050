// AI(W906-ATCEMG) 20261001: RULINGS_20261001 #5 (census 129 (a)(b) #17, safety gap) with its pair and its sibling: csystem.cpp's
//   gates G-ATC-B (EMG pressed -> golden :1442-1443 SendCommToATC7(ATC_EMG_DOWN)), G-ATC-C (EMG released -> :1470 ATC_EMG_UP)
//   and G-ATC-A (CheckATC6System, :1111-1129) are open through the bridges at the end of ATC/ATCInterface.cpp
//   (csystem.cpp cannot include ATC/ATCInterface.h -- the clWhite collision at its G-ATC block).
//   Part 1 calls the bridges: with no ATC client golden SendCommToATC7 writes its "No Client!" line into HandlerMemo -- one
//   line per edge; W906_CheckATC6System_Body does nothing on a machine without ATC 6.0 / 3.0 and returns before any socket
//   call on an ATC 6.0 machine whose SystemInitialOK is still false.
//   Part 2 reads csystem.cpp: each gate's live arm calls its bridge, the three bridges are called exactly once each, and the
//   golden statements that cannot compile there are no longer in a live arm.
//   NOT COVERED: IsEMGPressed's edge logic around the calls (golden static bServoOff; its pressed path dereferences
//   MOT[i].Motor of every motor, which a unit test has no objects for) and the ATC 6.0 socket calls (they open a real socket).
//   Memory only; no file is written, no socket is opened.
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "cmydef.h"
#include "ATC/ATCInterface.h"

void W906_ATC_SendEmgDown();
void W906_ATC_SendEmgUp();
void W906_CheckATC6System_Body();

static int g_fail = 0;

static void check(bool ok, const char* what)
{
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

static std::size_t CountOf(const std::string& s, const std::string& sub)
{
    std::size_t n = 0, p = 0;
    while ((p = s.find(sub, p)) != std::string::npos) { ++n; p += sub.size(); }
    return n;
}

static bool LastIsNoClient(TStringList* lines)
{
    if (lines->Count <= 0) return false;
    const AnsiString last = lines->Strings[lines->Count - 1];
    return std::string(last.c_str()).find("No Client!") != std::string::npos;
}

int main(int argc, char** argv)
{
    // ---- Part 1: the bridges ----
    TStringList* lines = ATCInterfaceForm->HandlerMemo->Lines;
    lines->Clear();
    W906_ATC_SendEmgDown();
    check(lines->Count == 1 && LastIsNoClient(lines),
          "EMG down bridge -> golden SendCommToATC7(ATC_EMG_DOWN): no ATC client -> one \"No Client!\" line (SendCommToATC7's no-client arm)");
    W906_ATC_SendEmgUp();
    check(lines->Count == 2 && LastIsNoClient(lines), "EMG up bridge -> golden SendCommToATC7(ATC_EMG_UP): one more line");

    const int oldAtc = ATC_SYSTEM;
    const bool oldInit = SystemInitialOK;
    ATC_SYSTEM = eATCUninstall;
    W906_CheckATC6System_Body();
    check(lines->Count == 2, "CheckATC6System on a machine without ATC 6.0 / 3.0 -> nothing (golden :1112-1113)");
    ATC_SYSTEM = eATC60;
    SystemInitialOK = false;
    W906_CheckATC6System_Body();
    check(lines->Count == 2, "ATC 6.0 but SystemInitialOK==false -> golden :1115-1116 returns before any socket call");
    ATC_SYSTEM = oldAtc;
    SystemInitialOK = oldInit;
    lines->Clear();

    // ---- Part 2: csystem.cpp's three gates call the bridges ----
    const std::string src = argc > 1 ? argv[1] : ".";
    std::ifstream f(src + "/csystem.cpp", std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string s = ss.str();
    check(!s.empty(), "csystem.cpp read (argv[1] = the source root)");
    check(CountOf(s, "W906_ATC_SendEmgDown(); }") == 1 && CountOf(s, "W906_ATC_SendEmgUp(); }") == 1 &&
          CountOf(s, "W906_CheckATC6System_Body(); }") == 1, "each bridge is called exactly once in csystem.cpp");
    const std::size_t gb = s.find("#if 1 // GATE G-ATC-B"), gc = s.find("#if 1 // GATE G-ATC-C");
    const std::size_t cb = s.find("W906_ATC_SendEmgDown(); }"), cc = s.find("W906_ATC_SendEmgUp(); }");
    check(gb != std::string::npos && cb != std::string::npos && gb < cb && s.find("#else", gb) > cb,
          "G-ATC-B is #if 1 and its live arm (before #else) calls W906_ATC_SendEmgDown");
    check(gc != std::string::npos && cc != std::string::npos && gc < cc && s.find("#else", gc) > cc,
          "G-ATC-C is #if 1 and its live arm (before #else) calls W906_ATC_SendEmgUp");
    const std::size_t ga = s.find("#if 0 // GATE G-ATC-A"), ca = s.find("W906_CheckATC6System_Body(); }");
    const std::size_t ea = ga == std::string::npos ? ga : s.find("#else", ga);
    const std::size_t xa = ea == std::string::npos ? ea : s.find("#endif", ea);
    check(ga != std::string::npos && ea != std::string::npos && xa != std::string::npos && ea < ca && ca < xa,
          "G-ATC-A keeps golden's text in #if 0 and its #else arm calls W906_CheckATC6System_Body");
    check(CountOf(s, "#if 0 // GATE G-ATC-B") == 0 && CountOf(s, "#if 0 // GATE G-ATC-C") == 0, "no G-ATC-B / C arm is still #if 0");

    std::printf("%s AtcEmgBridge (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
