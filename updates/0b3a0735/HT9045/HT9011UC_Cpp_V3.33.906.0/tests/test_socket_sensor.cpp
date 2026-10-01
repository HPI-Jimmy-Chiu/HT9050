// =============================================================================
//  test_socket_sensor.cpp -- J-6 VERIFY: CheckSocketSensor() is golden's body,
//                            not the stub that returned a hard-coded "error".
//
//  Wave: AI(W906-J6-SOCKETSENSOR) 20260930   Suite name (add_test): SocketSensor
//  Asked for by Jimmy, docs/handoff/TO_JERRY.md §4 (20260930 14:3x):
//    "做完請附一支 ctest（例：替身換成本體後，沒有殘料時 CheckSocketSensor 回
//     false、DoTestYFront 不再跳 case 50）".
//
//  WHAT THIS GUARDS
//  ----------------
//  Until 20260930 the LIVE CheckSocketSensor() was
//        (void)iArm; ...; return true;   // offline: socket sensor treated OK
//  and the 343 golden lines sat inert inside `#if 0`.  That return value is
//  BACKWARDS: golden's own last statement is `return bHasErr;`, so `true` means
//  "the socket sensor found a residual IC", not "OK".  The stub therefore
//  reported an error on EVERY call, in every build.
//
//  Measured consequence (simulation, f7a2f5b5, 20260930): the constant "error"
//  drove DoTestYFront case 115 -> Task 50, which leaves
//  MOT[MInShuttle1/2].fCanMoveM false with nothing to restore it, so
//  MotorMove(MInShuttle2) returned 0 forever and acarry.cpp's 30 s Shuttle-2
//  watchdog fired (Enc=45634 Tar=541).  Its ShowMyMessage is blocking, and
//  wb_serve is single-threaded, so MainProc/DoAllProcess/DoInArm stopped being
//  called at all.  See docs/handoff/FROM_JERRY.md J-6.
//
//  WHY THESE ASSERTIONS
//  --------------------
//  The cheap, deterministic, hardware-free part of golden's contract is its
//  OUTER GUARD (golden atester.cpp:10800):
//        if(IniConfig.bC08_SocketSensor && TestIF_File.bEnSocketSensor) { ... }
//        return bHasErr;                       // bHasErr starts false
//  With the feature switched off the whole body is skipped and the verdict is
//  false.  That single fact is what the stub got wrong, and it is exactly the
//  case this machine runs in (C08 off), so it is the regression guard with the
//  most value per line.  The sensor-reading arms need Sen[]/MOT[] state that
//  only the Sim HAL can stage; they are NOT asserted here -- this suite proves
//  the body is wired and its default verdict is correct, not the full ladder.
//
//  EQUIVALENCE NOTE: no Borland binary available; "equivalence" here == clean
//  g++ compile/link of the un-gated golden body + the verdict matches what is
//  hand-derived from golden's own guard and `return bHasErr;`.
// =============================================================================
#include "atester.h"                // CheckSocketSensor
#include "Config.h"                 // IniConfig (bC08_SocketSensor)
#include "cprod.h"                  // TestIF_File (bEnSocketSensor, iSocketCount)

#include <cstdio>

static int g_pass = 0, g_fail = 0;

static void CHECK(bool ok, const char* what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what); }
    else    { ++g_fail; std::printf("  FAIL: %s\n", what); }
}

int main()
{
    std::printf("=== J-6 CheckSocketSensor -- golden body, not the inverted stub ===\n");
    std::printf("(golden atester.cpp:10784-11126; outer guard :10800; `return bHasErr;`)\n\n");

    std::printf("[1] feature OFF -> no residual IC reported (the stub returned true here)\n");
    IniConfig.bC08_SocketSensor  = false;
    TestIF_File.bEnSocketSensor  = false;
    CHECK(CheckSocketSensor(0, "test_J6_arm0", false, false) == false,
          "arm 0: C08 off + bEnSocketSensor off -> false (golden skips the body)");
    CHECK(CheckSocketSensor(1, "test_J6_arm1", false, false) == false,
          "arm 1: same");

    std::printf("\n[2] only ONE half of the guard on -> still skipped (golden uses &&)\n");
    IniConfig.bC08_SocketSensor  = true;
    TestIF_File.bEnSocketSensor  = false;
    CHECK(CheckSocketSensor(0, "test_J6_c08only", false, false) == false,
          "C08 on, bEnSocketSensor off -> false");

    IniConfig.bC08_SocketSensor  = false;
    TestIF_File.bEnSocketSensor  = true;
    CHECK(CheckSocketSensor(0, "test_J6_enonly", false, false) == false,
          "C08 off, bEnSocketSensor on -> false");

    std::printf("\n[3] bInit resets the per-arm counters without changing the verdict\n");
    IniConfig.bC08_SocketSensor  = false;
    TestIF_File.bEnSocketSensor  = false;
    CHECK(CheckSocketSensor(0, "test_J6_init", true, false) == false,
          "bInit=true -> still false (golden clears iShowSocketSensor/iSocketSensorCT, verdict unchanged)");

    std::printf("\n[4] repeat calls are stable (the stub was constant-true; this must be constant-false)\n");
    {
        bool allFalse = true;
        for (int i = 0; i < 32; ++i)
            if (CheckSocketSensor(i & 1, "test_J6_loop", false, false)) { allFalse = false; break; }
        CHECK(allFalse, "32 calls, both arms, feature off -> false every time");
    }

    std::printf("\n==== J-6 SocketSensor summary: %d PASS, %d FAIL ====\n", g_pass, g_fail);
    return (g_fail == 0) ? 0 : 1;
}
