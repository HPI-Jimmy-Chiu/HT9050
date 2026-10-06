// ===========================================================================
//  tests/test_comparetech_ht9050.cpp   (ctest: CompareTechHt9050)
//
//  AI(W906-FR-NB2-3) 20261006: RULINGS_20261005 #20 (Jimmy 1005 22:4x, s0 #121 = A; NB2-1 R235 / U30; laptop card 1006 03:4x).
//  cinitial.cpp CompareTechData() -- the START-time teach-value gate (golden cinitial.cpp:13615-13674) -- skips its three
//  "point 2 is 10 mm past point 1" checks (In Shuttle 1/2 Y, Out Shuttle 1/2 Y, Auto1/2 X) on an HT9050, dry run or not:
//  HT9050 = W906_GpibModel == "9050GPIB" || MachineTypeChoice == Type_HT9050 (the tree-wide test). Pinned here:
//    1. the machine's 10-04 teach values (InSht1Y 113317 / InSht2Y -5749, OutSht1Y 53278 / OutSht2Y -5621,
//       Auto1X 214512 / Auto2X -35078) pass on an HT9050 by either half of the test, with the dry run off and on
//    2. the same values on an HT9045 are refused exactly as golden, one check at a time (in shuttle -> out shuttle -> unloader)
//    3. the other four checks (hot plate, Fix1/2, Fix2/3, Auto2/3) still refuse on an HT9050
//    4. source pins (cinitial.cpp): the machine-type test, the AI marker, no dry-run test left in CompareTechData
//  Reverse check (done by hand for the MR, R236): drop the skip (`!w906Ht9050 &&` x3) -> sections 1 and 4 go red.
//  Memory only (Tech / globals); argv[1] = the port root, read only.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
#include "LastSet.h"
#include "cinitial.h"
#include "canary_support.h"
#include "Motor/mymotor.h"
#include "Ht9050DryRun.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

extern AnsiString W906_GpibModel;

static int g_fail = 0, g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, e); }
}
#define CHECK(c) check((c), #c, __LINE__)

static const char* kInSht   = "The teaching of in shuttle is mistake, please verify!";
static const char* kOutSht  = "The teaching of out shuttle is mistake, please verify!";
static const char* kUnload  = "The teaching of unloader tray is mistake, please verify!";
static const char* kHotPlt  = "The teaching of hot plate is mistake, please verify!";
static const char* kFix     = "The teaching of fix tray is mistake, please verify!";

static int HookAnswers(int) { return 1; }                                      // an HT9050 1203 axis: "at home" (dry run's own test)

static void MachineTeach()                                                     // the HT9050's 10-04 values for the three pairs
{
    Tech.iInArmPlate1Y   = 0;      Tech.iInArmPlate2Y   = 20000;                // the other four: valid (machine passed them in the dry run)
    Tech.iInArmShuttle1Y = 113317; Tech.iInArmShuttle2Y = -5749;
    Tech.iOutArmShuttle1Y = 53278; Tech.iOutArmShuttle2Y = -5621;
    Tech.iOutArmFix1X    = 0;      Tech.iOutArmFix2X    = 20000;  Tech.iOutArmFix3X = 40000;
    Tech.iOutArmAuto1X   = 214512; Tech.iOutArmAuto2X   = -35078; Tech.iOutArmAuto3X = 0;
}
static bool Run(const char* expectS1)                                          // returns CompareTechData(); checks the refusal text
{
    W906_ShowMyMessage_Reset();
    const bool ok = CompareTechData();
    if (expectS1) CHECK(W906_ShowMyMessage_Count == 1 && W906_ShowMyMessage_LastS1 == AnsiString(expectS1));
    else          CHECK(W906_ShowMyMessage_Count == 0);
    if (W906_ShowMyMessage_Count) std::printf("    refused: %s\n", W906_ShowMyMessage_LastS1.c_str());
    return ok;
}
static std::string ReadAll(const std::string& path)
{
    std::ifstream f(path.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    std::string s = ss.str(), t;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] != 13) t += s[i];         // EOL-agnostic pins
    return t;
}
static int Count(const std::string& s, const std::string& what)
{
    int n = 0;
    for (size_t p = s.find(what); p != std::string::npos; p = s.find(what, p + what.size())) ++n;
    return n;
}

int main(int argc, char** argv)
{
#ifdef SOFT_SIMULTE
    std::printf("CompareTechHt9050 (SIM)\n");
#else
    std::printf("CompareTechHt9050 (SHIP)\n");
#endif
    USE_Scanner_AOI_Inspection = 0;                                             // < eBtnAOI_BottomInstall: the Fix checks run
    FIX3_FULL_PLACE = Fix3K_Uninstall;                                          // the Fix2/3 check runs too

    std::printf(" 1. HT9050 + the machine's 10-04 teach values: START passes, dry run off or on\n");
    MachineTeach();
    W906_Ht9050OrgHomeHook = 0;
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9050;
    CHECK(Run(0));
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9045;               // either half of the test is enough
    CHECK(Run(0));
    W906_GpibModel = "";         MachineTypeChoice = Type_HT9050;
    CHECK(Run(0));
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9050;
    W906_Ht9050OrgHomeHook = &HookAnswers;                                      // SHIP + W906_HT9050_DRYRUN: the dry run on (as before)
    CHECK(Run(0));
    W906_Ht9050OrgHomeHook = 0;

    std::printf(" 2. HT9045 + the same values: refused as golden, one check at a time\n");
    W906_GpibModel = ""; MachineTypeChoice = Type_HT9045;
    CHECK(!Run(kInSht));
    Tech.iInArmShuttle1Y = 0;  Tech.iInArmShuttle2Y = 20000;
    CHECK(!Run(kOutSht));
    Tech.iOutArmShuttle1Y = 0; Tech.iOutArmShuttle2Y = 20000;
    CHECK(!Run(kUnload));
    Tech.iOutArmAuto1X = 0;    Tech.iOutArmAuto2X = 20000;  Tech.iOutArmAuto3X = 40000;
    CHECK(Run(0));                                                              // all seven in golden order: passes
    W906_GpibModel = "HT-9045";                                                 // a non-9050 model string is not an HT9050 either
    Tech.iInArmShuttle1Y = 113317; Tech.iInArmShuttle2Y = -5749;
    CHECK(!Run(kInSht));

    std::printf(" 3. HT9050: the other four checks still refuse\n");
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9050;
    MachineTeach(); Tech.iInArmPlate2Y = 5000;
    CHECK(!Run(kHotPlt));
    MachineTeach(); Tech.iOutArmFix2X = 5000;
    CHECK(!Run(kFix));
    MachineTeach(); Tech.iOutArmFix3X = 25000;
    CHECK(!Run(kFix));
    MachineTeach(); Tech.iOutArmAuto3X = -30000;
    CHECK(!Run(kUnload));
    MachineTeach();
    CHECK(Run(0));

    std::printf(" 4. source pins (cinitial.cpp CompareTechData)\n");
    if (argc > 1)
    {
        const std::string src = ReadAll(std::string(argv[1]) + "/cinitial.cpp");
        const size_t b = src.find("bool CompareTechData()");
        const size_t e = (b == std::string::npos) ? b : src.find("\n}\n", b);
        CHECK(b != std::string::npos && e != std::string::npos);
        const std::string body = (b == std::string::npos || e == std::string::npos) ? std::string() : src.substr(b, e - b);
        CHECK(Count(body, "const bool w906Ht9050=(W906_GpibModel==\"9050GPIB\" || MachineTypeChoice==Type_HT9050);") == 1);
        CHECK(Count(body, "if(!w906Ht9050 && Tech.iInArmShuttle1Y+10000>Tech.iInArmShuttle2Y)") == 1);
        CHECK(Count(body, "if(!w906Ht9050 && Tech.iOutArmShuttle1Y+10000>Tech.iOutArmShuttle2Y)") == 1);
        CHECK(Count(body, "if(!w906Ht9050 && Tech.iOutArmAuto1X+10000>Tech.iOutArmAuto2X)") == 1);
        CHECK(Count(body, "!w906Ht9050") == 3);                                // only the three pairs, not the other four
        CHECK(Count(body, "W906_Ht9050DryRunOn") == 0);                        // no longer tied to the dry run
        CHECK(Count(body, "AI(W906-FR-NB2-3)") >= 1);
    }
    else
    {
        std::printf("  (no port root given -- source pins skipped)\n");
        CHECK(false);
    }

    std::printf("CompareTechHt9050: %d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
