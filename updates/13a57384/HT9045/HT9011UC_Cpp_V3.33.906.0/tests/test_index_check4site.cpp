// =============================================================================
//  test_index_check4site.cpp -- J-12 VERIFY: IndexCheck4Site() is golden's body,
//                               not the stub that returned false forever.
//
//  Wave: AI(W906-J12-INDEXCHECK4SITE) 20261002   Suite name (add_test): IndexCheck4Site
//
//  WHY THIS EXISTS
//  ---------------
//  NB2 R126 M1 (medium, reviewing MR !81 on main).  J-10 un-gated
//  DoCheckSocketHasIC, and its 1000-/2000-series states wait on
//        if(IndexCheck4Site(...)) ...                 (atester.cpp :4971 / :5188)
//  but the LIVE IndexCheck4Site was still
//        (void)bReset; (void)iWhichArm; (void)iSiteCount;
//        return false;                                // golden default
//  with golden's 117 lines inert inside GATE G-PTk3-IndexCheck4Site.  A callee
//  that can never return true turns J-10's fix into a NEW silent stall: Task
//  parks at 1056 / 2056 and only the 300 s CheckSocketHasICHangUpCheck leaves a
//  trace.  Reachability: bDevicConfirm && INDEX_SUCKER_TYPE==1 (:4898 / :4947),
//  and bDevicConfirm is set only for CC_ASE_KaohSiung (cSpeed.cpp:506-509), so
//  ASE Kaohsiung with Index DevicConfirm on and vacuum suckers is the machine
//  that would hit it.  HT9050 (CUSTOMER_CODE=0) is unaffected.
//
//  NB2 also pointed out that J-10's commit message justified "every free
//  function is present" from SYMBOL EXISTENCE, not from whether the definition
//  that actually binds is LIVE.  That is the real lesson: a constant-return stub
//  is indistinguishable from the real body if you only ask "does the name
//  resolve".  The dependency scan for THIS wave classifies every callee as
//  LIVE / STUB / GATED-ONLY / NONE, and re-measuring DoCheckSocketHasIC with it
//  showed IndexCheck4Site was its ONLY stubbed dependency -- nothing else hiding.
//
//  WHAT IS ASSERTED (all hardware-free; golden :9944-10060 == port :10440-10556)
//  ---------------------------------------------------------------------------
//   [1] bReset -> returns false AND parks the cursor at 1 (golden :10449-10453).
//       Taskinitial is golden's file-scope cursor (golden :9942); it has no
//       header, exactly as in golden, so this TU declares it extern itself.
//   [2] THE REGRESSION: with no sucker in the window flagged bNeedCheck, arm 0
//       and arm 1 both RETURN TRUE (golden :10480 / :10506).  The stub could not
//       produce this value at all -- it is the single fact that unblocks
//       DoCheckSocketHasIC's 1056 / 2056 wait.
//   [3] The verdict is read from the real grid, not constant: flag one sucker
//       inside the window and the same call no longer returns true -- it arms
//       the vacuum-check delay and advances the cursor to 2 (golden :10509-10511).
//       No motion and no modal on that path, which is why it is safe here.
//
//  WINDOW / BOUNDS: golden computes iSuckcount = iSiteCount*2, clamped to
//  TestSocket.iShtCnt, and scans [iSuckcount, iSuckcount+2).  fiosetview's grid
//  is bIndexSuck[2][4][8] (atester_shims.h), so the test stays at iSiteCount=0
//  -- window [0,2) -- and never walks past the third index.
//
//  EQUIVALENCE NOTE: no Borland binary available; "equivalence" here == clean
//  g++ compile/link of the un-gated golden body + the case-1 behaviour matches
//  what is hand-derived from golden's own text.
// =============================================================================
#include "atester.h"                // IndexCheck4Site (atester.h:133)
#include "aHotPlateSubstrate.h"     // FTestSuck / BTestSuck / TestSocket (-> mykitsuck.h:303 bNeedCheck)
#include "atester_shims.h"          // fiosetview (bIndexSuck grid)

#include <cstdio>

//  golden atester.cpp:9942 -- file-scope cursor, no header in golden either.
extern int Taskinitial;

static int g_pass = 0, g_fail = 0;

static void CHECK(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL: %s\n", what); }
}

//  Clear both test-head grids and give the socket a known size, then park the
//  cursor at 1 through golden's own reset path.
static void arm()
{
    for (int r = 0; r < 2; ++r)
        for (int c = 0; c < 8; ++c)
        {
            FTestSuck.bNeedCheck[r][c] = false;
            BTestSuck.bNeedCheck[r][c] = false;
        }
    TestSocket.iShtCnt = 4;
    IndexCheck4Site(true, 0, 0);                 // golden :10449 -- iTask=1, return false
}

int main()
{
    std::printf("=== J-12 IndexCheck4Site -- golden body, not the constant-false stub ===\n");
    std::printf("(golden atester.cpp:9944-10060, 117 lines; NB2 R126 M1 on MR !81)\n\n");

    std::printf("[1] bReset parks the cursor and reports \"not finished\"\n");
    Taskinitial = 777;
    CHECK(IndexCheck4Site(true, 0, 0) == false, "bReset -> false (golden :10452)");
    CHECK(Taskinitial == 1, "bReset -> cursor 1 (golden :10451)");

    std::printf("\n[2] nothing needs a vacuum check -> TRUE  (the value the stub could never return)\n");
    arm();
    CHECK(IndexCheck4Site(false, 0, 0) == true,
          "arm 0, no bNeedCheck in the window -> true (golden :10480)");
    CHECK(Taskinitial == 1,
          "...and it returns BEFORE advancing, so the cursor is still 1");

    arm();
    CHECK(IndexCheck4Site(false, 1, 0) == true,
          "arm 1, no bNeedCheck in the window -> true (golden :10506)");

    std::printf("\n[3] the verdict comes from the real grid, not from a constant\n");
    arm();
    FTestSuck.bNeedCheck[0][0] = true;           // inside the window [0,2)
    CHECK(IndexCheck4Site(false, 0, 0) == false,
          "arm 0 with one sucker flagged -> NOT true any more");
    CHECK(Taskinitial == 2,
          "...it armed the vacuum-check delay and moved to case 2 (golden :10509-10511)");

    arm();
    FTestSuck.bNeedCheck[0][4] = true;           // OUTSIDE the window -- must not count
    CHECK(IndexCheck4Site(false, 0, 0) == true,
          "a flag outside [iSuckcount, iSuckcount+2) is ignored -- the window is golden's");

    std::printf("\n==== J-12 IndexCheck4Site summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
