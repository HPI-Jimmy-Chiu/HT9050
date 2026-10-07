// =============================================================================
//  test_st02_n07_alarm.cpp -- ctest St02_N07Alarm.  AI(W906-C15-N07) 20261003 (St02-E helper), card ST02-C15.
//
//  The N07 SECS/GEM disconnect alarm (JSCC NetworkMonitor, Steven 20260603), golden 906_0625_Steven:
//    main.cpp:20937-20999 the Timer2 block + :20848-20860 WriteN07Log   -> SECSGEM/N07Alarm_St02.cpp
//    main.dfm:17279-17284 Timer2 (1000 ms), FormShow :10179 enable      -> MainTimersSt02.cpp Timer2 slot
//    ckernel.cpp:777-781 / :875-882,926 ShowRunLed, :2128 / :2137 ScanPannelKey -> port ckernel.cpp claims
//
//  HGem is a REAL THGem (its constructor opens no socket, SecsTagPublish.cpp:42-49); "the Host link" is its public
//  bConnect, read through the real THGem::IsConnect (uHGemEquipment.cpp:3931).  The clock is the dispatcher slot's own
//  `now` argument (W906_St02Timer2N07TickAt), so no test sleeps.
//
//    0. containment first: the cMyDB log roots and W906_SECSGEMLOG_ROOT must be ctest's machine_log_scratch
//    1. connected: nothing happens, no NetworkMonitor.log
//    2. the first disconnected tick turns the alarm on at once, one "SECS/GEM Connection Lost" line (golden bytes)
//    3. while on: the link is checked only on every 10th tick -- back on tick 1..9 = still on, tick 10 = released,
//       one "SECS/GEM Connection Alarm Released" line, bAlarmBuzzer and bN07BuzzerSilenced cleared
//    4. still down on the 10th tick: stays on, counter back to 0, the next check is 10 ticks later
//    5. off when [N07] SECS GEM Alarm / Enable SECS GEM / ON-LINE / running / HGem is missing; turning one off while
//       the alarm is on releases it (falling edge: the Released line, buzzer cleared)
//    6. ShowRunLed: running + alarm = LED_Message and the buzzer; silenced = LED_Message, no buzzer; the red tower light
//       follows FlushFlag (blinks), green and yellow off; alarm off = LED_Running
//    7. ScanPannelKey Alarm Reset (rear and front sensor of the front pad) silences the buzzer, the light keeps
//       blinking; the release clears the silence; Alarm Reset without the alarm does not set it
//    8. the Timer2 slot on a fake clock: latched by InitialOK, 1000 ms, first fire one Interval after the latch,
//       not before InitialOK, nothing after bSystemClose; trigger on the first fire, released 10 fires (10 s) later
//    9. the log only in the scratch root; the machine's D:\SECS_GEM_LOGS file for today is untouched (read only)
//   10. the hook-up lines are code, not comment (argv[1] = the tree root, read only)
//  Memory, SW[] outputs and files under machine_log_scratch only; the scratch folder is removed on a green run.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "mysensor.h"
#include "myswitch.h"
#include "ckernel.h"                 // ScanPannelKey
#include "cpublic.h"                 // GetTimeInfo
#include "forms/fMain.h"             // fMain->ledRed / ledGreen / ledYellow
#include "forms/fNote.h"             // fNote (ShowRunLed reads fShow)
#include "SECSGEM/uHGemEquipment.h"  // THGem, HGem
#include "SECSGEM/N07Alarm_St02.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

void ShowRunLed();                                        // ckernel.cpp:1604 (golden ckernel.cpp:704); not in ckernel.h
extern bool SECS_GEM_PPMUSIC_CONTROL_flag;                // ckernel.cpp:1272
extern bool SECS_GEM_PPSIGNALTOWER_CONTROL_flag;          // ckernel.cpp:1275

namespace ht9045 {
void W906_St02TimersReset();                              // MainTimersSt02.cpp (ctest only)
void W906_St02Timer2N07TickAt(unsigned long now);         // MainTimersSt02.cpp (ctest only): the Timer2 slot alone
void W906_St02TimersCountsT2(unsigned long* t2, bool* on);
}

static int g_total = 0, g_fail = 0;
#define CHECK(c) do { ++g_total; if (!(c)) { ++g_fail; std::printf("  FAIL line %d: %s\n", __LINE__, #c); } } while (0)

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
static int Count(const std::string& s, const std::string& needle)
{
    int n = 0;
    if (needle.empty()) return 0;
    for (size_t a = s.find(needle); a != std::string::npos; a = s.find(needle, a + 1)) ++n;
    return n;
}
// Code only: // and /* */ comments removed.  String / char state is reset at every line end, so an apostrophe in a
// golden #if 0 line cannot swallow the rest of the file; block comments may span lines.
static std::string CodeOnly(const std::string& s)
{
    std::string o;
    bool str = false, chr = false, block = false;
    for (size_t i = 0; i < s.size(); ++i)
    {
        const char c = s[i], d = i + 1 < s.size() ? s[i + 1] : '\0';
        if (c == '\n') { str = chr = false; o += c; continue; }
        if (block) { if (c == '*' && d == '/') { block = false; ++i; } continue; }
        if (str) { o += c; if (c == '\\' && d && d != '\n') { o += d; ++i; } else if (c == '"') str = false; continue; }
        if (chr) { o += c; if (c == '\\' && d && d != '\n') { o += d; ++i; } else if (c == '\'') chr = false; continue; }
        if (c == '/' && d == '/') { while (i + 1 < s.size() && s[i + 1] != '\n') ++i; continue; }
        if (c == '/' && d == '*') { block = true; ++i; continue; }
        if (c == '"') str = true;
        if (c == '\'') chr = true;
        o += c;
    }
    return o;
}
// Drops every `#if 0` ... matching `#endif` region (nested #if counted), so a pin can tell live code from gated text.
static std::string DropIf0(const std::string& s)
{
    std::string o;
    int skip = 0;
    size_t a = 0;
    while (a < s.size())
    {
        size_t e = s.find('\n', a);
        if (e == std::string::npos) e = s.size(); else ++e;
        const std::string ln = s.substr(a, e - a);
        size_t k = ln.find_first_not_of(" \t");
        const std::string t = (k == std::string::npos) ? std::string() : ln.substr(k);
        if (skip == 0)
        {
            if (t.compare(0, 5, "#if 0") == 0) skip = 1; else o += ln;
        }
        else
        {
            if (t.compare(0, 3, "#if") == 0) ++skip;
            else if (t.compare(0, 6, "#endif") == 0) --skip;
        }
        a = e;
    }
    return o;
}
static std::string LineWith(const std::string& s, const std::string& needle)
{
    const size_t a = s.find(needle);
    if (a == std::string::npos) return std::string();
    const size_t b = s.rfind('\n', a);
    const size_t e = s.find('\n', a);
    return s.substr(b == std::string::npos ? 0 : b + 1, (e == std::string::npos ? s.size() : e) - (b == std::string::npos ? 0 : b + 1));
}
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

// The file WriteN07Log wrote for the CURRENT SystemYear / Month / Date (NewRecordProcess may refresh them first), and
// the line it wrote (golden :20856-20857 format; G1: text-mode fopen turns the "\r\n" into "\r\r\n" on disk).
static std::string g_root;
static std::string LogPathNow()
{
    char b[64];
    std::snprintf(b, sizeof(b), "\\%04d\\%02d_%02d\\NetworkMonitor.log", (int)SystemYear, (int)SystemMonth, (int)SystemDate);
    return g_root + b;
}
static std::string LogLineNow(const char* ev)
{
    char b[160];
    std::snprintf(b, sizeof(b), "%04d/%02d/%02d %02d:%02d:%02d  %s\r\r\n", (int)SystemYear, (int)SystemMonth, (int)SystemDate,
                  (int)SystemHour, (int)SystemMin, (int)SystemSec, ev);
    return b;
}
static bool FileState(const std::string& p, DWORD* sizeLow, FILETIME* wt)
{
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (!::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &a)) return false;
    *sizeLow = a.nFileSizeLow;
    *wt = a.ftLastWriteTime;
    return true;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

// Sensor seam, as tests/test_w7_l2_ckernel.cpp:496-510 (no card: TYPE_A reads OFF, TYPE_B reads ON, Enable=false neither).
static void SenOff(int i)     { Sen[i].Enable = true; Sen[i].Type = TYPE_A; Sen[i].ISABase = eISABase; }
static void SenOn(int i)      { Sen[i].Enable = true; Sen[i].Type = TYPE_B; Sen[i].ISABase = eISABase; }
static void SenUnknown(int i) { Sen[i].Enable = false; }
static bool s_en[MAX_SENSOR_ITEM];
static int  s_ty[MAX_SENSOR_ITEM], s_isa[MAX_SENSOR_ITEM];

// Every key the front pad reads unwired, the ASE soft keys clear, the front pad selected, no safety lock
// (test_w7_l2_ckernel.cpp allKeysUnknown / clearAseFlags / safeLockRelease), then one call drains the key latch.
static void KeysIdle()
{
    for (int i = 0; i < 32; ++i) SenUnknown(i);
    SenOff(SnRearPadActive);                              // bFrontPadActive = Sen[SnRearPadActive].IsOff() = true
    bAseReset = bAsePause = bAseHome = bAseStart = bAseOneCycle = bAseRetry = false;
    bAseSKIP = bAseCleanOut = bAseTrayFeed = bAseTrayEnd = bAseAlarmReset = false;
    ScanPannelKey();
}

static THGem* g_gem = 0;
static void Tick() { ht9045::W906_N07Timer2Tick_St02(); }
static void Arm(bool connected)
{
    ht9045::W906_N07Timer2Reset_St02();
    IniConfig.bEnable_SECS_GEM = true;
    IniConfig.bN07_Alarm = true;
    LastSet.iTester = ON_LINE;
    SystemStart = true;
    HGem = g_gem;
    g_gem->bConnect = connected;
    bAlarmBuzzer = false;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("St02_N07Alarm\n");
    std::printf(" 0. containment first\n");
    if (!W906TestInsideCtestRoots("St02_N07Alarm"))
        return 2;
    {
        const char* r = std::getenv("W906_SECSGEMLOG_ROOT");
        std::printf("  W906_SECSGEMLOG_ROOT = %s\n", r ? r : "(unset)");
        if (r == 0 || *r == '\0' || Lower(r).find("machine_log_scratch") == std::string::npos)
        {
            std::printf("  ABORT: W906_SECSGEMLOG_ROOT is not ctest's machine_log_scratch (run it with ctest -R St02_N07Alarm) -- nothing was called\n");
            return 2;
        }
        g_root = r;
        for (size_t i = 0; i < g_root.size(); ++i) if (g_root[i] == '/') g_root[i] = '\\';
        RemoveTree(g_root);                               // a fresh root: earlier runs' lines must not count
    }
    GetTimeInfo();                                        // the clock globals start as the 9999 sentinel (cmydef.cpp:291-292)
    // The machine's own file for today (golden's literal root), read only: it must not change.
    SYSTEMTIME st0;
    ::GetLocalTime(&st0);
    char realBuf[96];
    std::snprintf(realBuf, sizeof(realBuf), "D:\\SECS_GEM_LOGS\\%04d\\%02d_%02d\\NetworkMonitor.log", (int)st0.wYear, (int)st0.wMonth, (int)st0.wDay);
    const std::string realPath = realBuf;
    DWORD realSize0 = 0; FILETIME realWt0 = { 0, 0 };
    const bool realThere0 = FileState(realPath, &realSize0, &realWt0);

    // save what this test touches
    THGem* const savedHGem = HGem;
    const bool sEnSecs = IniConfig.bEnable_SECS_GEM, sN07 = IniConfig.bN07_Alarm;
    const int sTester = LastSet.iTester, sTemp = LastSet.iTemperature, sRsm = LastSet.iRunStartMode;
    const bool sStart = SystemStart, sInit = InitialOK, sSysInit = SystemInitialOK, sClose = bSystemClose;
    const bool sBuzz = bAlarmBuzzer, sFlush = FlushFlag, sFront = bFrontPadActive, sEmp = bEnableEmployeeIDCheck;
    const bool sPPm = SECS_GEM_PPMUSIC_CONTROL_flag, sPPt = SECS_GEM_PPSIGNALTOWER_CONTROL_flag;
    const bool sMusic = bNeedMusicAndAlarmOn, sPause = bTesterPauseMusic, sPLC = Enable_PLCSafety_IO;
    const int sRun = RunState;
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) { s_en[i] = Sen[i].Enable; s_ty[i] = Sen[i].Type; s_isa[i] = Sen[i].ISABase; }

    g_gem = new THGem();

    std::printf(" 1. connected: nothing happens\n");
    {
        Arm(true);
        for (int i = 0; i < 25; ++i) Tick();
        CHECK(bN07AlarmActive == false && bN07BuzzerSilenced == false && bAlarmBuzzer == false);
        CHECK(ReadAll(LogPathNow()).empty());
    }

    std::printf(" 2. the first disconnected tick: on at once, one Lost line\n");
    std::string lostPath, lostLine;
    {
        Arm(true);
        Tick();
        CHECK(bN07AlarmActive == false);
        g_gem->bConnect = false;
        Tick();                                           // golden :20954-20960: no delay
        CHECK(bN07AlarmActive == true);
        CHECK(ht9045::W906_N07SecCounter_St02() == 0);
        lostPath = LogPathNow();
        lostLine = LogLineNow("SECS/GEM Connection Lost");
        const std::string f = ReadAll(lostPath);
        CHECK(f == lostLine);
        if (f != lostLine) std::printf("  got [%s] (%u bytes) in %s\n", f.c_str(), (unsigned)f.size(), lostPath.c_str());
        Tick();                                           // edge only: no second Lost line
        CHECK(ReadAll(lostPath) == lostLine);
    }

    std::printf(" 3. link back: still on for ticks 1..9, released on tick 10\n");
    std::string relPath, relLine;
    {
        // state from section 2: on, one tick after the trigger (counter 1)
        CHECK(bN07AlarmActive == true && ht9045::W906_N07SecCounter_St02() == 1);
        g_gem->bConnect = true;
        bN07BuzzerSilenced = true;                        // as Alarm Reset leaves it
        bAlarmBuzzer = true;                              // as ShowRunLed leaves it
        for (int i = 2; i <= 9; ++i) Tick();              // counter 2..9
        CHECK(bN07AlarmActive == true && ht9045::W906_N07SecCounter_St02() == 9);
        Tick();                                           // counter 10: checked, link back -> released
        CHECK(bN07AlarmActive == false);
        CHECK(bAlarmBuzzer == false && bN07BuzzerSilenced == false);   // golden :20989-20990
        relPath = LogPathNow();
        relLine = LogLineNow("SECS/GEM Connection Alarm Released");
        const std::string want = (relPath == lostPath) ? lostLine + relLine : relLine;
        const std::string f = ReadAll(relPath);
        CHECK(f == want);
        if (f != want) std::printf("  got [%s] in %s\n", f.c_str(), relPath.c_str());
        Tick();                                           // edge only
        CHECK(ReadAll(relPath) == want);
    }

    std::printf(" 4. still down on the 10th tick: stays on, the next check 10 ticks later\n");
    {
        Arm(false);
        Tick();                                           // trigger
        CHECK(bN07AlarmActive == true);
        for (int i = 1; i <= 10; ++i) Tick();             // 10th: checked, still down
        CHECK(bN07AlarmActive == true && ht9045::W906_N07SecCounter_St02() == 0);
        g_gem->bConnect = true;
        for (int i = 1; i <= 9; ++i) Tick();
        CHECK(bN07AlarmActive == true);
        Tick();                                           // 20th tick after the trigger
        CHECK(bN07AlarmActive == false);
    }

    std::printf(" 5. off when a condition is missing; turning one off releases\n");
    {
        struct Off { const char* what; int k; } offs[] = {
            { "[N07] SECS GEM Alarm off", 0 }, { "Enable SECS GEM off", 1 }, { "OFF-LINE", 2 }, { "not running", 3 }, { "HGem NULL", 4 } };
        for (int c = 0; c < 5; ++c)
        {
            Arm(false);
            switch (offs[c].k)
            {
                case 0: IniConfig.bN07_Alarm = false; break;
                case 1: IniConfig.bEnable_SECS_GEM = false; break;
                case 2: LastSet.iTester = OFF_LINE; break;
                case 3: SystemStart = false; break;
                case 4: HGem = NULL; break;
            }
            for (int i = 0; i < 12; ++i) Tick();
            const bool ok = (bN07AlarmActive == false);
            CHECK(ok);
            std::printf("  %-26s -> %s\n", offs[c].what, ok ? "off" : "ON (wrong)");
        }
        // falling edge by a condition: on, then the operator stops the machine
        Arm(false);
        Tick();
        CHECK(bN07AlarmActive == true);
        bN07BuzzerSilenced = true;
        bAlarmBuzzer = true;
        const std::string before = ReadAll(LogPathNow());
        SystemStart = false;
        Tick();
        CHECK(bN07AlarmActive == false && bAlarmBuzzer == false && bN07BuzzerSilenced == false);
        const std::string after = ReadAll(LogPathNow());
        const std::string rel = LogLineNow("SECS/GEM Connection Alarm Released");
        CHECK(after.size() > before.size() && after.size() >= rel.size() && after.compare(after.size() - rel.size(), std::string::npos, rel) == 0);
    }

    std::printf(" 6. ShowRunLed: buzzer, silence, the red blink\n");
    {
        CHECK(fMain != 0 && fNote != 0);
        SECS_GEM_PPMUSIC_CONTROL_flag = false;
        SECS_GEM_PPSIGNALTOWER_CONTROL_flag = false;      // golden :849: else the SECS host owns the tower
        bNeedMusicAndAlarmOn = false;
        Enable_PLCSafety_IO = false;
        LastSet.iTemperature = Tempture_Ambient;          // golden :766: no heating state
        LastSet.iRunStartMode = 0;                        // not rsmAutoRetest (:753)
        for (int i = 0; i < MAX_SENSOR_ITEM; ++i) SenUnknown(i);   // no EMG (IsEMGPressed), no key
        SystemStart = true;
        LastSet.iTester = ON_LINE;
        bN07AlarmActive = false;
        ShowRunLed();                                     // prime: afterwards ShowRunLed's OldFlushFlag == FlushFlag (golden :843-847)
        bN07AlarmActive = true;
        bN07BuzzerSilenced = false;
        bAlarmBuzzer = false;
        FlushFlag = !FlushFlag;                           // every toggle below is an edge, so the LED part runs
        ShowRunLed();
        CHECK(RunState == LED_Message && bAlarmBuzzer == true);    // golden :777-781
        CHECK(fMain->ledRed->Value == FlushFlag && fMain->ledGreen->Value == false && fMain->ledYellow->Value == false);   // :875-880
        CHECK(SW[SwTowerRed].OutValue == FlushFlag && SW[SwTowerGreen].OutValue == false && SW[SwTowerYellow].OutValue == false);
        const bool f1 = FlushFlag;
        FlushFlag = !FlushFlag;
        ShowRunLed();
        CHECK(FlushFlag != f1 && fMain->ledRed->Value == FlushFlag && SW[SwTowerRed].OutValue == FlushFlag);   // blinks
        bN07BuzzerSilenced = true;
        bAlarmBuzzer = false;
        FlushFlag = !FlushFlag;
        ShowRunLed();
        CHECK(RunState == LED_Message && bAlarmBuzzer == false);   // silenced: no buzzer ...
        CHECK(fMain->ledRed->Value == FlushFlag && SW[SwTowerRed].OutValue == FlushFlag);   // ... and still blinking
        bN07AlarmActive = false;
        bN07BuzzerSilenced = false;
        FlushFlag = !FlushFlag;
        ShowRunLed();
        CHECK(RunState == LED_Running);                   // golden :782-785
    }

    std::printf(" 7. ScanPannelKey Alarm Reset silences the buzzer, the light keeps blinking\n");
    {
        SystemInitialOK = true;
        bEnableEmployeeIDCheck = false;
        const int keys[2] = { SnRKAlarmReset, SnFKAlarmReset };    // port ckernel.cpp:3369 / :3377 (golden :2122 / :2131)
        for (int k = 0; k < 2; ++k)
        {
            Arm(false);
            Tick();
            CHECK(bN07AlarmActive == true && bN07BuzzerSilenced == false);
            KeysIdle();
            CHECK(bFrontPadActive == true);
            SenOn(keys[k]);
            ScanPannelKey();
            CHECK(bN07BuzzerSilenced == true);            // golden :2128 / :2137
            KeysIdle();
            bAlarmBuzzer = false;
            FlushFlag = !FlushFlag;
            ShowRunLed();
            CHECK(bAlarmBuzzer == false && RunState == LED_Message);
            CHECK(fMain->ledRed->Value == FlushFlag && SW[SwTowerRed].OutValue == FlushFlag);
            g_gem->bConnect = true;
            for (int i = 0; i < 10; ++i) Tick();          // released on the 10th
            CHECK(bN07AlarmActive == false && bN07BuzzerSilenced == false);
        }
        // Alarm Reset with no N07 alarm: the silence flag is not set
        Arm(true);
        Tick();
        KeysIdle();
        SenOn(SnFKAlarmReset);
        ScanPannelKey();
        CHECK(bN07AlarmActive == false && bN07BuzzerSilenced == false);
        KeysIdle();
    }

    std::printf(" 8. the Timer2 slot on a fake clock\n");
    {
        unsigned long n = 0; bool on = false;
        ht9045::W906_St02TimersReset();
        Arm(false);
        bSystemClose = false;
        InitialOK = false;
        ht9045::W906_St02Timer2N07TickAt(1000);           // AI(W906-ST02-MRB) 20261004: fMain->Timer2 in the timer table, on from the bind
        ht9045::W906_St02Timer2N07TickAt(5000);           //   (golden FormShow :10179); its grid starts at the first tick: due 2000, late here
        ht9045::W906_St02TimersCountsT2(&n, &on);
        CHECK(n == 0 && on == true && bN07AlarmActive == false);   // golden :20877: InitialOK false -> nothing (next due 6000)
        InitialOK = true;
        ht9045::W906_St02Timer2N07TickAt(5500);
        ht9045::W906_St02TimersCountsT2(&n, &on);
        CHECK(n == 0 && on == true && bN07AlarmActive == false);
        ht9045::W906_St02Timer2N07TickAt(6000);           // the first Timer2 tick with InitialOK: disconnected -> on at once
        ht9045::W906_St02TimersCountsT2(&n, 0);
        CHECK(n == 1 && bN07AlarmActive == true);
        g_gem->bConnect = true;                           // the cable goes back in right after
        unsigned long t = 6000;
        while (t < 15500) { t += 500; ht9045::W906_St02Timer2N07TickAt(t); }   // 500 ms beats to 15.5 s
        ht9045::W906_St02TimersCountsT2(&n, 0);
        CHECK(n == 10 && bN07AlarmActive == true);
        ht9045::W906_St02Timer2N07TickAt(16000);          // the 10th tick after the trigger = 10 s
        ht9045::W906_St02TimersCountsT2(&n, 0);
        CHECK(n == 11 && bN07AlarmActive == false);
        bSystemClose = true;                              // golden FormClose :11704 Timer2->Enabled=false
        g_gem->bConnect = false;
        for (t = 16500; t <= 25000; t += 500) ht9045::W906_St02Timer2N07TickAt(t);
        ht9045::W906_St02TimersCountsT2(&n, 0);
        CHECK(n == 11 && bN07AlarmActive == false);
        bSystemClose = false;
        ht9045::W906_St02TimersReset();
    }

    std::printf(" 9. the log only in the scratch root\n");
    {
        CHECK(Lower(lostPath).find("machine_log_scratch") != std::string::npos);
        DWORD realSize1 = 0; FILETIME realWt1 = { 0, 0 };
        const bool realThere1 = FileState(realPath, &realSize1, &realWt1);
        CHECK(realThere1 == realThere0);
        if (realThere0 && realThere1)
            CHECK(realSize1 == realSize0 && realWt1.dwLowDateTime == realWt0.dwLowDateTime && realWt1.dwHighDateTime == realWt0.dwHighDateTime);
        std::printf("  %s: %s\n", realPath.c_str(), realThere1 ? "present, unchanged" : "absent, still absent");
    }

    std::printf(" 10. the hook-up lines are code (argv[1] = the tree root, read only)\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string ck = CodeOnly(ReadAll(root + "/ckernel.cpp"));
        const std::string md = CodeOnly(ReadAll(root + "/cmydef.h") + "\n" + ReadAll(root + "/cmydef_core.h") + "\n" + ReadAll(root + "/cmydef_io.h") + "\n" + ReadAll(root + "/cmydef_rt.h"));   // AI(W906-S1-CMYDEF) 20261007 laptop: cmydef.h is split (Ifor01 !302) -- the header family, each extern still exactly once
        const std::string ds = CodeOnly(ReadAll(root + "/MainTimersSt02.cpp"));
        const std::string na = CodeOnly(ReadAll(root + "/SECSGEM/N07Alarm_St02.cpp"));
        const std::string cm = ReadAll(root + "/CMakeLists.txt");
        CHECK(!ck.empty() && !md.empty() && !ds.empty() && !na.empty() && !cm.empty());
        CHECK(Count(ck, "else if(bN07AlarmActive) { RunState=LED_Message; if(!bN07BuzzerSilenced) bAlarmBuzzer=true; }  else") == 1);
        CHECK(Count(ck, "if(bN07AlarmActive) { fMain->ledRed->Value=FlushFlag; fMain->ledGreen->Value=false; fMain->ledYellow->Value=false; } else {") == 1);
        CHECK(Count(ck, "if(bN07AlarmActive) bN07BuzzerSilenced=true;") == 2);
        CHECK(Count(md, "extern bool bN07AlarmActive;") == 1 && Count(md, "extern bool bN07BuzzerSilenced;") == 1);
        CHECK(Count(ds, "W906_N07Timer2Tick_St02();") == 2);   // the declaration + the OnTimer's call
        CHECK(Count(ds, "Timer2N07Tick(now);") == 0 && Count(ds, "&St02OnTimer2N07") == 1 &&
              Count(ds, "fMain->Timer2->Enabled = true;") == 1);   // AI(W906-ST02-MRB) 20261004: a table OnTimer (fMain->Timer2), on from the bind (golden FormShow :10179)
        CHECK(Count(na, "WriteN07Log(\"SECS/GEM Connection Lost\");") == 1 && Count(na, "WriteN07Log(\"SECS/GEM Connection Alarm Released\");") == 1);
        const std::string live = DropIf0(na);
        CHECK(Count(live, "Off_lineDisplay") == 0 && Count(live, "palMainStatus->") == 0);   // N3: the screen part stays gated ...
        CHECK(Count(na, "Off_lineDisplay->BorderWidth = 20;") == 1 && Count(na, "palMainStatus->Left=3;") == 1);   // ... golden's text kept under the gate
        CHECK(Count(live, "fMain->LoadTestModePicture();") == 1 && Count(live, "HGem->IsConnect()==false") == 1 && Count(live, "HGem->IsConnect()==true") == 1);
        const std::string cl = LineWith(cm, "SECSGEM/N07Alarm_St02.cpp");
        CHECK(!cl.empty() && cl.find("SECSGEM/N07Alarm_St02.cpp") < cl.find("#") && cl.find("MainTimer3.cpp") < cl.find("SECSGEM/N07Alarm_St02.cpp"));   // AI(W906-ST02-SKC) 20261004 (St02-E, NB2-1 R209): on MainTimer3.cpp's line, after it, before the comment (main put BinDisplay/BinDispBringUp_St02.cpp between them)
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    // restore
    for (int i = 0; i < MAX_SENSOR_ITEM; ++i) { Sen[i].Enable = s_en[i]; Sen[i].Type = s_ty[i]; Sen[i].ISABase = s_isa[i]; }
    ht9045::W906_N07Timer2Reset_St02();
    HGem = savedHGem;
    delete g_gem;
    g_gem = 0;
    IniConfig.bEnable_SECS_GEM = sEnSecs; IniConfig.bN07_Alarm = sN07;
    LastSet.iTester = sTester; LastSet.iTemperature = sTemp; LastSet.iRunStartMode = sRsm;
    SystemStart = sStart; InitialOK = sInit; SystemInitialOK = sSysInit; bSystemClose = sClose;
    bAlarmBuzzer = sBuzz; FlushFlag = sFlush; bFrontPadActive = sFront; bEnableEmployeeIDCheck = sEmp;
    SECS_GEM_PPMUSIC_CONTROL_flag = sPPm; SECS_GEM_PPSIGNALTOWER_CONTROL_flag = sPPt;
    bNeedMusicAndAlarmOn = sMusic; bTesterPauseMusic = sPause; Enable_PLCSafety_IO = sPLC;
    RunState = sRun;

    if (g_fail == 0)
        RemoveTree(g_root);
    else
        std::printf("  kept for inspection: %s\n", g_root.c_str());
    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
