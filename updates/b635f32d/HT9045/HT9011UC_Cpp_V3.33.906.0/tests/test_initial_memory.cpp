// AI(W906-INITMEM) 20260927: golden InitialMemory() (cmydef.cpp:5786-5872) -- cmydef_InitialMemory.cpp.
//   Dirty a sample of what it clears, blank SiteData[], call it once, then check that SiteData[] carries golden's
//   X x Y for the test modes the port reads (Command.cpp:8734, forms/fLotInfo.cpp:1240, cSetUp.cpp:1506) and that
//   the sample is back to zero -- including asGPIBTempShow[], the one line translated by assignment instead of
//   ZeroMemory (a vclcompat::AnsiString is a class, not one pointer).
//   NOT COVERED: lHandlerStopTime.LatchCycleTime(true) (a wall-clock latch; nothing to assert on without a clock seam).
#include "cmydef.h"
#include <cstdio>

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", what, line); }
    else     { std::printf("  PASS: %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool Site(int mode, int x, int y)
{
    return SiteData[mode].XItem == x && SiteData[mode].YItem == y && SiteData[mode].Cnt == x * y;
}

int main()
{
    for (int i = 0; i < TotalTestMode; i++) { SiteData[i].XItem = 0; SiteData[i].YItem = 0; SiteData[i].Cnt = 0; }
    bSiteHasTurnOn[3] = true;
    iByBinTotal[5] = 7;
    dTorqueArray[1][1999] = 1.5;
    bIdleNeedCheckSafeDoor[3][63][31][7] = true;
    asGPIBTempShow[0] = "stale";
    asGPIBTempShow[tcTotalCount - 1] = "stale too";

    InitialMemory();

    std::printf("[1] SiteData[] = golden X x Y (cmydef.cpp:5840-5858)\n");
    CHECK(Site(SingleSite, 1, 1));
    CHECK(Site(DualSite, 2, 1));
    CHECK(Site(QualSite2X2, 2, 2));
    CHECK(Site(_8Site2X4, 4, 2));
    CHECK(Site(_16Site2X8, 8, 2));
    CHECK(Site(_16Site4X4, 4, 4));
    CHECK(Site(_32Site4X8N, 8, 4));
    CHECK(Site(_8Site2X4N, 4, 2));

    std::printf("[2] the dirtied sample is zero again\n");
    CHECK(!bSiteHasTurnOn[3]);
    CHECK(iByBinTotal[5] == 0);
    CHECK(dTorqueArray[1][1999] == 0.0);
    CHECK(!bIdleNeedCheckSafeDoor[3][63][31][7]);

    std::printf("[3] asGPIBTempShow[] cleared by assignment (the one deviation from golden's ZeroMemory)\n");
    CHECK(asGPIBTempShow[0] == AnsiString(""));
    CHECK(asGPIBTempShow[tcTotalCount - 1] == AnsiString(""));
    asGPIBTempShow[0] = "still a working AnsiString";
    CHECK(asGPIBTempShow[0] == AnsiString("still a working AnsiString"));

    std::printf("[4] AI(W906-TOTALYIELD) GetTotalYield_double / _Str (golden cmydef.cpp:5885-5901, cmydef_TotalYield.cpp)\n");
    iSECSGEMPass = 0; iSECSGEMFail = 0;
    CHECK(GetTotalYield_double() == 0.0);
    iSECSGEMPass = 3; iSECSGEMFail = 1;
    CHECK(GetTotalYield_double() > 74.999 && GetTotalYield_double() < 75.001);
    CHECK(GetTotalYield_Str().Pos("75.00") == 1);   // the trailing lone '%' of golden's "%02.2f%" is not asserted
    iSECSGEMPass = 0; iSECSGEMFail = 0;

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
