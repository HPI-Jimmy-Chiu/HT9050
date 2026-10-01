// AI(W906-FUSELIMIT) 20261001: W906_TfMain_FormShow_TempFuseLimit (MainFormShow_TempFuse.cpp) = golden TfMain::FormShow
//   main.cpp:9535-9551.  Part 1 walks the golden decision table (each branch, both precedence overlaps, the "unset
//   customer code is HONPREC QC" quirk).  Part 2 checks that wb_serve's boot calls it before InitialHandler and before
//   the boot W906_DoReadLastData (whose ReadLastSetIni runs the cprod.cpp:3033 clamp) -- the order is the whole point:
//   called after them, the boot clamp would still see 0.0.  Memory only; no file is written.
//   NOT COVERED: the three readers themselves (cprod.cpp:3033, forms/fHS.cpp:857, uHeaterThread.cpp:1294).
#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include "cmydef.h"

void W906_TfMain_FormShow_TempFuseLimit();

static int g_fail = 0;

static void row(const char* name, int cc, int machine, int limit, double want)
{
    CUSTOMER_CODE = cc;
    MachineTypeChoice = machine;
    iTempLimitation = limit;
    TempFuseLimitType = -1.0;
    W906_TfMain_FormShow_TempFuseLimit();
    const bool ok = TempFuseLimitType == want;
    std::printf("%s %-44s cc=%d machine=%d limit=%d -> %.1f (want %.1f)\n", ok ? "PASS" : "FAIL", name, cc, machine,
                limit, TempFuseLimitType, want);
    if (!ok) ++g_fail;
}

int main(int argc, char** argv)
{
    const double L250 = TemperatureFuseLimit250, L200 = TemperatureFuseLimit200, L170 = TemperatureFuseLimit170;
    if (L250 != 250.0 || L200 != 200.0 || L170 != 165.0) {   // golden cmydef.cpp:5408-5410
        std::printf("FAIL constants 170/200/250 = %.1f/%.1f/%.1f (golden 165/200/250)\n", L170, L200, L250);
        ++g_fail;
    }
    const int OTHER = CC_KYEC_LEE;   // any customer outside both lists
    row("other customer, HT9045, 130", OTHER, Type_HT9045, tTemp130, L170);
    row("HONPREC QC", CC_HONPREC_QC, Type_HT9045, tTemp130, L250);
    row("EMemory", CC_EMemory, Type_HT9045, tTemp130, L250);
    row("tTemp200", OTHER, Type_HT9045, tTemp200, L250);
    row("SCC", CC_SCC, Type_HT9045, tTemp130, L200);
    row("HT9046_LS", OTHER, Type_HT9046_LS, tTemp130, L200);
    row("tTemp150", OTHER, Type_HT9045, tTemp150, L200);
    row("tTemp155", OTHER, Type_HT9045, tTemp155, L200);
    row("tTemp175", OTHER, Type_HT9045, tTemp175, L200);
    row("SCC + tTemp200 (first branch wins)", CC_SCC, Type_HT9045, tTemp200, L250);
    row("HONPREC QC + HT9046_LS + 150 (first wins)", CC_HONPREC_QC, Type_HT9046_LS, tTemp150, L250);
    row("unset customer code 0 == CC_HONPREC_QC", 0, Type_HT9045, tTemp130, L250);

    // Part 2: boot order in tools/wb_serve.cpp.
    const std::string src = argc > 1 ? argv[1] : ".";   // CMAKE_SOURCE_DIR (add_test passes it)
    std::ifstream f(src + "/tools/wb_serve.cpp", std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    const std::string s = ss.str();
    const size_t call = s.find("W906_TfMain_FormShow_TempFuseLimit(); }");
    const size_t ih = s.find("InitialHandler(); const bool okWP = true;");
    const size_t rld = s.find("W906_DoReadLastData(true, binSelLoaded, tempLoaded);");
    const size_t call2 = call == std::string::npos ? call : s.find("W906_TfMain_FormShow_TempFuseLimit(); }", call + 1);
    const bool orderOk = !s.empty() && call != std::string::npos && ih != std::string::npos &&
                         rld != std::string::npos && call < ih && ih < rld && call2 == std::string::npos;
    std::printf("%s boot order: call@%lld < InitialHandler@%lld < W906_DoReadLastData(true)@%lld, one call site (%s)\n",
                orderOk ? "PASS" : "FAIL", (long long)call, (long long)ih, (long long)rld,
                call2 == std::string::npos ? "yes" : "no");
    if (!orderOk) ++g_fail;

    std::printf("%s TempFuseLimit (%d failure(s))\n", g_fail ? "FAILED" : "ALL PASS", g_fail);
    return g_fail ? 1 : 0;
}
