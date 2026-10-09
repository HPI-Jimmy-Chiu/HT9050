// =============================================================================
//  AI(W906-W188) 20261009 (NB2-1): W-188 #2 -- DoInterFaceErrorStep is the golden body again, at golden 913
//  (GitLab honprec/rd/rd5/ht9045_913 main e9908638, atester.cpp:3890-4154; 0618 :3856-4101).
//  Before: the golden text sat in `#if 0` and the LIVE DoInterFaceErrorStep was a slim stub that always returned
//  false, so the two live callers (aTester_Front.cpp DoInterFaceErrorStep(TESTZ1UP) / aTester_Rear.cpp (TESTZ2UP))
//  never finished the interface-error / contact-over step.  Now:
//    G1  the golden-verbatim gate is `#if 1 // was: #if 0`; the slim stub is the `#else` arm (not compiled)
//    G2  golden 913 :3889 TQPF_Timer DoInterFaceErrorStepDiagTimer + the two Pattern #27 diagnostics
//        (:3923-3931 TESTZ1UP waits for Z2 safe / :3953-3961 TESTZ2UP waits for Z1 safe): one RecordProcess line
//        every 5 s while the other Z is not at its safe position -- log only
//    G3  golden 913 :3981 / :4097 WAR0354 -> WAR0357 (WAR0354 = "RTC Alarm Arm 1 NG"; WAR0357 text at cMyDB.cpp)
//    G4  ATC_InterfaceForm->SendHandler2DID(0, false) has no port -> `(void)0` on the same line (same gate as
//        aTester_Front.cpp [W7Ck4 GATE G5])
//  [1] behaviour (SIM): Task 1 with the other Z NOT at safe -> stays at Task 1, writes the 913 DiagP27 line once,
//      and not again inside the 5 s throttle; TESTZ2UP after the throttle writes the mirror line.  RecordProcess keeps
//      its 1st arg ("DoInterFaceErrorStep P27 Diag") in ExString -- the DiagP27 text itself is pinned by [2].
//  [2] source ratchets on atester.cpp (argv[1] = port root).
//  Use: only through ctest (NB2_W188InterfaceErr).
// =============================================================================
#include "atester.h"
#include "aHotPlateSubstrate.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "cmydef.h"
#include "cMyDB.h"
#include "w906_ctest_guard.h"
#include <cstdio>
#include <fstream>
#include <string>
#include <windows.h>                         // ::Sleep (one 5.3 s wait for the diagnostic throttle)
#include <vector>

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
static std::string Trim(const std::string& s)
{
    size_t a = s.find_first_not_of(" \t");
    return a == std::string::npos ? std::string() : s.substr(a);
}
static int CountIn(const std::vector<std::string>& L, int a, int b, const std::string& needle)
{
    int n = 0;
    for (int i = a; i <= b && i < (int)L.size(); ++i)
        if (i >= 0 && L[i].find(needle) != std::string::npos) ++n;
    return n;
}
static bool Has(const AnsiString& s, const char* needle) { return std::string(s.c_str()).find(needle) != std::string::npos; }

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188InterfaceErr", rt)) return 2;
    std::printf("==== W-188 #2 DoInterFaceErrorStep -> golden 913 ====\n");

    // ---------------------------------------------------------------- [1] behaviour
    std::printf("-- [1] Task 1, other Z not at safe: stays, writes the 913 DiagP27 line, throttled to 5 s --\n");
    const int saveTask = iDoInterFaceErrorStepTask;
    const int saveZ1Safe = Prod.TestZ1_Safe, saveZ2Safe = Prod.TestZ2_Safe;
    const AnsiString saveEx = ExString;
    Prod.TestZ2_Safe = MOT[MTestZ2].Gali_ReadPos() + 12345;              // Z2 is NOT at its safe position
    Prod.TestZ1_Safe = MOT[MTestZ1].Gali_ReadPos() + 12345;              // Z1 is NOT at its safe position

    iDoInterFaceErrorStepTask = 1;
    ExString = "";
    const bool r1 = DoInterFaceErrorStep(TESTZ1UP);
    CHECK(!r1 && iDoInterFaceErrorStepTask == 1, "[1] TESTZ1UP, Z2 not safe -> not finished, still Task 1 (golden waits)");
    CHECK(Has(ExString, "DoInterFaceErrorStep P27 Diag"), "[1] golden 913 :3923-3931 diagnostic written (RecordProcess tag; the DiagP27 text is its 2nd arg)");
    std::printf("     (%s)\n", ExString.c_str());
    ExString = "";
    DoInterFaceErrorStep(TESTZ1UP);
    CHECK(ExString.Length() == 0, "[1] second call inside 5 s -> no new diagnostic (DoInterFaceErrorStepDiagTimer throttle)");

    ::Sleep(5300);
    iDoInterFaceErrorStepTask = 1;
    ExString = "";
    DoInterFaceErrorStep(TESTZ2UP);
    CHECK(iDoInterFaceErrorStepTask == 1 && Has(ExString, "DoInterFaceErrorStep P27 Diag"),
          "[1] TESTZ2UP, Z1 not safe, after 5 s -> golden 913 :3953-3961 mirror line");

    Prod.TestZ1_Safe = saveZ1Safe; Prod.TestZ2_Safe = saveZ2Safe;
    iDoInterFaceErrorStepTask = saveTask;
    ExString = saveEx;

    // ---------------------------------------------------------------- [2] source
    std::printf("-- [2] source ratchets (atester.cpp) --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> L = ReadLines(root + "/atester.cpp");
    CHECK(!L.empty(), "[2] atester.cpp readable");
    int g = -1, el = -1, en = -1;
    for (int i = 0; i < (int)L.size(); ++i) {
        if (g < 0 && L[i].compare(0, 21, "#if 1 // was: #if 0 -") == 0 && L[i].find("W-188 #2") != std::string::npos) g = i;
        if (g >= 0 && el < 0 && L[i].compare(0, 6, "#else ") == 0 && L[i].find("W-188 #2") != std::string::npos) el = i;
        if (el >= 0 && en < 0 && L[i].compare(0, 7, "#endif ") == 0 && L[i].find("W-188 #2") != std::string::npos) en = i;
    }
    CHECK(g >= 0 && L[g + 1].find("bool DoInterFaceErrorStep(int ZAxisSelect)") != std::string::npos &&
          L[g + 1].find("TQPF_Timer DoInterFaceErrorStepDiagTimer;") == 0,
          "[2] G1/G2: gate opened; golden 913 :3889 timer declared on the signature line");
    CHECK(el > g && en > el && CountIn(L, el, en, "bool DoInterFaceErrorStep(int ZAxisSelect)") == 1 &&
          CountIn(L, el, en, "return false;") == 1,
          "[2] G1: the old slim stub sits only in the #else arm");
    CHECK(CountIn(L, g, el, "} else if(DoInterFaceErrorStepDiagTimer.Off()) { void W906_InterFaceErrDiag913(int); W906_InterFaceErrDiag913(1); }") == 1 &&
          CountIn(L, g, el, "} else if(DoInterFaceErrorStepDiagTimer.Off()) { void W906_InterFaceErrDiag913(int); W906_InterFaceErrDiag913(2); }") == 1,
          "[2] G2: both Pattern #27 diagnostic branches");
    CHECK(CountIn(L, g, el, "ShowErrorMessage(\"WAR0357\", K_RETRY, MTestY1, false, sErrMsg);") == 2 &&
          CountIn(L, g, el, "\"WAR0354\"") == 0,
          "[2] G3: WAR0357 at both golden 913 sites, no WAR0354 left in DoInterFaceErrorStep");
    CHECK(CountIn(L, g, el, "GATE(W906-W188-ATC2DID)") == 1, "[2] G4: ATC 2DID send gated on its own line");
    int h = -1;
    for (int i = 0; i < (int)L.size(); ++i) if (Trim(L[i]) == "void W906_InterFaceErrDiag913(int which)") h = i;
    CHECK(h > en && CountIn(L, h, h + 25, "DoInterFaceErrorStepDiagTimer.SetSecAndOn(5);") == 2 &&
          CountIn(L, h, h + 25, "RecordProcess(\"DoInterFaceErrorStep P27 Diag\", str);") == 2,
          "[2] G2 helper: golden 913 diagnostic bodies (5 s throttle, RecordProcess)");

    std::printf("==== W-188 #2: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
