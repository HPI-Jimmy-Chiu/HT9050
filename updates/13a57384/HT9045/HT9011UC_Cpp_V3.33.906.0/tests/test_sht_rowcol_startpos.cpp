// =============================================================================
//  test_sht_rowcol_startpos.cpp -- J-7 VERIFY: GetShtRowColStartPos() reads the
//                                  real TestSocket grid, not a pinned 0x0.
//
//  Wave: AI(W906-J12-INDEXCHECK4SITE) 20261002   Suite name (add_test): ShtRowColStartPos
//  Asked for by NB2 R126 (low): "GetShtRowColStartPos 的改動沒有 ctest".
//
//  WHAT THIS GUARDS
//  ----------------
//  Until 20261001 (J-7, MR !80) golden's two reads at :9095-9096 sat inert in a
//  `#if 0` and the live code was
//        int iRow=0; int iCol=0;
//  so GetShtStartPos() always derived the start position from a 0x0 grid.  The
//  in-arm place target therefore landed outside the soft limit and the machine
//  raised WAR0154 (measured in simulation 20261001).
//
//  WHY THE 0x0 DEFAULT IS WORSE THAN "NO OFFSET"
//  ---------------------------------------------
//        FindCentorPointIndex(n) = (n-1)/2          (ainarm9045.cpp:608-613)
//        GetShtStartPos(n,C,P)   = C - (int)(index*P)
//  n=0 gives index -0.5, i.e. C + P/2 -- the stub did not merely lose the grid,
//  it shifted the start HALF A PITCH THE WRONG WAY, identically for every
//  machine geometry.  That is exactly what this suite pins: the result must now
//  DEPEND on TestSocket, and must match golden's arithmetic for each grid.
//
//  All inputs here are plain globals (centre, Prod offset, pitch, bases), so the
//  expected values are hand-derived from golden's two lines above -- no motion,
//  no IO, no Sim HAL.
//
//  EQUIVALENCE NOTE: no Borland binary available; "equivalence" here == the
//  values match what golden's own arithmetic yields for the staged inputs.
// =============================================================================
#include "ainarm9045.h"             // GetShtRowColStartPos (ainarm9045.h:148)
#include "aHotPlateSubstrate.h"     // TestSocket (iShtRow :415 / iShtCol :447)
#include "cprod.h"                  // Prod, TestIF
#include "cmydef.h"                 // iInArmSht*CenterPos, iInArm*Base, bUseTwoArm32Site, MInShuttle1

#include <cstdio>

static int g_pass = 0, g_fail = 0;

static void CHECK(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL: %s\n", what); }
}

static void CHECK_EQ(int got, int want, const char* what)
{
    if (got == want) { ++g_pass; std::printf("  PASS: %s (= %d)\n", what, got); }
    else             { ++g_fail; std::printf("  FAIL: %s -- got %d, want %d\n", what, got, want); }
}

static const int kCentreX = 100000;
static const int kCentreY = 200000;
static const int kPitchX  = 1000;
static const int kPitchY  = 800;

//  Stage every input golden's two lines consume, so only TestSocket varies.
static void stage(int row, int col)
{
    iInArmShtXCenterPos = kCentreX;
    iInArmShtYCenterPos = kCentreY;
    iInArmXBase         = 0;
    iInArmYBase         = 0;
    bUseTwoArm32Site    = false;                 // :662 halves iRow when true
    Prod.XInArm_Shuttle1_Place[0][0] = 0;
    Prod.YInArm_Shuttle1_Place[0][0] = 0;
    TestIF.dSiteXPitch  = kPitchX;
    TestIF.dSiteYPitch  = kPitchY;
    TestSocket.iShtRow  = row;                   // golden :9095
    TestSocket.iShtCol  = col;                   // golden :9096
}

//  golden ainarm9045.cpp:608-619, reproduced here so the expectation is derived
//  from golden's text rather than from the function under test.
static int expect(int n, int centre, int pitch)
{
    const double index = (double)(n - 1) / 2;
    return centre - (int)(index * pitch);
}

int main()
{
    int c = -1, r = -1;

    std::printf("=== J-7 GetShtRowColStartPos -- the TestSocket grid reaches the result ===\n");
    std::printf("(golden ainarm9045.cpp:9093-9136; the two reads are golden :9095-9096)\n\n");

    std::printf("[1] 1x1 -- centre index 0, so the start IS the centre\n");
    stage(1, 1);
    GetShtRowColStartPos(MInShuttle1, c, r);
    CHECK_EQ(c, expect(1, kCentreX, kPitchX), "iColStart for 1 column");
    CHECK_EQ(r, expect(1, kCentreY, -kPitchY), "iRowStart for 1 row (golden negates the Y pitch)");

    std::printf("\n[2] 2x2 -- centre index 0.5, so the start backs off half a pitch\n");
    stage(2, 2);
    GetShtRowColStartPos(MInShuttle1, c, r);
    CHECK_EQ(c, expect(2, kCentreX, kPitchX), "iColStart for 2 columns");
    CHECK_EQ(r, expect(2, kCentreY, -kPitchY), "iRowStart for 2 rows");

    std::printf("\n[3] THE REGRESSION: the answer must DEPEND on TestSocket\n");
    {
        stage(1, 1);
        GetShtRowColStartPos(MInShuttle1, c, r);
        const int c1 = c, r1 = r;
        stage(2, 2);
        GetShtRowColStartPos(MInShuttle1, c, r);
        CHECK(c != c1, "a different column count gives a different iColStart");
        CHECK(r != r1, "a different row count gives a different iRowStart");

        //  What the stub did: iRow=iCol=0 for every geometry.
        stage(2, 2);
        GetShtRowColStartPos(MInShuttle1, c, r);
        CHECK(c != expect(0, kCentreX, kPitchX),
              "and it is NOT the old 0x0 answer (which sat half a pitch the wrong side)");
    }

    std::printf("\n[4] an unhandled target leaves both outputs at golden's 0 (the `else return;`)\n");
    stage(2, 2);
    c = r = -1;
    GetShtRowColStartPos(-12345, c, r);
    CHECK_EQ(c, 0, "iColStart");
    CHECK_EQ(r, 0, "iRowStart");

    std::printf("\n==== J-7 ShtRowColStartPos summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
