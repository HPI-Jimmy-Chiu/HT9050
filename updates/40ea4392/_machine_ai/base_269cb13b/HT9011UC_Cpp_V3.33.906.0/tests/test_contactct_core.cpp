// =============================================================================
//  test_contactct_core.cpp -- FW-3 queue item 2: TfContactCT test coverage
//
//  AI(W906-FW3-ContactCT-WA) 20260818: new file.
//
//  Covers (per this wave's task brief): ReturnSiteData / ReturnSiteDataArray /
//  GetLowYield_AutoClean / ClearData -- the four "direct-write-globals-then-
//  call" oracle targets (same idiom as tests/test_ga1_cprod.cpp / tests/
//  test_yieldmon_core.cpp).
//
//  Every test seeds TArm/TMySocket state through the ALREADY-PORTED,
//  ALREADY-ACTIVE cSocket.h API (SetPassCT/SetFailCT/ClearALLCT) rather than
//  poking TMySocket's private Pass/Fail/Total fields directly -- these
//  setters are the SAME entry points golden's own ClearData_AutoClean/
//  ClearData already call, verified by reading cSocket.cpp:383-434 (TMySocket)
//  and :796-918 (TArm) this wave: GetTotal()=Pass+Fail (a COUNT, despite the
//  name), GetPCA()=ChangeToFloat(Pass,Total) (a 0-100 PERCENTAGE), TArm::
//  GetPassCT()/GetTotalCT() SUM across all MAX_SOCKET_ROW(4)xMAX_SOCKET_COL(8)
//  sites, TArm::ClearALLCT() (no-arg) zeroes the whole arm.
//
//  Each test group calls ArmData[k]->ClearALLCT() / ArmData_AutoClean[k]->
//  ClearALLCT() first so results are independent of test execution order
//  within this binary (these are process-global TArm objects, cSocket.cpp).
// =============================================================================
#include "forms/fContactCT.h"

#include "MachineType.h"        // test-mode enums, MAX_SOCKET_ROW/COL
#include "cmydef.h"              // ASE_Yield[], iAutoTempOfsTriggerCnt, iLowYieldCloseCount
#include "cprod.h"                // Prod/TestIF/TestIF_File
#include "LastSet.h"               // LastSet.bUseTestSocket[][][]
#include "Config.h"                 // IniConfig
#include "CosFunction.h"             // CosFunction
#include "aHotPlateSubstrate.h"       // TestSocket
#include "cSocket.h"                   // ArmData[3]/ArmData_AutoClean[3]/TArm/TMySocket

#include <cstdio>
#include <cmath>

// ---------------------------------------------------------------------------
//  Minimal CHECK harness (matches tests/test_yieldmon_core.cpp precedent)
// ---------------------------------------------------------------------------
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do { \
        if (cond) { g_pass++; } \
        else { g_fail++; std::printf("FAIL: %s (line %d)\n", msg, __LINE__); } \
    } while (0)

static bool NearlyEqual(double a, double b, double eps = 0.01)
{
    return std::fabs(a - b) < eps;
}

// ---------------------------------------------------------------------------
//  Reset the handful of globals every test in this file touches, so each
//  test controls only what it actually varies (same idiom as tests/
//  test_yieldmon_core.cpp's ResetCommonGuards).
// ---------------------------------------------------------------------------
static void ResetGlobals()
{
    for (int k = 0; k < 3; k++)
        ArmData[k]->ClearALLCT();
    for (int k = 0; k < 3; k++)
        ArmData_AutoClean[k]->ClearALLCT();

    // A test mode outside every NN-mode list (QualSite2X2N/_6Site2X3N/
    // _8Site2X4N/_32Site4X8N/_16Site4X4) so IsNNMode() deterministically
    // returns None_NN (0) -- see cinitial.cpp:7253-7268.
    TestIF.iTestMode = QualSite1X4;
    TestIF_File.iTestMode = QualSite1X4;
    TestIF.iShuttleMode = 0;
    TestIF.iShuttle_Sel = 0;
    IniConfig.bA09_ByArmCloseSite = false;

    for (int i = 0; i < MAX_SOCKET_ROW; i++)
        for (int j = 0; j < MAX_SOCKET_COL; j++)
        {
            LastSet.bUseTestSocket[0][i][j] = false;
            LastSet.bUseTestSocket[1][i][j] = false;
        }

    TestSocket.iShtRow = 0;
    TestSocket.iShtCol = 0;
}

// =============================================================================
//  (1) ReturnSiteData -- golden :317-518
// =============================================================================
static void Test_ReturnSiteData_TotalCount_And_Percentage()
{
    ResetGlobals();
    TfContactCT f;

    // QualSite1X4 hits none of ReturnSiteData's explicit iTestMode branches
    // and TestSite2X2Mode() returns false for it (golden :311, "<=QualSite1X4"
    // -- verified this wave), so the layout falls to the final `else{i=0;
    // j=ARow-1;}` -- ARow=1 -> (i=0, j=0).
    ArmData[0]->SetPassCT(0, 0, 3);
    ArmData[0]->SetFailCT(0, 0, 1);   // Arm0 site(0,0): Total=4, PCA=75.00%

    // rgYieldType->ItemIndex==1 -> raw Total COUNT (not a percentage).
    f.rgYieldType->ItemIndex = 1;
    AnsiString total = f.ReturnSiteData(0, 1);
    CHECK(total == "4", "ReturnSiteData: ItemIndex=1 (Total count) returns raw count \"4\"");

    // rgYieldType->ItemIndex==3 -> GetPCA() as a "%3.2f%%"-formatted percentage.
    f.rgYieldType->ItemIndex = 3;
    AnsiString pct = f.ReturnSiteData(0, 1);
    CHECK(pct == "75.00%", "ReturnSiteData: ItemIndex=3 (PCA%) returns \"75.00%\" for Pass=3/Fail=1");

    // Arm==2 ("Total" -- sums/averages both physical arms). Seed Arm1 too.
    ArmData[1]->SetPassCT(0, 0, 1);
    ArmData[1]->SetFailCT(0, 0, 1);   // Arm1 site(0,0): Total=2, PCA=50.00%

    f.rgYieldType->ItemIndex = 1;   // < 3 -> Result=(iColA+iColB), a raw count sum
    AnsiString totalSum = f.ReturnSiteData(2, 1);
    CHECK(totalSum == "6", "ReturnSiteData: Arm==2, ItemIndex=1 sums both arms' Total counts (4+2=6)");
}

// =============================================================================
//  (2) ReturnSiteDataArray -- golden :523-591
// =============================================================================
static void Test_ReturnSiteDataArray_TotalAndPercentage()
{
    ResetGlobals();
    TfContactCT f;
    (void)f;   // ReturnSiteDataArray is independent of the facade's own widget
               // state (no rgYieldType dependency) -- kept for API symmetry.

    ArmData[0]->SetPassCT(0, 0, 90);
    ArmData[0]->SetFailCT(0, 0, 10);   // Arm0 (0,0): Total=100, PCA=90.00
    ArmData[1]->SetPassCT(0, 0, 70);
    ArmData[1]->SetFailCT(0, 0, 30);   // Arm1 (0,0): Total=100, PCA=70.00

    // bType=true -> Total count. IsNNMode()!=NN_2Row (QualSite1X4) -> the
    // `else` arm: dResult=iColA+iColB = 100+100 = 200.
    double totalResult = f.ReturnSiteDataArray(true, 0, 0);
    CHECK(NearlyEqual(totalResult, 200.0), "ReturnSiteDataArray: bType=true sums both arms' Total (100+100=200)");

    // bType=false -> %. IniConfig.bA09_ByArmCloseSite==false (ResetGlobals) +
    // IsNNMode()!=NN_2Row -> `Result.sprintf("%3.2f%%",(dColA+dColB)/2)` =
    // (90.00+70.00)/2 = 80.00.
    double pctResult = f.ReturnSiteDataArray(false, 0, 0);
    CHECK(NearlyEqual(pctResult, 80.0), "ReturnSiteDataArray: bType=false averages both arms' PCA ((90+70)/2=80)");

    // Negative index guard (golden :529-533): both branches return 0 without
    // touching ArmData at all.
    double negResult = f.ReturnSiteDataArray(false, -1, 0);
    CHECK(NearlyEqual(negResult, 0.0), "ReturnSiteDataArray: i<0 guard returns 0.0");
}

// =============================================================================
//  (3) GetLowYield_AutoClean -- golden :647-722
// =============================================================================
static void Test_GetLowYield_AutoClean_BothTypes()
{
    ResetGlobals();
    TfContactCT f;

    // Two sites, both "used" by both arms, deliberately DIFFERENT PCA so
    // iType=1's dMax-dMin has a real, non-zero answer.
    TestSocket.iShtRow = 1;
    TestSocket.iShtCol = 2;
    LastSet.bUseTestSocket[0][0][0] = true;
    LastSet.bUseTestSocket[1][0][0] = true;
    LastSet.bUseTestSocket[0][0][1] = true;
    LastSet.bUseTestSocket[1][0][1] = true;

    ArmData_AutoClean[0]->SetPassCT(0, 0, 90);
    ArmData_AutoClean[0]->SetFailCT(0, 0, 10);   // site(0,0) Arm0: PCA=90.00
    ArmData_AutoClean[1]->SetPassCT(0, 0, 90);
    ArmData_AutoClean[1]->SetFailCT(0, 0, 10);   // site(0,0) Arm1: PCA=90.00 -> dCol=90.00

    ArmData_AutoClean[0]->SetPassCT(0, 1, 50);
    ArmData_AutoClean[0]->SetFailCT(0, 1, 50);   // site(0,1) Arm0: PCA=50.00
    ArmData_AutoClean[1]->SetPassCT(0, 1, 50);
    ArmData_AutoClean[1]->SetFailCT(0, 1, 50);   // site(0,1) Arm1: PCA=50.00 -> dCol=50.00

    // iType=0 -> ChangeToPercentage(ipass, sum) over the WHOLE arm (TArm::
    // GetTotalCT()/GetPassCT() sum every MAX_SOCKET_ROW x MAX_SOCKET_COL
    // site, not just the two seeded above -- all others are 0 after
    // ResetGlobals's ClearALLCT()). ipass=(90+50)+(90+50)=280,
    // sum=(100+100)+(100+100)=400 -> 70.00%.
    double lowYield = f.GetLowYield_AutoClean(0);
    CHECK(NearlyEqual(lowYield, 70.0), "GetLowYield_AutoClean(0): ipass/sum over both seeded sites = 70.00%");

    // iType=1 -> per-site dCol=(PCA0+PCA1)/2 at each "both arms used" site,
    // dYield=|dMax-dMin| = |90.00-50.00| = 40.00.
    double siteDiff = f.GetLowYield_AutoClean(1);
    CHECK(NearlyEqual(siteDiff, 40.0), "GetLowYield_AutoClean(1): site-yield difference |90-50|=40.00");
}

// =============================================================================
//  (4) ClearData -- golden :888-920
// =============================================================================
static void Test_ClearData_ClearsSiteAndGuardsNegativeIndex()
{
    ResetGlobals();
    TfContactCT f;

    ArmData[0]->SetPassCT(1, 2, 5);
    ArmData[0]->SetFailCT(1, 2, 3);
    ArmData[1]->SetPassCT(1, 2, 4);
    ArmData[1]->SetFailCT(1, 2, 2);
    CHECK(ArmData[0]->ArmSKET[1][2]->GetTotal() == 8, "ClearData setup: Arm0 site(1,2) seeded to Total=8");

    // IsNNMode()==None_NN (QualSite1X4, see ResetGlobals) -> the final
    // `else{for(k=0;k<3;k++) ArmData[k]->ClearALLCT(iRow,iCol);}` branch --
    // ALL THREE arms (0,1,2) get cleared at (1,2).
    f.ClearData(1, 2);
    CHECK(ArmData[0]->ArmSKET[1][2]->GetTotal() == 0, "ClearData: Arm0 site(1,2) cleared to Total=0");
    CHECK(ArmData[1]->ArmSKET[1][2]->GetTotal() == 0, "ClearData: Arm1 site(1,2) cleared to Total=0");

    // Negative-index guard (golden :890-893): re-seed, call with a negative
    // index, confirm NOTHING was touched.
    ArmData[0]->SetPassCT(0, 0, 9);
    ArmData[0]->SetFailCT(0, 0, 1);
    f.ClearData(-1, 0);
    CHECK(ArmData[0]->ArmSKET[0][0]->GetTotal() == 10, "ClearData: negative iRow guard leaves site(0,0) untouched");
}

int main()
{
    Test_ReturnSiteData_TotalCount_And_Percentage();
    Test_ReturnSiteDataArray_TotalAndPercentage();
    Test_GetLowYield_AutoClean_BothTypes();
    Test_ClearData_ClearsSiteAndGuardsNegativeIndex();

    std::printf("%d/%d checks passed (test_contactct_core)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
