// test_jamday_hook.cpp -- RUN_INFO::SaveJamRateByDay's body hook (cprod.cpp:1070-1071).
// AI(W906-JAMDAY) 20261001: census 129 (e) E-FT2-011. Golden RUN_INFO::AddAlarm (cprod.cpp:1043-1047) saves the day's
//   JamRate_Daily file on the first JAM of a new day and then clears the counts. The port's SaveJamRateByDay body is
//   `#if 0` (library split: cprod.cpp is ht9045_globals; the golden-faithful copy is FileRW/MainClose.cpp
//   W906_RunInfo_SaveJamRateByDay, wb_serve only), so the rollover cleared yesterday's counts without saving them.
//   The body now runs through W906_SaveJamRateByDayBody when one is installed.
// Pins: no body -> exactly as before (the rollover clears, nothing else); a body -> called once from the rollover with
//   bUpload=true (golden's default argument) BEFORE the counts are cleared (it must see yesterday's); no call on the same
//   day; SaveJamRateByDay(false) passes false; a non-JAM code does nothing. Memory only: the body here is a recorder.
#include "cprod.h"

#include <cstdio>

#include "cmydef.h"      // SystemDate is `Word` (cmydef.h:227); this test first declared its own `extern int SystemDate;`,
                         // read 4 bytes of a 2-byte global and failed its iToday checks -- the linker never compares types

extern void (*W906_SaveJamRateByDayBody)(bool);

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("FAIL line %d: %s\n", line, what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static int g_calls = 0;
static bool g_lastUpload = false;
static int g_seenDaily = -1;
static void Body(bool bUpload)
{
    ++g_calls;
    g_lastUpload = bUpload;
    g_seenDaily = (int)RunInfo.vDailyJam.size();
}

int main()
{
    std::printf("[1] no body installed: the rollover clears yesterday, nothing else\n");
    W906_SaveJamRateByDayBody = 0;
    RunInfo.InitialDailyData();
    RunInfo.AddAlarm("JAM0001", "a");
    CHECK(RunInfo.vDailyJam.size() == 1 && RunInfo.iToday == SystemDate);
    RunInfo.iToday = SystemDate + 1;                         // the counts are from another day
    RunInfo.AddAlarm("JAM0002", "b");
    CHECK(RunInfo.vDailyJam.size() == 1 && RunInfo.vDailyJam.count("JAM0002") == 1 && RunInfo.iToday == SystemDate);

    std::printf("[2] a body: called once from the rollover, with bUpload=true, before the clear\n");
    W906_SaveJamRateByDayBody = &Body;
    RunInfo.iToday = SystemDate + 1;
    RunInfo.AddAlarm("JAM0003", "c");
    CHECK(g_calls == 1 && g_lastUpload == true && g_seenDaily == 1);          // saw JAM0002, yesterday's
    CHECK(RunInfo.vDailyJam.size() == 1 && RunInfo.vDailyJam.count("JAM0003") == 1 && RunInfo.iToday == SystemDate);

    std::printf("[3] same day: no call\n");
    RunInfo.AddAlarm("JAM0003", "c");
    CHECK(g_calls == 1 && RunInfo.vDailyJam["JAM0003"].iCount == 2);

    std::printf("[4] SaveJamRateByDay(false) passes false, then clears\n");
    RunInfo.SaveJamRateByDay(false);
    CHECK(g_calls == 2 && g_lastUpload == false && g_seenDaily == 1 && RunInfo.vDailyJam.empty());

    std::printf("[5] a non-JAM code does nothing\n");
    RunInfo.iToday = SystemDate + 1;
    RunInfo.AddAlarm("ERR0001", "d");
    CHECK(g_calls == 2 && RunInfo.vDailyJam.empty() && RunInfo.iToday == SystemDate + 1);

    W906_SaveJamRateByDayBody = 0;
    std::printf("test_jamday_hook: %d/%d checks passed\n", g_total - g_fail, g_total);
    if (g_fail) { std::printf("FAILED: %d checks\n", g_fail); return 1; }
    std::printf("PASS\n");
    return 0;
}
