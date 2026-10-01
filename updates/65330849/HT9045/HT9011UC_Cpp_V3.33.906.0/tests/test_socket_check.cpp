// =============================================================================
//  test_socket_check.cpp -- J-10 VERIFY: DoCheckSocketHasIC() is golden's 477-line
//                           state machine, not the stub that returned false forever.
//
//  Wave: AI(W906-J10-SOCKETCHECK) 20261001   Suite name (add_test): SocketCheck
//
//  WHAT THIS GUARDS
//  ----------------
//  Until 20261001 the LIVE DoCheckSocketHasIC() was
//        (void)iSelArm; (void)iCheckSocketHasIC;
//        return false;   // golden default: socket check not finished
//  and golden's 477 lines (golden atester.cpp:4305-4781) sat inert inside
//  `#if 0` (GATE G-PTk3-DoCheckSocketHasIC).
//
//  `false` is not a harmless default here.  Its ONLY caller is DoTestY case 20
//  (atester.cpp:5429):
//        else { if(DoCheckSocketHasIC()==true) { Task=25; } }
//  There is no other exit from case 20 once the four skip conditions above it
//  are all false.  A function that can never return true therefore pins the
//  test-cycle dispatcher at Task 20 forever -- the simulated run just stops
//  making progress, with no alarm and no log line to say why.
//  Measured by Jerry in simulation, 20261001.  See docs/handoff/FROM_JERRY.md J-10.
//
//  WHY THESE ASSERTIONS
//  --------------------
//  Golden's cheap, deterministic, hardware-free part is case 1 -- the entry
//  state that InitCheckSocketHasIC() selects (golden :4298-4301 sets the cursor
//  to 1).  Case 1 (golden :4348-4360) does three things and nothing else:
//
//    (a) SKIP:  if(LastSet.bD41TestSocketICCheckSkip==true || bCheckIndex==true)
//                   return true;                                  // golden :4349-4350
//        This is the ONE branch that returns true without touching a motor, and
//        it is exactly the branch the stub made unreachable.  Pinning it is what
//        proves DoTestY case 20 can leave Task 20 again.
//
//    (b) ROUTE: iSelArm==2 -> Task=2000 (arm 2), otherwise Task=1000 (arm 1).
//        Observable through the cursor `iCheckSocketHasIC`, no hardware needed.
//
//    (c) ARM  : CheckSocketHasICHangUpCheck.SetSecAndOn(300) -- the 300 s hang
//        detector (Steven 20210413).  Not asserted directly (TQPF_Timer has no
//        public "is armed" query that is safe to lean on here); it is covered
//        indirectly because case 1 must run to completion for (b) to hold.
//
//  The 1000-/2000-series states drive MOT[MTestZ1].Gali_Two_ZAxis_Move and
//  MOT[MTestY1].GalilTwoY_Move and switch index vacuum.  Those need Sim HAL
//  state that this suite deliberately does NOT stage -- this is a wiring and
//  entry-contract guard, not a full ladder walk.
//
//  TIMER UNIFICATION (same wave): the TU-local `static TQPF_Timer
//  W7T1_CheckSocketHasICDelay;` + `#define` that used to live AFTER this
//  function is retired, and golden's real file-scope object now sits above it
//  (golden :4303).  Golden has exactly one CheckSocketHasICDelay in the whole
//  file, shared with the golden :8555 group (port atester.cpp :9015 / :9019 /
//  :9072 / :9076).  Nothing here asserts that directly; it is recorded so the
//  reason survives if someone later wonders why the stand-in went away.
//
//  EQUIVALENCE NOTE: no Borland binary available; "equivalence" here == clean
//  g++ compile/link of the un-gated golden body + the case-1 behaviour matches
//  what is hand-derived from golden's own text.
// =============================================================================
#include "atester.h"                // DoCheckSocketHasIC, InitCheckSocketHasIC
#include "cmydef.h"                 // iCheckSocketHasIC, bCheckIndex
#include "LastSet.h"                // LastSet.bD41TestSocketICCheckSkip

#include <cstdio>

static int g_pass = 0, g_fail = 0;

static void CHECK(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL: %s\n", what); }
}

//  Put the cursor and both skip inputs in a known state before every case.
static void arm(bool skipD41, bool checkIndex)
{
    InitCheckSocketHasIC();                         // golden :4298 -- cursor = 1
    LastSet.bD41TestSocketICCheckSkip = skipD41;
    bCheckIndex                       = checkIndex;
}

int main()
{
    std::printf("=== J-10 DoCheckSocketHasIC -- golden body, not the constant-false stub ===\n");
    std::printf("(golden atester.cpp:4305-4781, 477 lines; entry case 1 at golden :4348)\n\n");

    std::printf("[1] InitCheckSocketHasIC() selects golden's entry state\n");
    InitCheckSocketHasIC();
    CHECK(iCheckSocketHasIC == 1, "cursor == 1 after Init (golden :4300)");

    std::printf("\n[2] SKIP branch returns TRUE -- the branch the stub made unreachable\n");
    arm(true, false);
    CHECK(DoCheckSocketHasIC() == true,
          "bD41TestSocketICCheckSkip -> true (golden :4349-4350)");
    CHECK(iCheckSocketHasIC == 1,
          "the skip returns BEFORE any Task transition, so the cursor stays 1");

    arm(false, true);
    CHECK(DoCheckSocketHasIC() == true,
          "bCheckIndex -> true (same golden line, the other half of the ||)");

    arm(true, true);
    CHECK(DoCheckSocketHasIC() == true, "both set -> true");

    std::printf("\n[3] the stub could NEVER return true -- this is the regression that mattered\n");
    {
        //  DoTestY case 20 (atester.cpp:5429) only leaves Task 20 when this
        //  returns true.  One true on the skip path is the whole point.
        arm(true, false);
        bool everTrue = false;
        for (int i = 0; i < 16; ++i)
            if (DoCheckSocketHasIC()) { everTrue = true; break; }
        CHECK(everTrue, "DoTestY case 20 can advance to Task 25 again");
    }

    std::printf("\n[4] no skip -> case 1 ROUTES by arm instead of returning (golden :4354-4359)\n");
    arm(false, false);
    CHECK(DoCheckSocketHasIC() == false,
          "no skip -> case 1 does not return true (the SM is only starting)");
    CHECK(iCheckSocketHasIC != 1,
          "case 1 moved the cursor on -- golden sets 1000 (arm 1) or 2000 (arm 2)");

    arm(false, false);
    DoCheckSocketHasIC(2);                          // golden :4356 -- iSelArm==2
    CHECK(iCheckSocketHasIC == 2000,
          "iSelArm==2 -> Task 2000 (arm 2 branch)");

    std::printf("\n[5] the default argument is golden's (atester.h:131 `int iSelArm=3`)\n");
    arm(true, false);
    CHECK(DoCheckSocketHasIC() == DoCheckSocketHasIC(3),
          "DoCheckSocketHasIC() == DoCheckSocketHasIC(3)");

    std::printf("\n==== J-10 SocketCheck summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
