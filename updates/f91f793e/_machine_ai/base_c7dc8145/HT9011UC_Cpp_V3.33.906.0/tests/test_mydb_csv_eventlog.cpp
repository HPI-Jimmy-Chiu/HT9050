// ===========================================================================
//  tests/test_mydb_csv_eventlog.cpp -- cMyDB CSV plan P1: the EventLogTxt / HANDLER LOG CSV backbone.
//  AI(W906-CSVONLY-P1) 20260926.
//
//  Real code only (the machine archives, RESCAN group): TMyStringList (Public/MyStringList.cpp), cMyDB.cpp's lifted
//  slEventLog path, LogObjects.cpp's SaveEventLog, and SaveEventLogInfo's HANDLER LOG write.
//  ⚠ NEVER touches D:\HT9045_Log: every path the exercised code uses is pointed at a fresh %TEMP% folder BEFORE the
//  first call -- slEventLog is built there (instead of W906_CreateLogObjects' golden D:\HT9045_Log path), and the two
//  path globals SaveEventLogInfo / SaveEventTracker read (as9045LogPath, asSaveEventLogPath) are redirected.
//  MyDBITotalLoader(…) passes "220000000" to SaveEventLogInfo, which therefore skips SaveEventTracker (golden).
//
//    1. MyDBITotalLoader(7) appends "TimeDataTotalLoader" to the EventLogTxt file under the temp EventLogTxt root.
//    2. SaveEventLogInfo wrote "HANDLER LOG_<SocketHandlerID>_yyyy_mm_dd.csv" under the temp asSaveEventLogPath.
//    3. RunInfo.slEventLogFile remembers the EventLogTxt file name (golden SaveEventLog).
// ===========================================================================
#include "Public/MyStringList.h"
#include "cMyDB.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "cprod.h"
#include "Config.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_fail = 0;
static int g_total = 0;
static void check(bool c, const char* e, int line)
{
    ++g_total;
    if (!c)
    {
        ++g_fail;
        std::printf("FAIL line %d: %s\n", line, e);
    }
}
#define CHECK(c) check((c), #c, __LINE__)

// Recursively look for a file whose name contains `nameHas` and whose content contains `textHas`.
static bool FindFileWith(const std::string& dir, const char* nameHas, const char* textHas, std::string* found)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    bool ok = false;
    do
    {
        const std::string n = fd.cFileName;
        if (n == "." || n == "..")
            continue;
        const std::string p = dir + "\\" + n;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (FindFileWith(p, nameHas, textHas, found)) { ok = true; break; }
        }
        else if (n.find(nameHas) != std::string::npos)
        {
            FILE* f = std::fopen(p.c_str(), "rb");
            if (f)
            {
                std::string all;
                char buf[4096];
                size_t r;
                while ((r = std::fread(buf, 1, sizeof(buf), f)) > 0)
                    all.append(buf, r);
                std::fclose(f);
                if (textHas == 0 || all.find(textHas) != std::string::npos)
                {
                    if (found) *found = p;
                    ok = true;
                    break;
                }
            }
        }
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return ok;
}

int main()
{
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_mydb_csv_eventlog_" + stamp;
    ::CreateDirectoryA(root.c_str(), 0);
    const std::string evRoot = root + "\\EventLogTxt";
    const std::string saveRoot = root + "\\SaveEventLog";
    ::CreateDirectoryA(evRoot.c_str(), 0);
    ::CreateDirectoryA(saveRoot.c_str(), 0);

    // --- redirect every path BEFORE the first call (see banner) ---
    as9045LogPath = AnsiString(root.c_str());
    asSaveEventLogPath = AnsiString(saveRoot.c_str());
    IniConfig.bSPILFunction = false;
    GetTimeInfo();
    TMyStringList* const saved = slEventLog;
    slEventLog = new TMyStringList(AnsiString(evRoot.c_str()), "EventLogTxt",
                                   "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");
    CHECK(AnsiString(slEventLog->Path).Pos(AnsiString(root.c_str())) == 1);
    CHECK(asSaveEventLogPath.Pos(AnsiString(root.c_str())) == 1);

    MyDBITotalLoader(7);

    std::string evFile, hlFile;
    CHECK(FindFileWith(evRoot, "EventLogTxt", "TimeDataTotalLoader", &evFile));
    CHECK(FindFileWith(saveRoot, "HANDLER LOG_", 0, &hlFile));
    CHECK(RunInfo.slEventLogFile != 0 && RunInfo.slEventLogFile->Count >= 1);
    if (RunInfo.slEventLogFile && RunInfo.slEventLogFile->Count >= 1)
        CHECK(std::string(AnsiString(RunInfo.slEventLogFile->Strings[0]).c_str()).find("EventLogTxt") != std::string::npos);
    std::printf("EventLogTxt file: %s\nHANDLER LOG file: %s\n", evFile.c_str(), hlFile.c_str());

    delete slEventLog;
    slEventLog = saved;
    std::printf("test_mydb_csv_eventlog: %d/%d passed (files left under %s)\n", g_total - g_fail, g_total, root.c_str());
    return g_fail == 0 ? 0 : 1;
}
