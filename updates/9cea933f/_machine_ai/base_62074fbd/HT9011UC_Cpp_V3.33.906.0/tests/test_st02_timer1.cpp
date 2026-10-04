// =============================================================================
//  tests/test_st02_timer1.cpp -- E-T1-022: golden TfMain::Timer1Timer's NUMBER_PANEL_TYPE==2 bin-number panel
//  (golden 906_0625_Steven main.cpp:3439-3440 fShowBinSelect->DoShowBinDigital()) in St02's dispatcher, OPTION A
//  (MainTimersSt02.cpp Timer1BinTick: every pass, golden's 30 ms deadline and Timer1's guards).
//  AI(W906-ET1022) 20261001 (St02-E).  Suite: St02_Timer1 (both configs).  Memory only (SW[] outputs, no file).
//
//  CONTAINMENT FIRST (st02_test_containment.h).
//    1. the rate on a fake clock: 10 ms steps over 1 s -> golden's 30 ms grid (33 +- 1 calls, never two < 20 ms apart);
//       500 ms steps (the PumpTick beat, option A) -> one call per step.
//    2. only NUMBER_PANEL_TYPE == 2 (types 0 / 1 / 3 / 4: no call -- Timer3 G7 drives those).
//    3. golden Timer1's guards, each alone: InitialOK false, SystemInitialOK false, an S10F3 alarm (bSECSGEMAlarm without
//       bSECSGEM_NoteAlarm), bSystemClose -> no call; the S10F3 alarm with bSECSGEM_NoteAlarm -> calls.
//    4. what one call does: task 1 -> all 12 SW[SwLoaderBin+i] on, task 100 (cShowBinSelect.cpp case 1).
//    5. TIMING (the laptop's question b, 20261001 20:0x): QueryPerformanceCounter around 2000 direct calls of
//       fShowBinSelect->DoShowBinDigital() through its states -> prints "[timing] ..." (avg / max microseconds).  Not a
//       pass / fail check: St01 reports the number.
//    6. the hook-up line is code (argv[1] = tree root): MainTimersSt02.cpp calls Timer1BinTick(now) once, comment-stripped.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "myswitch.h"
#include "forms/fShowBinSelect.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <string>
#include <vector>

namespace ht9045 {
void W906_St02TimersReset();
void W906_St02TimersCountsT1(unsigned long* t1);
void W906_St02Timer1BinTickAt(unsigned long now);
}

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static unsigned long Calls()
{
    unsigned long n = 0;
    ht9045::W906_St02TimersCountsT1(&n);
    return n;
}

static void Base()   // type 2, every guard open, the panel idle at task 1
{
    ht9045::W906_St02TimersReset();
    NUMBER_PANEL_TYPE = 2;
    InitialOK = true;
    SystemInitialOK = true;
    bSECSGEMAlarm = false;
    bSECSGEM_NoteAlarm = false;
    bSystemClose = false;
    fShowBinSelect->bUpdateBinDigital = false;
    fShowBinSelect->iShowBinDigitalTask = 1;
}

// ---- source check helpers (comments stripped, string literals respected) ----
static std::string ReadAll(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return std::string();
    std::string s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}
static std::string StripComments(const std::string& s)
{
    std::string o;
    bool str = false, chr = false, line = false, block = false;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i], d = i + 1 < s.size() ? s[i + 1] : '\0';
        if (line) { if (c == '\n') { line = false; o += c; } continue; }
        if (block) { if (c == '*' && d == '/') { block = false; ++i; } continue; }
        if (str) { o += c; if (c == '\\' && d) { o += d; ++i; } else if (c == '"') str = false; continue; }
        if (chr) { o += c; if (c == '\\' && d) { o += d; ++i; } else if (c == '\'') chr = false; continue; }
        if (c == '/' && d == '/') { line = true; ++i; continue; }
        if (c == '/' && d == '*') { block = true; ++i; continue; }
        if (c == '"') str = true;
        if (c == '\'') chr = true;
        o += c;
    }
    return o;
}
static int Count(const std::string& s, const std::string& needle)
{
    int n = 0;
    for (size_t a = s.find(needle); a != std::string::npos; a = s.find(needle, a + 1)) ++n;
    return n;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("St02_Timer1\n");
    if (!W906TestInsideCtestRoots("St02_Timer1"))
        return 2;
#ifdef SOFT_SIMULTE
    const char* const build = "SIM";
#else
    const char* const build = "SHIP";
#endif
    std::printf("  build: %s\n", build);
    CHECK(fShowBinSelect != 0);
    if (fShowBinSelect == 0) return 1;
    const int savedType = NUMBER_PANEL_TYPE;
    const bool savedInit = InitialOK, savedSys = SystemInitialOK, savedSecs = bSECSGEMAlarm, savedNote = bSECSGEM_NoteAlarm;
    const bool savedClose = bSystemClose;
    const unsigned long t0 = 500000;

    std::printf(" 1. the rate\n");
    {
        Base();
        std::vector<unsigned long> at;
        unsigned long last = 0;
        for (unsigned long t = t0; t <= t0 + 1000; t += 10)
        {
            ht9045::W906_St02Timer1BinTickAt(t);
            if (Calls() != last) { at.push_back(t); last = Calls(); }
        }
        bool spaced = true;
        for (size_t i = 1; i < at.size(); ++i) spaced = spaced && at[i] - at[i - 1] >= 20;
        CHECK(at.size() >= 32 && at.size() <= 34 && spaced);         // golden Timer1's 30 ms grid
        CHECK(!at.empty() && at[0] == t0 + 30);                      // the first fire one Interval after the start
        Base();
        for (int k = 0; k <= 10; ++k) ht9045::W906_St02Timer1BinTickAt(t0 + 500u * k);   // option A: the PumpTick beat
        CHECK(Calls() == 10);                                        // one per 500 ms pass after the start
    }

    std::printf(" 2. only type 2\n");
    {
        const int other[] = { 0, 1, 3, 4 };
        for (int k = 0; k < 4; ++k)
        {
            Base();
            NUMBER_PANEL_TYPE = other[k];
            for (unsigned long t = t0; t <= t0 + 300; t += 10) ht9045::W906_St02Timer1BinTickAt(t);
            char what[64];
            std::snprintf(what, sizeof(what), "type %d: no call", other[k]);
            check(Calls() == 0, what, __LINE__);
        }
    }

    std::printf(" 3. golden Timer1's guards\n");
    {
        for (int g = 0; g < 5; ++g)
        {
            Base();
            if (g == 0) InitialOK = false;
            if (g == 1) SystemInitialOK = false;
            if (g == 2) bSECSGEMAlarm = true;
            if (g == 3) bSystemClose = true;
            if (g == 4) { bSECSGEMAlarm = true; bSECSGEM_NoteAlarm = true; }
            for (unsigned long t = t0; t <= t0 + 300; t += 10) ht9045::W906_St02Timer1BinTickAt(t);
            static const char* const names[] = { "InitialOK false", "SystemInitialOK false", "S10F3 alarm", "bSystemClose",
                                                 "S10F3 alarm with the Note box (golden goes on)" };
            char what[96];
            std::snprintf(what, sizeof(what), "%s: %s", names[g], g == 4 ? "calls" : "no call");
            check(g == 4 ? Calls() > 0 : Calls() == 0, what, __LINE__);
        }
    }

    std::printf(" 4. one call = one step of the panel machine\n");
    {
        Base();
        fShowBinSelect->bUpdateBinDigital = true;
        for (int i = 0; i < 12; ++i) SW[SwLoaderBin + i].Off();
        ht9045::W906_St02Timer1BinTickAt(t0);
        ht9045::W906_St02Timer1BinTickAt(t0 + 30);
        bool allOn = true;
        for (int i = 0; i < 12; ++i) allOn = allOn && SW[SwLoaderBin + i].OutValue;
        CHECK(Calls() == 1 && allOn && fShowBinSelect->iShowBinDigitalTask == 100);   // cShowBinSelect.cpp case 1
        for (int i = 0; i < 12; ++i) SW[SwLoaderBin + i].Off();
    }

    std::printf(" 5. timing (the laptop's question b; St01 reports the number)\n");
    {
        Base();
        LARGE_INTEGER f, a, b;
        ::QueryPerformanceFrequency(&f);
        double sum = 0, mx = 0;
        const int n = 2000;
        for (int k = 0; k < n; ++k)
        {
            if (k % 200 == 0) { fShowBinSelect->bUpdateBinDigital = true; fShowBinSelect->iShowBinDigitalTask = 1; }
            ::QueryPerformanceCounter(&a);
            fShowBinSelect->DoShowBinDigital();
            ::QueryPerformanceCounter(&b);
            const double us = (double)(b.QuadPart - a.QuadPart) * 1e6 / (double)f.QuadPart;
            sum += us;
            if (us > mx) mx = us;
        }
        std::printf("  [timing] DoShowBinDigital (%s build): avg %.2f us, max %.2f us over %d calls\n", build, sum / n, mx, n);
        for (int i = 0; i < 12; ++i) SW[SwLoaderBin + i].Off();
        CHECK(true);
    }

    std::printf(" 6. the hook-up line is code\n");
    if (argc > 1)
    {
        const std::string ds = StripComments(ReadAll(std::string(argv[1]) + "/MainTimersSt02.cpp"));
        CHECK(!ds.empty());
        CHECK(Count(ds, "Timer1BinTick(now);") == 2);                // the dispatcher's call + the ctest entry's
        CHECK(Count(ds, "fShowBinSelect->DoShowBinDigital();") == 1);
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    NUMBER_PANEL_TYPE = savedType;
    InitialOK = savedInit; SystemInitialOK = savedSys; bSECSGEMAlarm = savedSecs; bSECSGEM_NoteAlarm = savedNote;
    bSystemClose = savedClose;
    fShowBinSelect->bUpdateBinDigital = false;
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
