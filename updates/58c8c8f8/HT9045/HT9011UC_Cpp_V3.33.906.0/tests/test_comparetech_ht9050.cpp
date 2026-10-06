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
//    3. (FR-CT1, see below) the other four checks (hot plate, Fix1/2, Fix2/3, Auto2/3) are skipped on an HT9050 too,
//       and still refuse exactly as golden on an HT9045
//    4. source pins (cinitial.cpp): the machine-type test, the AI marker, no dry-run test left in CompareTechData
//    5. (FR-CT1) the HT9050 Auto order test W906_Ht9050AutoOrderBad (Auto1X > Auto2X > Auto3X, 10000 apart) and its switch
//       W906_HT9050_AUTO_ORDER_CHECK, which is OFF (W906_Ht9050AutoOrderRefused answers false on the machine's 1006 values)
//  Reverse check (done by hand for the MR, R236): drop the skip (`!w906Ht9050 &&` x3) -> sections 1 and 4 go red.
//
//  AI(W906-FR-CT1) 20261006: Frank 1006 14:4x / 15:0x (RULINGS_20261005 #20 = Frank's call): an HT9050 skips all seven
//  checks -- "HT9050沒有HP2", "HT9050沒有FIX的選項", Auto = B ("Auto 先全部不檢查，等 EastSun 把 Auto2／3 校正好再打開"),
//  and "這先不要專用檢查" (no HT9050-only checks). Auto order per Frank: "Auto 1->Auto 2->Auto 3 X相是左正右負".
//  Section 3 used to pin the opposite (the four refused on an HT9050); it now pins the ruling. Section 5 is new.
//  Reverse check (by hand for the MR): each `!w906Ht9050 &&` added by FR-CT1 dropped, the Auto line back to golden, the switch
//  default 0 -> 1, the order test flipped, w906Ht9050 forced true -> each goes red (see the MR text).
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
bool W906_Ht9050AutoOrderBad(AnsiString &Str);                                 // cinitial.cpp (end of file), FR-CT1
bool W906_Ht9050AutoOrderRefused(AnsiString &Str);

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

    std::printf(" 3. HT9050: hot plate, Fix1/2, Fix2/3 and Auto2/3 are skipped too (FR-CT1); HT9045 still refuses them\n");
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9050;
    MachineTeach(); Tech.iInArmPlate2Y = 5000;                                 // each of the four broken on its own
    CHECK(Run(0));
    MachineTeach(); Tech.iOutArmFix2X = 5000;
    CHECK(Run(0));
    MachineTeach(); Tech.iOutArmFix3X = 25000;
    CHECK(Run(0));
    MachineTeach(); Tech.iOutArmAuto3X = -30000;
    CHECK(Run(0));
    MachineTeach();                                                             // the machine's 1006 11:51 values (runcfg teach.ini)
    Tech.iInArmPlate1Y = -50046; Tech.iInArmPlate2Y = -6723;
    Tech.iOutArmFix1X = -35657;  Tech.iOutArmFix2X = -21371; Tech.iOutArmFix3X = -7061;
    Tech.iOutArmAuto3X = -16538;
    CHECK(Run(0));
    Tech = TECH();                                                             // every point 0 (untaught points zeroed): golden would refuse
    CHECK(Run(0));
    W906_GpibModel = ""; MachineTypeChoice = Type_HT9045;                       // every other machine: golden, one check at a time
    Tech = TECH();
    CHECK(!Run(kHotPlt));                                                       // 0-10000 < 0
    Tech.iInArmPlate2Y = 20000;
    Tech.iInArmShuttle2Y = 20000; Tech.iOutArmShuttle2Y = 20000;
    CHECK(!Run(kFix));                                                          // Fix1/2: 0+10000 > 0
    Tech.iOutArmFix2X = 20000;
    CHECK(!Run(kFix));                                                          // Fix2/3: 20000+10000 > 0
    Tech.iOutArmFix3X = 40000;
    CHECK(!Run(kUnload));                                                       // Auto1/2: 0+10000 > 0
    Tech.iOutArmAuto2X = 20000;
    CHECK(!Run(kUnload));                                                       // Auto2/3: 20000+10000 > 0 (Auto1/2 passes now)
    Tech.iOutArmAuto3X = 40000;
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
        CHECK(Count(body, "if(!w906Ht9050 && Tech.iInArmPlate2Y-10000<Tech.iInArmPlate1Y)") == 1);             // FR-CT1: no HP2
        CHECK(Count(body, "if(!w906Ht9050 && Tech.iOutArmFix1X+10000>Tech.iOutArmFix2X)") == 1);               // FR-CT1: no Fix
        CHECK(Count(body, "if(!w906Ht9050 && FIX3_FULL_PLACE==Fix3K_Uninstall && (Tech.iOutArmFix2X+10000>Tech.iOutArmFix3X))") == 1);
        CHECK(Count(body, "if(w906Ht9050 ? W906_Ht9050AutoOrderRefused(Str) : (Tech.iOutArmAuto2X+10000>Tech.iOutArmAuto3X))") == 1);
        CHECK(Count(body, "W906_Ht9050DryRunOn") == 0);                        // no longer tied to the dry run
        CHECK(Count(body, "AI(W906-FR-NB2-3)") >= 1);
        CHECK(Count(body, "AI(W906-FR-CT1)") >= 1);
    }
    else
    {
        std::printf("  (no port root given -- source pins skipped)\n");
        CHECK(false);
    }

    std::printf(" 5. the HT9050 Auto order (Auto1X > Auto2X > Auto3X, 10000 apart) and its switch (OFF)\n");
    AnsiString s;
    Tech.iOutArmAuto1X = 214512; Tech.iOutArmAuto2X = 135000; Tech.iOutArmAuto3X = 56000;   // taught right (layout spacing)
    s = ""; CHECK(!W906_Ht9050AutoOrderBad(s) && s == AnsiString(""));
    Tech.iOutArmAuto2X = -35078; Tech.iOutArmAuto3X = -16538;                  // the machine's 1006 values: Auto2/3 reversed
    s = ""; CHECK(W906_Ht9050AutoOrderBad(s) && s == AnsiString("Auto2=-35078, Auto3=-16538"));
    Tech.iOutArmAuto1X = 0; Tech.iOutArmAuto2X = 20000; Tech.iOutArmAuto3X = 40000;         // golden's order is wrong here
    s = ""; CHECK(W906_Ht9050AutoOrderBad(s) && s == AnsiString("Auto1=0, Auto2=20000"));
    Tech.iOutArmAuto1X = 30000; Tech.iOutArmAuto2X = 20000; Tech.iOutArmAuto3X = 10000;     // exactly 10000 apart: allowed (as golden)
    s = ""; CHECK(!W906_Ht9050AutoOrderBad(s));
    Tech.iOutArmAuto2X = 20001;                                                 // 1 short of 10000
    s = ""; CHECK(W906_Ht9050AutoOrderBad(s) && s == AnsiString("Auto1=30000, Auto2=20001"));
    Tech.iOutArmAuto2X = 20000; Tech.iOutArmAuto3X = 10001;
    s = ""; CHECK(W906_Ht9050AutoOrderBad(s) && s == AnsiString("Auto2=20000, Auto3=10001"));
    // The switch: Frank 1006 15:0x B, OFF until EastSun calibrates Auto2/3. Turning it on (W906_HT9050_AUTO_ORDER_CHECK 1)
    // is a ruling change -- flip these two checks then.
    Tech.iOutArmAuto1X = 214512; Tech.iOutArmAuto2X = -35078; Tech.iOutArmAuto3X = -16538;
    s = ""; CHECK(!W906_Ht9050AutoOrderRefused(s) && s == AnsiString(""));
    W906_GpibModel = "9050GPIB"; MachineTypeChoice = Type_HT9050;
    MachineTeach(); Tech.iOutArmAuto3X = -16538;
    CHECK(Run(0));

    std::printf("CompareTechHt9050: %d/%d checks passed\n", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
