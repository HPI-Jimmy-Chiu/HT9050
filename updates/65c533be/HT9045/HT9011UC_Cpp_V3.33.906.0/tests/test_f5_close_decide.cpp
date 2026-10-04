// AI(W906-F5-CLOSE) 20261004: NB2-1 (i), machine dispatch 4 -- the pure decisions of the F5-close rule (tools/F5CloseDecide.h), applied by
//   tools/wb_serve.cpp (F5Close* at EOF). The end-to-end replay (hmi_shell + wait page + wb_serve) is tools/f5_close_replay.ps1.
#include "tools/F5CloseDecide.h"
#include <cstdio>

static int g_fail = 0, g_total = 0;
#define CHECK(c) do { ++g_total; if (!(c)) { ++g_fail; std::printf("  FAIL (line %d): %s\n", __LINE__, #c); } } while (0)

int main()
{
    using namespace ht9045;
    // at start
    CHECK(F5CloseAtStart(false, 10.0) == kF5StartNormally);       // no marker (HT9045_Web.cmd, tests, every non-F5 start): never
    CHECK(F5CloseAtStart(true, -1.0) == kF5StartNormally);        // no flag
    CHECK(F5CloseAtStart(true, 0.0) == kF5DoNotStart);
    CHECK(F5CloseAtStart(true, 352.0) == kF5DoNotStart);          // the slowest F5 build measured (dispatch 5, -O2 MachineType.h)
    CHECK(F5CloseAtStart(true, 3600.0) == kF5DoNotStart);
    CHECK(F5CloseAtStart(true, 3600.5) == kF5StaleFlag);          // left over (a crash): deleted, normal start
    // while running
    CHECK(!F5ClosePending(false, 5.0, false));
    CHECK(F5ClosePending(true, 5.0, false));
    CHECK(!F5ClosePending(true, 5.0, true));                      // SystemStart: the machine runs -- never close it, always reopen its window
    CHECK(!F5ClosePending(true, -1.0, false));
    CHECK(!F5ClosePending(true, 3600.5, false));
    // close now
    CHECK(!F5CloseQuitNow(false, 0, 5000ULL));
    CHECK(!F5CloseQuitNow(true, 1, 5000ULL));                     // a page is there (someone opened it): stay
    CHECK(!F5CloseQuitNow(true, 0, 1999ULL));
    CHECK(F5CloseQuitNow(true, 0, 2000ULL));
    std::printf("F5CloseDecide: %s %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
