// ===========================================================================
//  tests/test_ht9050_prodflow.cpp   (ctest: HT9050_ProdFlow)
//
//  AI(W906-HT9050-PRODFLOW) 20261005: NB2-1 R233, Jimmy 1005 16:2x / 16:4x (to NB2, URGENT U23) -- "把空跑關掉，改跑 Frank 的正式流程 ...
//  除非機台端要求修改空跑功能，不然就是修正式流程" + "機台端不要影響，讓Eastsun決定要不要開起空跑".
//  Ht9050DryRun.cpp W906_Ht9050DryRunOn() now also needs a machine build (no SOFT_SIMULTE):
//    SIM  (SOFT_SIMULTE: dev PCs, NB2, ctest SIM) -- an HT9050 (9050GPIB, the origin hook answering) START runs the Type_HT9050
//         production flow: W906_Ht9050DryRunOn() answers false
//    SHIP (the machine's build) -- unchanged: MachineType.h W906_HT9050_DRYRUN on => the dry run owns DoAllProcess as before
//         (EastSun decides: comment the define out = the production flow on the machine)
//  Source pins: MachineType.h keeps its live #define W906_HT9050_DRYRUN; Ht9050DryRun.cpp has the SOFT_SIMULTE condition;
//  csystem.cpp still asks W906_Ht9050DryRunOn() first. Memory only (argv[1] = the port root, read only).
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
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

static int HookAnswers(int) { return 1; }                                      // an HT9050 1203 axis: "at home"
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
    const bool sim = true;
#else
    const bool sim = false;
#endif
    std::printf("HT9050_ProdFlow (%s)\n", sim ? "SIM" : "SHIP");
    const AnsiString gm = W906_GpibModel;
    const int mt = MachineTypeChoice;

    std::printf(" 1. an HT9050 START: SIM = the production flow, SHIP = the dry run as before (EastSun's switch)\n");
    W906_GpibModel = "9050GPIB";
    MachineTypeChoice = Type_HT9050;
    W906_Ht9050OrgHomeHook = &HookAnswers;
    CHECK(W906_Ht9050OrgHome(MTrayX) != -2);                                    // the dry run's own HT9050 test says yes
#ifdef W906_HT9050_DRYRUN
    CHECK(W906_Ht9050DryRunOn() == !sim);                                       // SIM: false (production flow); SHIP: true (unchanged)
#else
    CHECK(!W906_Ht9050DryRunOn());                                              // the define commented out: the production flow everywhere
#endif
    W906_GpibModel = "HT-9045";                                                 // not an HT9050: never the dry run
    W906_Ht9050OrgHomeHook = 0;
    CHECK(!W906_Ht9050DryRunOn());

    std::printf(" 2. source pins\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string mth = ReadAll(root + "/MachineType.h");
        const std::string dr  = ReadAll(root + "/Ht9050DryRun.cpp");
        const std::string cs  = ReadAll(root + "/csystem.cpp");
        CHECK(!mth.empty() && !dr.empty() && !cs.empty());
        CHECK(Count(mth, "\n#define W906_HT9050_DRYRUN   //AI(W906-HT9050-DRYRUN)") == 1);   // the machine's switch untouched
        CHECK(Count(dr, "#if defined(W906_HT9050_DRYRUN) && !defined(SOFT_SIMULTE)") == 1);
        CHECK(Count(cs, "if(W906_Ht9050DryRunOn())") == 1);                     // DoAllProcess still asks it first
    }
    else
        std::printf("  (skipped: no source root given)\n");

    W906_GpibModel = gm;
    MachineTypeChoice = mt;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
