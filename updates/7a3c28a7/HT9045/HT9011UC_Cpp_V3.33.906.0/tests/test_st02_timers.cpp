// =============================================================================
//  tests/test_st02_timers.cpp -- S-15: golden TfMain::Timer8Timer + TimerTemperatureStorageMinuteTimer at golden's
//  1000 ms (MainTimersSt02.cpp / MainTimer8.cpp).  AI(W906-S15) 20261001 (St02-E).  Suite: St02_MainTimers.
//
//  CONTAINMENT FIRST (st02_test_containment.h): RecordProcess / MyDBIProcess write through the ctest redirect roots
//  only; the temperature log roots (asHiSiLogPath, asLbTempLogPath) point at a %TEMP% sandbox before anything runs.
//    1. the rate: W906_St02TimersTickAt every 500 ms -> each timer every second call; the first one 1000 ms after the
//       start; a 5 s gap -> one call, no catch-up; a jittered beat stays on the 1000 ms deadline grid (E2 correction 1).
//    2. [P29] (golden main.cpp:32195-32214): interval 0, the loader-full sensor on -> MES0921 (kcode 0) once; while its
//       notice is still showing (fNote->fShow, edErrorCode "MES0921") Timer8 does nothing (the St02-M deviation:
//       golden's ShowModal held Timer8); notice gone -> MES0921 again; the sensor off -> no more.
//    3. the gated parts: VTEST (S25) with the tester-alarm sensor on -> no MES0732 in 12 calls.
//    4. [O11] Jam Rate (golden :32106-32150) with a fake clock: off -> nothing; on, 1 minute, 3 jams / 120 loaders ->
//       after 60 s exactly one "[Jam Rate Record] 3/120  MTBF 1/3hr", the two counters back to 0.
//    5. the temperature timer (golden :31175-31180): [L10] "only while testing" on -> TemperatureStorageLog not called
//       (no month folder in the sandbox); off -> called (the month folder appears).
//    6. pnlCleanCount->Visible follows bEnableAutoCleanFunction && iAutoClean_Function.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "cprod.h"
#include "mysensor.h"
#include "canary_support.h"
#include "common.h"
#include "cpublic.h"           // GetTimeInfo
#include "forms/fMain.h"
#include "forms/fNote.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <ctime>
#include <string>

namespace ht9045 {
void W906_St02TimersTickAt(unsigned long now);
void W906_St02TimersReset();
void W906_St02TimersCounts(unsigned long* t8, unsigned long* ts);
void W906_Timer8Timer();
void W906_TimerTemperatureStorageMinuteTimer();
void W906_Timer8SetClock(clock_t (*fn)());
}

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static clock_t g_fakeClock = 1000;
static clock_t FakeClock() { return g_fakeClock; }

static void FixSensor(int idx, bool on)   // as tests/test_i115b_lifted.cpp:112-118: no IO read, Type decides
{
    Sen[idx].Enable  = true;
    Sen[idx].ISABase = 99;
    Sen[idx].Type    = on ? 0 : 1;
}

static bool DirExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("St02_MainTimers\n");
    if (!W906TestInsideCtestRoots("St02_MainTimers"))
        return 2;
    char tmp[MAX_PATH];
    ::GetTempPathA(MAX_PATH, tmp);
    char sb[MAX_PATH];
    std::snprintf(sb, sizeof(sb), "%sht9045_st02timers_%lu", tmp, (unsigned long)::GetTickCount());
    const std::string sandbox = sb;
    ::CreateDirectoryA(sandbox.c_str(), 0);
    asHiSiLogPath   = (sandbox + "\\Temperature").c_str();
    asLbTempLogPath = (sandbox + "\\LB_Temperature").c_str();
    LB_TEMP_UpDown  = false;
    ht9045::W906_Timer8SetClock(&FakeClock);   // before the first Timer8: RecordJamRateByTime's static start reads it

    InitialOK = true;
    GetTimeInfo();
    IniConfig.bP29LoaderCheckIsFull = false;
    IniConfig.bRecordJamRateByTime  = false;
    IniConfig.bL10IndexTestlogTemp  = true;
    IniConfig.bVTESTFunction        = false;
    CHECK(fMain != 0 && fNote != 0);
    if (fMain == 0 || fNote == 0) return 1;
    fNote->fShow = false;

    std::printf(" 1. the rate\n");
    {
        ht9045::W906_St02TimersReset();
        unsigned long n8 = 0, nts = 0;
        const unsigned long t0 = 100000;
        ht9045::W906_St02TimersTickAt(t0);            // the start
        ht9045::W906_St02TimersTickAt(t0 + 500);
        ht9045::W906_St02TimersCounts(&n8, &nts);
        CHECK(n8 == 0 && nts == 0);                   // not before one Interval
        for (int i = 2; i <= 9; ++i)
            ht9045::W906_St02TimersTickAt(t0 + 500 * i);   // 1000 .. 4500
        ht9045::W906_St02TimersCounts(&n8, &nts);
        CHECK(n8 == 4 && nts == 4);                   // 1000 / 2000 / 3000 / 4000
        ht9045::W906_St02TimersTickAt(t0 + 9500);     // a 5 s gap
        ht9045::W906_St02TimersCounts(&n8, &nts);
        CHECK(n8 == 5 && nts == 5);                   // one call, no catch-up

        ht9045::W906_St02TimersReset();               // a jittered ~500 ms beat: deadlines at +1000 / +2000 / +3000
        const unsigned long j[] = { 0, 1000, 1490, 1990, 2010, 2500, 2990, 3005 };
        for (size_t i = 0; i < sizeof(j) / sizeof(j[0]); ++i)
            ht9045::W906_St02TimersTickAt(t0 + j[i]);
        ht9045::W906_St02TimersCounts(&n8, &nts);
        CHECK(n8 == 3 && nts == 3);                   // +1000, +2010, +3005 ("1000 since the last call" would give 2)
    }

    std::printf(" 2. [P29] loader full -> MES0921, held while its notice is up\n");
    {
        IniConfig.bP29LoaderCheckIsFull = true;
        IniConfig.dP29LoaderCheckIsFullInterval = 0;
        FixSensor(SnLoaderIsFull, true);
        const int c0 = W906_ShowErrorMessage_Count;
        ht9045::W906_Timer8Timer();                   // arms the delay (0 s); golden checks it in the same call
        int c1 = W906_ShowErrorMessage_Count;
        if (c1 == c0) { ::Sleep(5); ht9045::W906_Timer8Timer(); c1 = W906_ShowErrorMessage_Count; }
        CHECK(c1 == c0 + 1);                          // the delay is over -> MES0921, once
        CHECK(std::string(W906_ShowErrorMessage_LastCode.c_str()) == "MES0921" && W906_ShowErrorMessage_LastKCode == 0);
        fNote->fShow = true;                          // the notice is still up
        if (fNote->edErrorCode) fNote->edErrorCode->Text = "MES0921";
        ht9045::W906_Timer8Timer();
        ht9045::W906_Timer8Timer();
        CHECK(W906_ShowErrorMessage_Count == c1);     // held, as golden's ShowModal
        fNote->fShow = false;                         // the operator closed it
        ht9045::W906_Timer8Timer();
        CHECK(W906_ShowErrorMessage_Count == c1 + 1);
        FixSensor(SnLoaderIsFull, false);
        ht9045::W906_Timer8Timer();
        ht9045::W906_Timer8Timer();
        CHECK(W906_ShowErrorMessage_Count == c1 + 1); // sensor off -> no more
        IniConfig.bP29LoaderCheckIsFull = false;
        Sen[SnLoaderIsFull].Enable = false;
    }

    std::printf(" 3. gated: VTEST (S25)\n");
    {
        IniConfig.bVTESTFunction = true;
        FixSensor(SnTesterAlarm, true);
        const int c0 = W906_ShowErrorMessage_Count;
        for (int i = 0; i < 12; ++i)
            ht9045::W906_Timer8Timer();
        CHECK(W906_ShowErrorMessage_Count == c0);
        IniConfig.bVTESTFunction = false;
        Sen[SnTesterAlarm].Enable = false;
    }

    std::printf(" 4. [O11] Jam Rate Record\n");
    {
        iRecordJamRateByTime_JamCount = 3;
        iRecordJamRateByTime_LoaderCount = 120;
        IniConfig.bRecordJamRateByTime = false;
        g_fakeClock += 120000;
        ht9045::W906_Timer8Timer();
        CHECK(iRecordJamRateByTime_JamCount == 3 && iRecordJamRateByTime_LoaderCount == 120);   // off: nothing
        IniConfig.bRecordJamRateByTime = true;
        IniConfig.iRecordJamRateIntervalTime = 1;
        bRecordJamRateByTime_Clear = true;            // golden: restart the interval now
        ht9045::W906_Timer8Timer();
        CHECK(bRecordJamRateByTime_Clear == false && iRecordJamRateByTime_JamCount == 3);
        g_fakeClock += 59000;
        ht9045::W906_Timer8Timer();
        CHECK(iRecordJamRateByTime_JamCount == 3);    // 59 s: not yet
        g_fakeClock += 1000;
        ht9045::W906_Timer8Timer();
        CHECK(iRecordJamRateByTime_JamCount == 0 && iRecordJamRateByTime_LoaderCount == 0);   // 60 s: recorded, reset
        IniConfig.bRecordJamRateByTime = false;
    }

    std::printf(" 5. the temperature timer\n");
    {
        char month[16];
        std::snprintf(month, sizeof(month), "%04d%02d", (int)SystemYear, (int)SystemMonth);
        const std::string monthDir = sandbox + "\\Temperature\\" + month;
        IniConfig.bL10IndexTestlogTemp = true;
        ht9045::W906_TimerTemperatureStorageMinuteTimer();
        CHECK(!DirExists(monthDir));                  // [L10] on: not called
        IniConfig.bL10IndexTestlogTemp = false;
        ht9045::W906_TimerTemperatureStorageMinuteTimer();
        CHECK(DirExists(monthDir));                   // called: TemperatureStorageLog makes the month folder
        IniConfig.bL10IndexTestlogTemp = true;
    }

    std::printf(" 6. pnlCleanCount\n");
    {
        CHECK(fMain->pnlCleanCount != 0);
        if (fMain->pnlCleanCount)
        {
            IniConfig.bEnableAutoCleanFunction = true;
            TestIF_File.iAutoClean_Function = 1;
            ht9045::W906_Timer8Timer();
            CHECK(fMain->pnlCleanCount->Visible == true);
            TestIF_File.iAutoClean_Function = 0;
            ht9045::W906_Timer8Timer();
            CHECK(fMain->pnlCleanCount->Visible == false);
        }
    }

    ht9045::W906_Timer8SetClock(0);
    std::printf("%s: %d/%d checks passed (sandbox %s)\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total, sandbox.c_str());
    return g_fail ? 1 : 0;
}
