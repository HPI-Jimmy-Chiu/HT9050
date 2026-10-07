// =============================================================================
//  tests/test_pci1203_reopen.cpp -- AI(W906-1203REOPEN) 20261007: EtherCAT/Pci1203Reopen.h, the 1203 re-open decision.
//  EastSun 1007 rulings: automatic while stopped + operator button; reset only the ring with the problem; every module
//  problem retries; 10 s apart, 3 tries per fault, the button gives 3 more; WAR16150 / WAR16157 (host). Pure: fake clock.
//  AI(W906-1203REOPEN-2) 20261007: review 20261007 -- every attempt now waits for the module check's verdict (a repeated
//  fault = failed, Recovered = done, nothing for 30 s = failed); the alarm only after the 3rd attempt's failure; the operator
//  button keeps 3 tries even when a fault appears during the manual re-open; an attempt that could not be issued fails at once.
// =============================================================================
#include "EtherCAT/Pci1203Reopen.h"
#include <cstdio>

using namespace ht9045;

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    if (!ok) ++g_fail;
    std::printf("  %s: %s\n", ok ? "PASS" : "FAIL", what);
}

int main()
{
    std::printf("[1] no fault -> nothing\n");
    {
        Pci1203Reopen r; unsigned rings = 9;
        check(r.Step(true, 0, &rings) == kRaNone && rings == 0, "idle: kRaNone, rings 0");
    }

    std::printf("[2] open fail: each try waits for its verdict, 10 s gap after a failure, alarm only after the 3rd verdict\n");
    {
        Pci1203Reopen r; unsigned rings = 0; unsigned long t = 1000;
        r.Fault(kRfOpenFail);
        check(r.Step(true, t, &rings) == kRaReopenCard && r.Tries() == 1, "try 1 at once: re-open card");
        check(r.Step(true, t + 50000, &rings) == kRaNone, "nothing while the attempt has not reported done");
        r.AttemptDone(t + 200);
        check(r.Step(true, t + 15000, &rings) == kRaNone && r.Awaiting(), "after the attempt: waiting for the verdict, no new try even 15 s later");
        r.Fault(kRfOpenFail);                                           // verdict: still failing
        check(r.Step(true, t + 15100, &rings) == kRaNone && !r.Awaiting(), "verdict 'failed' -> the 10 s gap starts now");
        check(r.Step(true, t + 25099, &rings) == kRaNone, "9.999 s after the verdict: wait");
        check(r.Step(true, t + 25100, &rings) == kRaReopenCard && r.Tries() == 2, "10 s after the verdict: try 2 (the repeated fault kept the count)");
        r.AttemptDone(t + 25300);  r.Fault(kRfOpenFail);
        r.Step(true, t + 25400, &rings);
        check(r.Step(true, t + 35400, &rings) == kRaReopenCard && r.Tries() == 3, "try 3");
        r.AttemptDone(t + 35500);
        check(r.Step(true, t + 35600, &rings) == kRaNone && !r.Alarmed(), "right after try 3: NO alarm yet -- it waits for try 3's verdict (review C1)");
        r.Fault(kRfOpenFail);
        check(r.Step(true, t + 40000, &rings) == kRaAlarm, "try 3's verdict 'failed' -> ONE alarm (host: WAR16157)");
        check(r.Step(true, t + 99999, &rings) == kRaNone && r.Alarmed(), "no second alarm, no 4th try");
        r.Fault(kRfOpenFail);
        check(r.Step(true, t + 199999, &rings) == kRaNone, "the same fault again does not refill the budget");
        r.OperatorRetry();
        check(r.Step(true, t + 200000, &rings) == kRaReopenCard && r.Tries() == 1, "operator button: 3 more, the first one now");
    }

    std::printf("[3] no verdict within 30 s counts as a failure; a recovery inside the window ends it\n");
    {
        Pci1203Reopen r; unsigned rings = 0; unsigned long t = 0;
        r.Fault(kRfModules, 2u);
        check(r.Step(true, t, &rings) == kRaResetRing && rings == 2u, "ring 1 problem -> reset ring 1 only");
        r.AttemptDone(t + 100);
        check(r.Step(true, t + 30099, &rings) == kRaNone && r.Awaiting(), "29.999 s without a verdict: still waiting");
        check(r.Step(true, t + 30100, &rings) == kRaNone && !r.Awaiting(), "30 s without a verdict: failed, gap starts");
        check(r.Step(true, t + 40100, &rings) == kRaResetRing && r.Tries() == 2, "10 s later: try 2");
        r.AttemptDone(t + 40200);
        r.Recovered();                                                  // the module check passed 12 s later
        check(r.Step(true, t + 52200, &rings) == kRaRecovered, "check passed -> kRaRecovered once");
        check(r.Step(true, t + 99999, &rings) == kRaNone && r.FaultKind() == kRfNone && r.Tries() == 0, "then idle, budget refilled");
    }

    std::printf("[4] modules: rings merge; no ring -> whole card; running -> nothing; open fail outranks rings\n");
    {
        Pci1203Reopen r; unsigned rings = 0;
        r.Fault(kRfModules, 2u);
        r.Step(true, 0, &rings);  r.AttemptDone(10);
        r.Fault(kRfModules, 1u);                                        // the re-check also sees ring 0
        r.Step(true, 20, &rings);
        check(r.Step(true, 10020, &rings) == kRaResetRing && rings == 3u && r.Tries() == 2, "merged: rings 0+1, try 2");
        Pci1203Reopen n; n.Fault(kRfModules, 0u);
        check(n.Step(true, 0, &rings) == kRaReopenCard, "a module fault with no ring -> whole-card re-open");
        Pci1203Reopen s; s.Fault(kRfModules, 1u);
        check(s.Step(false, 100, &rings) == kRaNone && s.Tries() == 0, "running: no attempt, no try counted");
        s.Fault(kRfOpenFail);
        check(s.Step(true, 200, &rings) == kRaReopenCard, "open fail on top of a ring fault -> whole-card re-open");
    }

    std::printf("[5] operator button with no fault: a manual re-open; a fault during it keeps the 3-try budget (review C6)\n");
    {
        Pci1203Reopen r; unsigned rings = 0;
        r.OperatorRetry();
        check(r.FaultKind() == kRfManual && r.Step(true, 0, &rings) == kRaReopenCard && r.Tries() == 1, "manual: re-open card, try 1");
        r.AttemptDone(10);  r.Fault(kRfOpenFail);                       // the manual re-open left the card closed
        check(r.FaultKind() == kRfOpenFail && r.Tries() == 1, "the fault keeps the manual try count (was reset to 0 = 1 + 3 tries)");
        r.Step(true, 20, &rings);
        check(r.Step(true, 10020, &rings) == kRaReopenCard && r.Tries() == 2, "try 2");
        Pci1203Reopen ok; ok.OperatorRetry(); ok.Step(true, 0, &rings); ok.AttemptDone(1); ok.Recovered();
        check(ok.Step(true, 2, &rings) == kRaRecovered, "manual re-open with a clean verdict -> recovered");
    }

    std::printf("[6] an attempt that could not be issued fails at once (10 s gap, no 30 s verdict wait)\n");
    {
        Pci1203Reopen r; unsigned rings = 0;
        r.Fault(kRfOpenFail);
        r.Step(true, 0, &rings);  r.AttemptNotIssued(5);
        check(!r.Awaiting() && r.Step(true, 9999, &rings) == kRaNone, "not issued: no verdict wait, gap running");
        check(r.Step(true, 10005, &rings) == kRaReopenCard && r.Tries() == 2, "10 s later: try 2");
    }

    std::printf("[7] tick wrap; unknown fault kind\n");
    {
        Pci1203Reopen w; unsigned rings = 0; unsigned long nearWrap = 0xFFFFFFFFul - 3000ul;
        w.Fault(kRfOpenFail);
        w.Step(true, nearWrap, &rings);  w.AttemptDone(nearWrap);
        check(w.Step(true, nearWrap + 29000ul, &rings) == kRaNone && w.Awaiting(), "across the wrap: 29 s -> still waiting for the verdict");
        check(w.Step(true, nearWrap + 30000ul, &rings) == kRaNone && !w.Awaiting(), "across the wrap: 30 s -> failed");
        check(w.Step(true, nearWrap + 40000ul, &rings) == kRaReopenCard, "across the wrap: gap over -> try 2");
        Pci1203Reopen x;
        x.Fault(7);
        check(x.FaultKind() == kRfNone && x.Step(true, 0, &rings) == kRaNone, "an unknown fault kind is ignored");
    }

    std::printf("==== Pci1203Reopen: %d checks, %d failed ====\n", g_total, g_fail);
    return g_fail == 0 ? 0 : 1;
}
