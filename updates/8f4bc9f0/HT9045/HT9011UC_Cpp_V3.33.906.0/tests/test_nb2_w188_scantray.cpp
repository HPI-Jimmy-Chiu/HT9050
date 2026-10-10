// =============================================================================
//  AI(W906-W188) 20261010 (NB2-1): W-188 #7 -- ScanTrayStatus 46LA Fix manual-hold = golden 913 (GitLab honprec/rd/rd5/ht9045_913
//  main e9908638; RogerYang 20260722) csystem.cpp :16901-16903 (statics), :16917-16930 (hold block), :16936-16943 (the whole Fix
//  zone is skipped; the 0618 Fix2-only cylinder skip is commented out).
//  On a Manual Button (Fix3K_UseCylinder46LA) machine: while SnFix3Lock is on or the full-place cylinder is out, and for 5 s after
//  release, ScanTrayStatus does not scan any Fix tray (no "new tray" init, no tray-count bump).
//  [1] behaviour: Fix trays present (sensor), fHasTray false -> with the manual button held no Fix tray is initialised; right after
//      release (inside the 5 s settle) still none; after 5 s the scan runs again and initialises them.  Non-46LA machines unaffected.
//  [2] source ratchets (argv[1] = source root).
//  Use: only through ctest (NB2_W188ScanTray).
// =============================================================================
#include "csystem.h"
#include "cmydef.h"
#include "mysensor.h"
#include "mycylin.h"
#include "Motor/mymotor.h"
#include "cprod.h"
#include "canary_support.h"
#include "w906_ctest_guard.h"
#include <windows.h>
#include <cstdio>
#include <fstream>
#include <string>
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
static int Count(const std::vector<std::string>& L, const std::string& n)
{
    int c = 0;
    for (size_t i = 0; i < L.size(); ++i) if (L[i].find(n) != std::string::npos) ++c;
    return c;
}

// Fix trays "present" (sensor reads not-off), fHasTray false, so a scan initialises them.
static void ArmFixTrays()
{
    for (int i = iFixMin; i <= iFixMax; i++) {
        int iFix = iAutoIndex[i];
        Sen[SnFixedTrayDetect[iFix]].Enable = false;      // IsOff() == false (disabled sensor reads "not off")
        MOT[iMMAuto[i]].fHasTray = false;
    }
}
static int CountInitialised()
{
    int n = 0;
    for (int i = iFixMin; i <= iFixMax; i++) if (MOT[iMMAuto[i]].fHasTray) ++n;
    return n;
}

int main(int argc, char** argv)
{
    extern AnsiString DataPath;
    const char* const rt[] = { "DataPath", DataPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("NB2_W188ScanTray", rt)) return 2;
    std::printf("==== W-188 #7 ScanTrayStatus 46LA manual-hold -> golden 913 ====\n");

    const int saveFix3 = FIX3_FULL_PLACE, saveHome = iHome;
    const bool saveP10 = IniConfig.bP10FixedTrayProposeTheInitialQuestion;
    iHome = 0;
    IniConfig.bP10FixedTrayProposeTheInitialQuestion = false;
    Sen[iSafeDoor[5]].Enable = false;                     // IsOff()==false on both doors would return early; keep door 5 "off"
    Sen[iSafeDoor[5]].Enable = true; Sen[iSafeDoor[5]].Type = 1;
    Cylinder[C_FixTray_FullPlace].Enable = false;         // OffSensor() == true -> only the manual button drives the hold
    const int nFix = iFixMax - iFixMin + 1;

    // ---------------------------------------------------------------- [0] baseline: the scan does initialise Fix trays
    std::printf("-- [0] baseline (non-46LA) --\n");
    FIX3_FULL_PLACE = 0;
    ArmFixTrays();
    ScanTrayStatus();
    char msg[160];
    std::snprintf(msg, sizeof msg, "non-46LA machine: the scan initialises the present Fix trays (%d/%d)", CountInitialised(), nFix);
    CHECK(CountInitialised() == nFix, msg);

    // ---------------------------------------------------------------- [1] 46LA manual hold
    std::printf("-- [1] 46LA manual button --\n");
    FIX3_FULL_PLACE = Fix3K_UseCylinder46LA;
    Sen[SnFix3Lock].Enable = true; Sen[SnFix3Lock].Type = 0;     // IsOn() == true in SIM (button held)
    ArmFixTrays();
    ScanTrayStatus();
    std::snprintf(msg, sizeof msg, "button held: no Fix tray scanned (%d initialised)", CountInitialised());
    CHECK(CountInitialised() == 0, msg);

    Sen[SnFix3Lock].Enable = false;                              // released
    ScanTrayStatus();
    std::snprintf(msg, sizeof msg, "released, inside the 5 s settle: still none (%d)", CountInitialised());
    CHECK(CountInitialised() == 0, msg);

    ::Sleep(5300);
    ScanTrayStatus();
    std::snprintf(msg, sizeof msg, "5 s after release: scan runs again (%d/%d)", CountInitialised(), nFix);
    CHECK(CountInitialised() == nFix, msg);

    FIX3_FULL_PLACE = saveFix3; iHome = saveHome;
    IniConfig.bP10FixedTrayProposeTheInitialQuestion = saveP10;

    // ---------------------------------------------------------------- [2] source ratchets
    std::printf("-- [2] source ratchets --\n");
    const std::string root = argc > 1 ? argv[1] : ".";
    const std::vector<std::string> C = ReadLines(root + "/csystem.cpp");
    CHECK(Count(C, "static bool bRunScanTray=false;   static bool bFixManualHold=false; static TQPF_Timer hFixManualSettle; bool bFixZoneManualMove=false;") == 1,
          "statics declared in code (not after the comment)");
    CHECK(Count(C, "{ bFixManualHold=true; hFixManualSettle.SetSecAndOn(5); } else if(bFixManualHold && hFixManualSettle.Off()) { bFixManualHold=false; } bFixZoneManualMove=bFixManualHold; }") == 1,
          "hold block: button / cylinder -> hold, 5 s settle after release");
    CHECK(Count(C, "        if(bFixZoneManualMove==true) continue;") == 1 && Count(C, "//           i==eFix2 &&") == 1,
          "whole Fix zone skipped; the 0618 Fix2-only cylinder skip is commented out as in 913");

    std::printf("==== W-188 #7: %d passed, %d failed ====\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
