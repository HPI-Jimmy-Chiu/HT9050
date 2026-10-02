// =============================================================================
//  test_ela_timedata.cpp -- ★W48-2 = B: the Handler's hourly TimeData record and the ELA reading it back.
//
//  AI(W906-ELA-W48B) 20260928 (St02-E helper).  Suite name (add_test): ELA_TimeData
//  Steven 0928 09:3x W48-2 = B: split idle vs off by the software-on time, which V906 records the golden way --
//  RecordTimeData (golden 906_0625_Steven cMyDB.cpp:339-498, V906 cMyDB.cpp:479) hourly, golden HS_Function.cpp:233-238
//  (TFormHS::TimerAutoBackupTimer), V906 TesterComm/Handler/TesterCommWiring.cpp W906_TimeDataHourTick.
//    0. containment first (the tests/test_tcp_cmd_server.cpp b12ab375 rule): before ANY Handler code, refuse (exit 2)
//       unless the four cMyDB log roots and the ctest redirect variables are ctest's machine_log_scratch sandbox; then
//       point as9045LogPath / asSaveEventLogPath at %TEMP%\ht9045_ela_timedata_<tick> (refused under D:\HT9045*), so
//       every file this test makes is there;
//    1. W906_TimeDataHourEdge = golden CheckClockTrigger(60) (:4312-4328): arms in mm:ss 00:00 .. 00:02, fires once at
//       >= 00:04, a skipped window = no row that hour;
//    2. W906_TimeDataHourTickAt: InitialOK false = golden :106 (no clock read, the edge is not armed);
//    3. the hour edge -> RecordTimeData(3) (iN10UploadProductMethod 0): one TimeData row with the [3] counters in
//       seconds, the golden header, <sandbox>\HT9045_Log\TimeData\<yyyy>\TimeData_<yyyy>.csv; the [2] / [3] counters,
//       BinCT[1] and iJamCount[2] reset, [0] untouched; a HANDLER LOG row in the sandbox;
//       AI(W906-CMYDB-A5) 20260928 (St02-E helper): that row's TesterID column (the 3rd) = fLotInfo->edtASECL_TesterID->Text
//       (golden 906_0625_Steven cMyDB.cpp:1741 / :1800-1801; the box is set in memory here, nothing else changes);
//    4. iN10UploadProductMethod 1 -> RecordTimeData(2) (the [2] counters);
//    5. VTEST (bVTESTFunction) -> golden :345-346 return: no row, nothing reset;
//    6. ela::ReadTimeData / ParseTimeDataLine read exactly what the Handler wrote (PowerOnTime, StartTime);
//    7. AI(W906-CMYDB-A4) 20260928 (St02-E helper): the hourly Production_Loader file (golden :393-431) -- step 3 writes
//       <sandbox>\Production_Loader\<yyyy>\<yyyy-mm-dd>-<0800-2000|2000-0800>.txt = [Product] LoaderCount / ProductTime and
//       resets LastSet.iLoaderCount; step 4 (iDataType 2, not CC_TERAPOWER) leaves both alone; CC_TERAPOWER + iDataType 2
//       adds onto the same file (golden ReadIniData + WriteIniData).  The expected name is worked out HERE from the golden
//       rule over the real clock (LoaderName), not by calling the port.  asProduct_LoaderPath AND W906_PRODLOADER_ROOT
//       (cMyDB.cpp W906ProductLoaderDir) both point into the sandbox, refused under D:\HT9045*.
//  fMain->slTimeData is built here with golden's name / header / TByYear (LogObjects.cpp:136-140 does it in wb_serve).
//  The sandbox is removed on a green run.
// =============================================================================
#include <winsock2.h>                     // before anything that pulls <windows.h>
#include "TesterComm/Handler/TesterCommWiring.h"
#include "EventLogAnalysis/ElaOee.h"
#include "Public/MyStringList.h"
#include "forms/fMain.h"
#include "forms/fLotInfo.h"               // AI(W906-CMYDB-A5): fLotInfo->edtASECL_TesterID (as tests/test_tcp_cmd_server.cpp:37)
#include "common.h"
#include "cmydef.h"
#include "Config.h"
#include "LastSet.h"
#include "MachineType.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// AI(W906-CMYDB-A4) 20260928 (St02-E helper): _putenv for W906_PRODLOADER_ROOT -- the tests/test_agv_e84.cpp:159 guard
// (MinGW.org 6.3.0 declares neither _putenv nor putenv under this tree's strict -std=c++17).
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define ELA_TD_PUTENV _putenv
#else
#define ELA_TD_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }               \
        else      { std::printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
        if (s[i] == '/') s[i] = '\\';
    }
    return s;
}

static bool UnderMachineTree(const std::string& p)
{
    return Lower(p).compare(0, 9, "d:\\ht9045") == 0;   // D:\HT9045\... and D:\HT9045_Log\...
}

static std::string Bytes(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char b[4096];
    size_t n;
    while ((n = std::fread(b, 1, sizeof(b), f)) > 0) s.append(b, n);
    std::fclose(f);
    return s;
}

static std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    size_t p = 0;
    while (p < text.size())
    {
        size_t e = text.find('\n', p);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(p, e - p);
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        if (!l.empty()) v.push_back(l);
        p = e + 1;
    }
    return v;
}

// every file under dir (recursive), for the clean-up and the "only in the sandbox" count
static void ListFiles(const std::string& dir, std::vector<std::string>* files, std::vector<std::string>* dirs)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do
    {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..") continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            ListFiles(p, files, dirs);
            dirs->push_back(p);
        }
        else
            files->push_back(p);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

static void SetCounters(int set, long base)          // LastSet.SystemAccSecond[set][*] = base + i * 1000 ms
{
    for (int i = 0; i < 8; ++i) LastSet.SystemAccSecond[set][i] = base + i * 1000L;
}

static bool Zero(int set)
{
    for (int i = 0; i < 8; ++i) if (LastSet.SystemAccSecond[set][i] != 0) return false;
    return true;
}

// AI(W906-CMYDB-A4) 20260928 (St02-E helper): the golden rule (906_0625_Steven cMyDB.cpp:396-425) over one local clock
// reading -> "<yyyy>\<yyyy-mm-dd>-<slot>.txt" under asProduct_LoaderPath.  iNowTime = hhmmss; the date AND the year
// folder are Now()+iDecDay (whole days).
static std::string LoaderName(const SYSTEMTIME& st)
{
    const int t = st.wHour * 10000 + st.wMinute * 100 + st.wSecond;
    int dec;
    const char* slot;
    if (80000 + 20 > t && t > 80000)        { dec = -1; slot = "2000-0800"; }   // :398-403 (08:00:01 .. 08:00:19)
    else if (200000 + 20 > t && t > 200000) { dec = 0;  slot = "0800-2000"; }   // :404-408
    else if (200000 > t && t > 80000)       { dec = 0;  slot = "0800-2000"; }   // :409-413
    else                                    { dec = 1;  slot = "2000-0800"; }   // :414-418 (also 00:00 .. 08:00:00)
    FILETIME ft;
    ::SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER u;
    u.LowPart = ft.dwLowDateTime;
    u.HighPart = ft.dwHighDateTime;
    u.QuadPart += (ULONGLONG)((LONGLONG)dec * 864000000000LL);                  // one day in 100 ns
    ft.dwLowDateTime = u.LowPart;
    ft.dwHighDateTime = u.HighPart;
    SYSTEMTIME d;
    ::FileTimeToSystemTime(&ft, &d);
    char b[64];
    std::snprintf(b, sizeof(b), "%04d\\%04d-%02d-%02d-%s.txt", (int)d.wYear, (int)d.wYear, (int)d.wMonth, (int)d.wDay, slot);
    return b;
}

// every name a record made between the clock readings a <= b can carry (one per second, usually one name)
static std::vector<std::string> LoaderNames(const SYSTEMTIME& a, const SYSTEMTIME& b)
{
    std::vector<std::string> v;
    FILETIME fa, fb;
    ::SystemTimeToFileTime(&a, &fa);
    ::SystemTimeToFileTime(&b, &fb);
    ULARGE_INTEGER ua, ub;
    ua.LowPart = fa.dwLowDateTime; ua.HighPart = fa.dwHighDateTime;
    ub.LowPart = fb.dwLowDateTime; ub.HighPart = fb.dwHighDateTime;
    ua.QuadPart -= ua.QuadPart % 10000000ULL;
    for (ULONGLONG q = ua.QuadPart; q <= ub.QuadPart; q += 10000000ULL)
    {
        ULARGE_INTEGER uq;
        uq.QuadPart = q;
        FILETIME fq;
        fq.dwLowDateTime = uq.LowPart;
        fq.dwHighDateTime = uq.HighPart;
        SYSTEMTIME sq;
        ::FileTimeToSystemTime(&fq, &sq);
        const std::string n = LoaderName(sq);
        bool seen = false;
        for (size_t i = 0; i < v.size(); ++i) seen = seen || v[i] == n;
        if (!seen) v.push_back(n);
    }
    return v;
}

static bool NameIn(const std::string& path, const std::string& dir, const std::vector<std::string>& names)
{
    for (size_t i = 0; i < names.size(); ++i)
        if (path == dir + "\\" + names[i]) return true;
    return false;
}

static long IniNumber(const std::vector<std::string>& l, const std::string& key)   // "<key>=<n>" -> n, else -1
{
    for (size_t i = 0; i < l.size(); ++i)
        if (l[i].compare(0, key.size() + 1, key + "=") == 0) return std::atol(l[i].c_str() + key.size() + 1);
    return -1;
}

// AI(W906-CMYDB-A5) 20260928 (St02-E helper): the k-th (0-based) comma field of a line, no quote handling -- the first three
// HANDLER LOG columns (SiteID = SocketHandlerID, ProjectID = recipe, TesterID) carry no comma here
static std::string Field(const std::string& line, int k)
{
    size_t b = 0;
    for (int i = 0; i < k; ++i)
    {
        b = line.find(',', b);
        if (b == std::string::npos) return std::string();
        ++b;
    }
    const size_t e = line.find(',', b);
    return line.substr(b, e == std::string::npos ? std::string::npos : e - b);
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("ELA_TimeData\n");

    // ---- 0. containment first (the b12ab375 rule): refuse, exit 2, before ANY Handler code ----
    {
        const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
        const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
        const char* const envs[] = { "W906_HT9045LOG_ROOT", "W906_SAVEEVENTLOG_ROOT", "W906_RMS_ROOT", "W906_PRODINFO_ROOT",
                                     "W906_EVENTLOG_ROOT", "W906_AUTH_PATH",
                                     "W906_PRODLOADER_ROOT", "W906_O19_ROOT" };   // AI(W906-CMYDB-A4) 20260928: tests/CMakeLists.txt :3553 (cMyDB.cpp W906ProductLoaderDir).  AI(W906-O19-C4) 20261002 (St02-E helper): W906_O19_ROOT = the cMyDB.cpp O19 block's W906O19_SaveDir (tests/CMakeLists.txt EOF) -- RecordTimeData reads the real clock, and a run at 08:00:01 .. 08:00:19 enters that block
        bool contained = true;
        for (int i = 0; i < 4; ++i)
        {
            std::printf("  %s = %s\n", names[i], roots[i]->c_str());
            if (Lower(roots[i]->c_str()).find("machine_log_scratch") == std::string::npos)
                contained = false;
        }
        for (size_t i = 0; i < sizeof(envs) / sizeof(envs[0]); ++i)
        {
            const char* v = std::getenv(envs[i]);
            const std::string lv = Lower(v ? v : "");
            if (lv.find("machine_log_scratch") == std::string::npos && lv.find("machine_config_scratch") == std::string::npos)
            {
                std::printf("  %s = %s\n", envs[i], v ? v : "(unset)");
                contained = false;
            }
        }
        if (!contained)
        {
            std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R ELA_TimeData) -- nothing was called\n");
            return 2;
        }
    }

    // ---- the sandbox ----
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_ela_timedata_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\HT9045_Log").c_str(), 0);
    ::CreateDirectoryA((root + "\\SaveEventLog").c_str(), 0);
    as9045LogPath = AnsiString((root + "\\HT9045_Log").c_str());
    asSaveEventLogPath = AnsiString((root + "\\SaveEventLog").c_str());
    // AI(W906-CMYDB-A4) 20260928 (St02-E helper): the golden global AND the seam (env first, cMyDB.cpp :474) -> the sandbox
    const std::string plDir = root + "\\Production_Loader";
    asProduct_LoaderPath = AnsiString(plDir.c_str());
    static std::string s_plEnv;
    s_plEnv = "W906_PRODLOADER_ROOT=" + plDir;
    ELA_TD_PUTENV(s_plEnv.c_str());
    const char* plEnv = std::getenv("W906_PRODLOADER_ROOT");
    if (UnderMachineTree(root) || UnderMachineTree(as9045LogPath.c_str()) || UnderMachineTree(asSaveEventLogPath.c_str()) ||
        UnderMachineTree(asProduct_LoaderPath.c_str()) || plEnv == 0 || std::string(plEnv) != plDir || UnderMachineTree(plEnv))
    {
        std::printf("  ABORT: %s is not in the sandbox (W906_PRODLOADER_ROOT = %s) -- nothing was called\n", root.c_str(),
                    plEnv ? plEnv : "(unset)");
        return 2;
    }
    CUSTOMER_CODE = 0;   // not CC_TERAPOWER: only iDataType 3 writes the Production_Loader file (golden :393-394) until step 7
    const std::string tdDir = root + "\\HT9045_Log\\TimeData";

    // 1. the hour edge (a caller-owned state, as golden's function-local static)
    {
        bool s = false;
        const bool a = !W906_TimeDataHourEdge(95959, &s) && !s;         // 09:59:59: nothing
        const bool b = !W906_TimeDataHourEdge(100000, &s) && s;         // 10:00:00: armed
        const bool c = !W906_TimeDataHourEdge(100003, &s) && s;         // 10:00:03: not yet (> 3 needed)
        const bool d = W906_TimeDataHourEdge(100004, &s) && !s;         // 10:00:04: fires once
        const bool e = !W906_TimeDataHourEdge(100005, &s) && !s;
        bool t = false;
        const bool f = !W906_TimeDataHourEdge(105959, &t) && !W906_TimeDataHourEdge(110004, &t) && !t;   // window skipped
        CHECK(a && b && c && d && e && f,
              "1. hour edge (golden :4312-4328): armed at mm:ss < 00:03, fires once at >= 00:04; a skipped window = no row");
    }

    // the Handler's slTimeData, as wb_serve builds it (LogObjects.cpp:136-140 = golden main.cpp:1539-1543)
    fMain->slTimeData = new TMyStringList(as9045LogPath + "\\TimeData", "TimeData",
                                          "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, "
                                          "JamTime, PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA");
    fMain->slTimeData->SaveType = TByYear;
    IniConfig.bVTESTFunction = false;
    IniConfig.iN10UploadProductMethod = 0;

    // 2. InitialOK false: golden :106 returns before the clock is read -- no row, no reset, the edge not armed
    SetCounters(0, 900000);
    SetCounters(2, 7000);
    SetCounters(3, 11000);
    InitialOK = false;
    W906_TimeDataHourTickAt(100000);
    W906_TimeDataHourTickAt(100004);
    InitialOK = true;
    W906_TimeDataHourTickAt(100005);                                     // (100000 was not seen: still not armed)
    {
        std::vector<ela::TimeDataRow> rows = ela::ReadTimeData(tdDir, ela::EncodeDate(2026, 1, 1), ela::EncodeDate(2030, 1, 1), 0);
        // AI(W906-ELA-W48B-A6) 20260929: the first call with InitialOK true (100005) runs the one-time first-tick reset
        CHECK(rows.empty() && Zero(3) && Zero(2) && LastSet.SystemAccSecond[0][0] == 900000,
              "2. InitialOK false: no row, the edge not armed (golden guard :106); the first InitialOK tick clears [2] / [3] once (A6), [0] untouched");
    }
    SetCounters(2, 7000);                                                // back to the step-3 fixture after the A6 reset
    SetCounters(3, 11000);

    // 3. the hour edge -> RecordTimeData(3): [3] = 11 / 12 / ... / 18 s, PowerOn [3][stPowerOn] = 3600 s
    LastSet.SystemAccSecond[3][stPowerOn] = 3600000;
    LastSet.BinCT[1][0] = 5;
    LastSet.BinCT[1][3] = 7;
    LastSet.iJamCount[2] = 3;
    LastSet.iLoaderCount = 9;                                            // AI(W906-CMYDB-A4): 9 ICs taken from the loader this hour
    const AnsiString testerId = "A5-TESTER-7";                           // AI(W906-CMYDB-A5): the ASE-CL Tester ID box, in memory only
    fLotInfo->edtASECL_TesterID->Text = testerId;
    W906_TimeDataHourTickAt(110001);                                     // armed
    SYSTEMTIME s3a;
    ::GetLocalTime(&s3a);                                                // (the Production_Loader name comes from the real clock)
    W906_TimeDataHourTickAt(110004);                                     // fires
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char yyyy[8];
    std::snprintf(yyyy, sizeof(yyyy), "%04d", (int)st.wYear);
    const std::string file = tdDir + "\\" + yyyy + "\\TimeData_" + yyyy + ".csv";
    {
        const std::vector<std::string> l = Lines(Bytes(file));
        // [3] = StartTime 11, PauseTime 12, PowerOn 3600, ProductTime 14, JamTime 15, SystemNG 16, ContactTest 17, HomeTime 18
        // -> "%i,...": StartTime, HomeTime, ContactTest, Pause, Product, Jam, PowerOn = 11,18,17,12,14,15,3600; Sum 12,
        //    JamCount 3, MUBA 12 / 3 = 4, MTBA (12 + 14 + 15) / 3 = 13
        const bool ok = l.size() == 2 &&
                        l[0] == "Date, Time, StartTime, HomeTime, ContactTestTime, PauseTime, ProductionTime, JamTime, "
                                "PowerOnTime, UnloadingCount, JamCount, MUBA, MTBA" &&
                        l[1].find("11,18,17,12,14,15,3600,12,3,4") != std::string::npos;
        if (!ok)
            for (size_t i = 0; i < l.size(); ++i) std::printf("    %s\n", l[i].c_str());
        CHECK(ok, "3. RecordTimeData(3): the golden header + one row of the [3] seconds (PowerOn 3600, Sum 12, MUBA 4, MTBA 13)");
        CHECK(Zero(2) && Zero(3) && LastSet.SystemAccSecond[0][0] == 900000 && LastSet.BinCT[1][0] == 0 &&
                  LastSet.BinCT[1][3] == 0 && LastSet.iJamCount[2] == 0,
              "3. [2] / [3], BinCT[1], iJamCount[2] reset (golden :359-363 / :479-484); [0] untouched");
        std::vector<std::string> f, d;
        ListFiles(root + "\\SaveEventLog", &f, &d);
        bool handlerLog = false;
        for (size_t i = 0; i < f.size(); ++i) handlerLog = handlerLog || f[i].find("HANDLER LOG_") != std::string::npos;
        CHECK(handlerLog, "3. the golden SaveEventLogInfo(\"220000000\") row went to the sandbox's HANDLER LOG");
        // AI(W906-CMYDB-A5) 20260928 (St02-E helper): golden cMyDB.cpp:1741 -- the TesterID column is the box's text (was " ")
        bool testerCol = false;
        for (size_t i = 0; i < f.size(); ++i)
        {
            if (f[i].find("HANDLER LOG_") == std::string::npos) continue;
            const std::vector<std::string> l = Lines(Bytes(f[i]));
            for (size_t j = 1; j < l.size(); ++j) testerCol = testerCol || Field(l[j], 2) == testerId.c_str();
        }
        CHECK(testerCol, "3. A5: the HANDLER LOG row's TesterID column = fLotInfo->edtASECL_TesterID->Text (golden :1741)");
        fLotInfo->edtASECL_TesterID->Text = "";
    }
    // 3. AI(W906-CMYDB-A4) 20260928 (St02-E helper): golden :393-431 -- one Production_Loader file, the golden name for the
    //    clock at the edge, WritePrivateProfileString layout: [Product] / LoaderCount=0+9 / ProductTime=0+[3][stProductTime] 14 s
    const std::vector<std::string> names3 = LoaderNames(s3a, st);
    std::string pl3;                                                     // its bytes, for step 4
    {
        std::vector<std::string> f, d;
        ListFiles(plDir, &f, &d);
        const std::vector<std::string> l = f.size() == 1 ? Lines(Bytes(f[0])) : std::vector<std::string>();
        const bool ok = f.size() == 1 && NameIn(f[0], plDir, names3) && l.size() == 3 && l[0] == "[Product]" &&
                        l[1] == "LoaderCount=9" && l[2] == "ProductTime=14";
        if (!ok)
        {
            for (size_t i = 0; i < names3.size(); ++i) std::printf("    expected %s\\%s\n", plDir.c_str(), names3[i].c_str());
            for (size_t i = 0; i < f.size(); ++i) std::printf("    found    %s\n", f[i].c_str());
            for (size_t i = 0; i < l.size(); ++i) std::printf("      %s\n", l[i].c_str());
        }
        CHECK(ok, "3. golden :393-431: <sandbox>\\Production_Loader\\<yyyy>\\<yyyy-mm-dd>-<slot>.txt = [Product] LoaderCount=9, ProductTime=14");
        CHECK(LastSet.iLoaderCount == 0, "3. golden :431: LastSet.iLoaderCount = 0 after the record");
        if (f.size() == 1) pl3 = Bytes(f[0]);
    }

    // 4. iN10UploadProductMethod 1 -> RecordTimeData(2)
    IniConfig.iN10UploadProductMethod = 1;
    SetCounters(2, 21000);
    LastSet.SystemAccSecond[2][stPowerOn] = 1800000;
    LastSet.iLoaderCount = 4;                                            // AI(W906-CMYDB-A4): not CC_TERAPOWER -> kept
    W906_TimeDataHourTickAt(120000);
    W906_TimeDataHourTickAt(120004);
    {
        const std::vector<std::string> l = Lines(Bytes(file));
        CHECK(l.size() == 3 && l[2].find("21,28,27,22,24,25,1800,0,0,0") != std::string::npos && Zero(2),
              "4. method 1: RecordTimeData(2) -- the [2] seconds (PowerOn 1800), then reset");
        std::vector<std::string> f, d;
        ListFiles(plDir, &f, &d);
        CHECK(LastSet.iLoaderCount == 4 && f.size() == 1 && !pl3.empty() && Bytes(f[0]) == pl3,
              "4. golden :393-394: iDataType 2 without CC_TERAPOWER -- the Production_Loader file untouched, iLoaderCount kept (4)");
    }

    // 5. VTEST: golden :345-346 returns first -- no row, nothing reset
    IniConfig.bVTESTFunction = true;
    SetCounters(3, 31000);
    W906_TimeDataHourTickAt(130000);
    W906_TimeDataHourTickAt(130004);
    IniConfig.bVTESTFunction = false;
    {
        const std::vector<std::string> l = Lines(Bytes(file));
        CHECK(l.size() == 3 && LastSet.SystemAccSecond[3][0] == 31000, "5. VTEST: no TimeData row, the counters kept");
    }

    // 6. the ELA reads back what the Handler wrote
    {
        std::vector<std::string> files;
        const std::vector<ela::TimeDataRow> rows =
            ela::ReadTimeData(tdDir, ela::EncodeDate(st.wYear, st.wMonth, st.wDay), ela::EncodeDate(st.wYear, st.wMonth, st.wDay), &files);
        // (the two rows may carry the same millisecond: ReadTimeData then orders them by PowerOnTime -- match by value)
        bool r3 = false, r2 = false;
        for (size_t i = 0; i < rows.size(); ++i)
        {
            r3 = r3 || (rows[i].onMs == 3600000 && rows[i].startMs == 11000);
            r2 = r2 || (rows[i].onMs == 1800000 && rows[i].startMs == 21000);
        }
        CHECK(files.size() == 1 && rows.size() == 2 && r3 && r2,
              "6. ela::ReadTimeData: both Handler rows, PowerOnTime / StartTime as written");
    }

    // 7. AI(W906-CMYDB-A4) 20260928 (St02-E helper): CC_TERAPOWER -- golden :393-394 takes iDataType 2 as well, and
    //    :427-430 add onto what the file already holds: LoaderCount 9 + 4 (kept since step 4), ProductTime 14 + [2] 44 s
    CUSTOMER_CODE = CC_TERAPOWER;
    SetCounters(2, 41000);                                               // [2][stProductTime] = 44000 ms
    W906_TimeDataHourTickAt(140000);
    SYSTEMTIME s7a, s7b;
    ::GetLocalTime(&s7a);
    W906_TimeDataHourTickAt(140004);                                     // method 1 -> RecordTimeData(2)
    ::GetLocalTime(&s7b);
    CUSTOMER_CODE = 0;
    {
        const std::vector<std::string> names7 = LoaderNames(s7a, s7b);
        std::vector<std::string> both = names3;
        both.insert(both.end(), names7.begin(), names7.end());
        std::vector<std::string> f, d;
        ListFiles(plDir, &f, &d);
        long loaders = 0, seconds = 0;
        bool named = !f.empty() && f.size() <= 2;
        for (size_t i = 0; i < f.size(); ++i)
        {
            const std::vector<std::string> l = Lines(Bytes(f[i]));
            named = named && NameIn(f[i], plDir, both) && !l.empty() && l[0] == "[Product]";
            loaders += IniNumber(l, "LoaderCount");
            seconds += IniNumber(l, "ProductTime");
        }
        // the usual case: steps 3 and 7 share one slot -> one file holding the sums
        const bool oneSlot = names3.size() == 1 && names7.size() == 1 && names3[0] == names7[0];
        const std::vector<std::string> l0 = f.size() == 1 ? Lines(Bytes(f[0])) : std::vector<std::string>();
        const bool exact = !oneSlot || (f.size() == 1 && l0.size() == 3 && l0[1] == "LoaderCount=13" && l0[2] == "ProductTime=58");
        const bool ok = named && loaders == 13 && seconds == 58 && exact;
        if (!ok)
            for (size_t i = 0; i < f.size(); ++i) std::printf("    found %s\n", f[i].c_str());
        CHECK(ok, "7. CC_TERAPOWER + iDataType 2: the same file adds up -- LoaderCount=13 (9 + 4), ProductTime=58 (14 + 44)");
        CHECK(LastSet.iLoaderCount == 0, "7. golden :431: LastSet.iLoaderCount = 0 again");
    }

    delete fMain->slTimeData;                                             // the golden dtor flushes (nothing left)
    fMain->slTimeData = nullptr;

    std::printf("ELA_TimeData: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
    {
        std::vector<std::string> f, d;
        ListFiles(root, &f, &d);
        for (size_t i = 0; i < f.size(); ++i) ::DeleteFileA(f[i].c_str());
        for (size_t i = 0; i < d.size(); ++i) ::RemoveDirectoryA(d[i].c_str());
        ::RemoveDirectoryA(root.c_str());
    }
    return g_fail ? 1 : 0;
}
