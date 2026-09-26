// =============================================================================
//  test_set_test_timeout_timer.cpp -- SetTestTimeOutTimer arms the three test time-out timers (golden 912
//                                      atester.cpp:10920-10955).
//
//  AI(W906-GB-P2b) 20260926.  Suite name (add_test): TesterComm_TestTimeOutTimer
//
//  Translation-gap fix: SetTestTimeOutTimer was a no-op, so in On-Line the live checks
//  `LastSet.iTester==ON_LINE && h?TestTimeOutDelay.Off()` (aTester_Front.cpp / aTester_Rear.cpp) saw an unarmed
//  TQPF_Timer, which is Off() at once, and every test timed out right after SOT.  This proves:
//    (a) an unarmed TQPF_Timer is Off() at once (the premise of the defect);
//    (b) SetTestTimeOutTimer(0) arms hFTestTimeOutDelay to TestIF.iMaxTime and TestTimeOut to iMaxTime+10;
//        hBTestTimeOutDelay stays unarmed; after iMaxTime the per-arm timer is Off(), TestTimeOut is not;
//    (c) SetTestTimeOutTimer(1) arms hBTestTimeOutDelay the same way;
//    (d) bInitialMaxTime==true uses TestIF.iInitialMaxTime instead.
//  TQPF_Timer runs on QueryPerformanceCounter (no fake clock), so the times are 50-200 ms and the waits are
//  Sleep() with a wide margin.  No file or disk access.
// =============================================================================
#include "atester.h"     // SetTestTimeOutTimer, TestTimeOut
#include "cprod.h"       // TestIF
#include "cmydef.h"      // bInitialMaxTime
#include "myTimer.h"
#include <windows.h>
#include <cstdio>

extern TQPF_Timer hFTestTimeOutDelay;    // aTester_Front.cpp:3085
extern TQPF_Timer hBTestTimeOutDelay;    // aTester_Rear.cpp:2974

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

int main()
{
    printf("TesterComm_TestTimeOutTimer\n");

    // (a) the premise: an unarmed timer is Off() straight away
    {
        TQPF_Timer t;
        CHECK(t.Off(), "(a) unarmed TQPF_Timer is Off() at once");
    }

    // (b) Index 0 -> hFTestTimeOutDelay = iMaxTime, TestTimeOut = iMaxTime+10
    bInitialMaxTime = false;
    TestIF.iMaxTime = 0.05;
    TestIF.iInitialMaxTime = 0.4;
    SetTestTimeOutTimer(0);
    CHECK(!hFTestTimeOutDelay.Off(), "(b) Index 0: hFTestTimeOutDelay armed right after the call");
    CHECK(!TestTimeOut.Off(),        "(b) Index 0: TestTimeOut armed right after the call");
    CHECK(hBTestTimeOutDelay.Off(),  "(b) Index 0: hBTestTimeOutDelay not armed");
    Sleep(150);
    CHECK(hFTestTimeOutDelay.Off(),  "(b) Index 0: hFTestTimeOutDelay Off() after iMaxTime");
    CHECK(!TestTimeOut.Off(),        "(b) Index 0: TestTimeOut still on (iMaxTime+10)");

    // (c) Index 1 -> hBTestTimeOutDelay
    SetTestTimeOutTimer(1);
    CHECK(!hBTestTimeOutDelay.Off(), "(c) Index 1: hBTestTimeOutDelay armed right after the call");
    CHECK(hFTestTimeOutDelay.Off(),  "(c) Index 1: hFTestTimeOutDelay not re-armed");
    Sleep(150);
    CHECK(hBTestTimeOutDelay.Off(),  "(c) Index 1: hBTestTimeOutDelay Off() after iMaxTime");

    // (d) bInitialMaxTime -> iInitialMaxTime (0.4 s): still on after 150 ms, Off() after 600 ms
    bInitialMaxTime = true;
    SetTestTimeOutTimer(0);
    Sleep(150);
    CHECK(!hFTestTimeOutDelay.Off(), "(d) bInitialMaxTime: hFTestTimeOutDelay uses iInitialMaxTime (still on at 150 ms)");
    Sleep(450);
    CHECK(hFTestTimeOutDelay.Off(),  "(d) bInitialMaxTime: hFTestTimeOutDelay Off() after iInitialMaxTime");
    CHECK(!TestTimeOut.Off(),        "(d) bInitialMaxTime: TestTimeOut still on (iInitialMaxTime+10)");
    bInitialMaxTime = false;

    printf("TesterComm_TestTimeOutTimer: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
