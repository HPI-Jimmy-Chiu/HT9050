// =============================================================================
//  tests/test_mydb_p4_containment.cpp -- cMyDB P4: the five stand-ins are gone and the golden bodies write for real;
//  under ctest they must write ONLY into the build dir's machine_log_scratch.
//  AI(W906-CMYDB-P4) 20260927 (St02-E).  Suite name (add_test): MyDB_P4_Containment
//
//    0. containment first: refuses to run (exit 1, no P4 entry called) unless as9045LogPath, asSaveEventLogPath,
//       asProductionLogPath and sProductionInfoFilePath all contain "machine_log_scratch" -- W906_HT9045LOG_ROOT /
//       W906_SAVEEVENTLOG_ROOT / W906_RMS_ROOT / W906_PRODINFO_ROOT from tests/CMakeLists.txt _ht9045_env_extra
//       (common.cpp :240 / :303 / :263 / :293).  Then it notes which real golden roots exist and the start time.
//    1. every P4 entry once, each with its own token: RecordProcess (golden 906_0625_Steven cMyDB.cpp:1564),
//       NewRecordProcess (:1545), MyDBIProcessNew (:724), the 2-arg adapter (aHotPlateSubstrate.cpp), MyDBIProcess
//       3-arg (:789), RecordChangeLogProcess, the wb_serve MyMessageBox host shapes MyDBIProcess("Message", ...) /
//       ("Exception", ...) (tools/wb_serve.cpp :6143 / :6847 / :6890 / :6953 / :7083 -- functions of that exe cannot be
//       linked into a test, so the same calls are made here), ProductionLog with O06 on (D2) and
//       TfProductionInfo::SaveMessageHistroy with CC_Greatek + N14_1 (D4);
//    2. each token is found in the sandbox files: EventLogTxt (slEventLog built under the sandbox, as the P1 test),
//       HANDLER LOG_*.csv, *_EventTracker.csv, RMS\*.logs, *_History.csv;
//    3. under the real golden roots nothing new exists and no file has a write time >= the start time.
//  Files stay in the sandbox (the build dir's scratch, shared with the other tests).
//  ⚠ A running wb_serve / HT9045.exe on the same machine writes those real roots -> a false failure in step 3.
//  ⚠ Run by hand (no ctest environment) it refuses at step 0 -- that is the point.
// =============================================================================
#include "Public/MyStringList.h"
#include "cMyDB.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "cprod.h"
#include "Config.h"
#include "MachineType.h"
#include "forms/fMain.h"
#include "forms/fProductionInfo.h"

#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

// the 2-arg MyDBIProcess adapter (aHotPlateSubstrate.cpp).  Declared, never called by name here: with cMyDB.h's 3-arg
// default visible a 2-argument call would be ambiguous, so it is reached through a typed pointer below.
void MyDBIProcess(AnsiString S1, AnsiString S2);
extern int W906_MyDBIProcess_Count;   // aHotPlateSubstrate.cpp

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", what, line); }
    else     { std::printf("  PASS: %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool Has(const char* s, const char* sub) { return std::strstr(s, sub) != 0; }

static void MakeDirs(const std::string& p)
{
    for (size_t i = 3; i <= p.size(); ++i)
        if (i == p.size() || p[i] == '\\' || p[i] == '/')
            ::CreateDirectoryA(p.substr(0, i).c_str(), 0);
}

// Recursively look for a file whose name contains `nameHas` and whose content contains `textHas` (as the P1 test).
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
                if (all.find(textHas) != std::string::npos)
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

// Files under `dir` written at or after t0 (read-only walk; reparse points are not followed).
static void ScanNewer(const std::string& dir, const FILETIME& t0, std::vector<std::string>* hits, int depth)
{
    if (depth > 16 || hits->size() >= 20)
        return;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE)
        return;
    do
    {
        if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0)
            continue;
        const std::string p = dir + "\\" + fd.cFileName;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT))
                ScanNewer(p, t0, hits, depth + 1);
        }
        else if (::CompareFileTime(&fd.ftLastWriteTime, &t0) >= 0 && hits->size() < 20)
            hits->push_back(p);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

static bool Exists(const char* p) { return ::GetFileAttributesA(p) != INVALID_FILE_ATTRIBUTES; }

int main()
{
    std::printf("MyDB_P4_Containment\n");

    // ---- 0. containment first --------------------------------------------------------------------------------
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
    bool sandboxed = true;
    for (int i = 0; i < 4; ++i)
    {
        std::printf("  %s = %s\n", names[i], roots[i]->c_str());
        if (!Has(roots[i]->c_str(), "machine_log_scratch"))
            sandboxed = false;
    }
    CHECK(sandboxed);
    if (!sandboxed)
    {
        std::printf("FAIL: not sandboxed -- refusing to call any P4 entry (they would write the machine's real log trees)\n");
        return 1;
    }

    // The golden roots these bodies (and the W7 log objects) write when the seams are unset.
    static const char* const kReal[] = {
        "D:\\HT9045_Log\\SaveEventLog", "D:\\HT9045_Log\\ASE log", "D:\\HT9045_Log\\EventLogTxt", "D:\\RMS",
        "D:\\HT9045_log\\ProductionInfo",
        // W7 (LogObjects.cpp; no ctest creates those objects -- listed so that a regression which does is caught)
        "D:\\HT9045_Log\\2DMapping", "D:\\HT9045_Log\\MNetLog", "D:\\HT9045_Log\\UploadFile",
        "D:\\HT9045_Log\\TTL_Signal_LOG", "D:\\HT9045_Log\\Heater_On_Off_LOG", "D:\\HT9045_Log\\JamAlarmLogTxt",
        "D:\\HT9045_Log\\SocketIdProductData", "D:\\HT9045_Log\\TimeData", "D:\\HT9045_Log\\Hana_TrayMap",
        "D:\\HT9045_Log\\IndexPos", "D:\\HT9045_Log\\QtyData", "D:\\HT9045_Log\\ProductRecord", "D:\\HT9045_Log\\ASM",
        "D:\\HT9045_Log\\LotInfo", "D:\\HT9045_Log\\TriTemp", "D:\\HT9045_Log\\DewPoint_Log", "D:\\HT9045_Log\\RunState",
        "D:\\HT9045_Log\\TestLog", "D:\\HT9045_Log\\TorqueLog", "D:\\HT9045_log\\HPCARD", "D:\\HT9045_log\\GroundManLog" };
    const int kRealN = (int)(sizeof(kReal) / sizeof(kReal[0]));
    std::vector<bool> existed(kRealN);
    for (int i = 0; i < kRealN; ++i)
        existed[i] = Exists(kReal[i]);

    FILETIME t0;
    ::GetSystemTimeAsFileTime(&t0);

    char tokbuf[64];
    std::snprintf(tokbuf, sizeof(tokbuf), "W906P4TOK%lu", (unsigned long)::GetTickCount());
    const std::string tok = tokbuf;
    struct Tok { std::string s; AnsiString a() const { return AnsiString(s.c_str()); } };
    const Tok tRec{tok + "_RecordProcess"}, tNew{tok + "_NewRecordProcess"}, tNewDb{tok + "_MyDBIProcessNew"},
              tAd2{tok + "_Adapter2"}, tDb3{tok + "_MyDBIProcess3"}, tChg{tok + "_ChangeLog"},
              tMsg{tok + "_wbMessage"}, tExc{tok + "_wbException"}, tProd{tok + "_ProductionLog"}, tGrt{tok + "_Greatek"};

    GetTimeInfo();
    IniConfig.bSPILFunction = false;
    IniConfig.SocketHandlerID = "W906P4";
    const std::string sev = std::string(as9045LogPath.c_str()) + "\\EventLogTxt";
    const std::string ssave = asSaveEventLogPath.c_str();
    const std::string sase = std::string(as9045LogPath.c_str()) + "\\ASE log";
    const std::string srms = asProductionLogPath.c_str();
    const std::string spi = sProductionInfoFilePath.c_str();
    MakeDirs(sev);
    MakeDirs(ssave);
    MakeDirs(srms);                   // golden ProductionLog never creates its folder (D:\RMS is expected to exist)
    TMyStringList* const savedEv = slEventLog;
    slEventLog = new TMyStringList(AnsiString(sev.c_str()), "EventLogTxt",
                                   "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");

    // ---- 1. every P4 entry ------------------------------------------------------------------------------------
    RecordProcess(tRec.a());                                                   // -> MyDBIProcess("Process", S, "")
    NewRecordProcess("MES2110", tNew.a(), "p4");                               // -> MyDBIProcessNew("Process", ...)
    MyDBIProcessNew("Process", "MES2111", tNewDb.a(), "p4");
    {
        void (*const adapter2)(AnsiString, AnsiString) = &MyDBIProcess;       // the 2-arg overload, by type
        const int c0 = W906_MyDBIProcess_Count;
        adapter2("Exception", tAd2.a());                                       // golden: a 2-arg call = (asTable, S1, "")
        CHECK(W906_MyDBIProcess_Count == c0 + 1);
    }
    MyDBIProcess("Exception", tDb3.a(), "p4");
    RecordChangeLogProcess(tChg.a(), "p4");                                    // -> MyDBIProcess("ChangeLog", S, S2)
    MyDBIProcess("Message", tMsg.a(), "s3");                                   // wb_serve.cpp :6143 / :6890 / :6953 / :7083 shape
    MyDBIProcess("Exception", tExc.a(), "s3");                                 // wb_serve.cpp :6847 shape

    CHECK(fMain != 0);
    if (fMain != 0)                                                            // D2: ProductionLog saves the memo (fMain->MemoProductionLog)
    {
        IniConfig.bO06SaveLogTimePeriod = true;
        RecordProcess(tProd.a());                                              // MyDBIProcess -> ProductionLog(S1+S2, true)
        IniConfig.bO06SaveLogTimePeriod = false;
    }

    CHECK(fProductionInfo != 0);
    if (fProductionInfo != 0)                                                  // D4: NewRecordProcess's Greatek branch
    {
        const int savedCC = CUSTOMER_CODE;
        CUSTOMER_CODE = CC_Greatek;
        IniConfig.bN14_1_EnableOEEFunction = true;
        fProductionInfo->sLoadMO_MO = "W906P4MO";
        fProductionInfo->_sOEE_MO = "W906P4OEE";
        NewRecordProcess("MES2112", tGrt.a(), "p4");                            // -> SaveMessageHistroy("", S, 0, 0)
        CUSTOMER_CODE = savedCC;
        IniConfig.bN14_1_EnableOEEFunction = false;
    }

    delete slEventLog;
    slEventLog = savedEv;

    // ---- 2. the rows are in the sandbox -----------------------------------------------------------------------
    const Tok* const evAll[] = { &tRec, &tNew, &tNewDb, &tAd2, &tDb3, &tChg, &tMsg, &tExc };
    for (int i = 0; i < 8; ++i)
    {
        std::string f;
        const bool inEv = FindFileWith(sev, "EventLogTxt", evAll[i]->s.c_str(), &f);
        const bool inHl = FindFileWith(ssave, "HANDLER LOG_", evAll[i]->s.c_str(), &f);
        std::printf("  %s: EventLogTxt %s, HANDLER LOG %s\n", evAll[i]->s.c_str(), inEv ? "yes" : "NO", inHl ? "yes" : "NO");
        CHECK(inEv);
        CHECK(inHl);
    }
    std::string f;
    CHECK(FindFileWith(sase, "_EventTracker", tNewDb.s.c_str(), &f));          // MyDBIProcessNew's code != 220000000
    CHECK(FindFileWith(sase, "_EventTracker", tNew.s.c_str(), &f));
    CHECK(!FindFileWith(sase, "_EventTracker", tDb3.s.c_str(), &f));           // golden: "220000000" skips SaveEventTracker
    if (fMain != 0)
        CHECK(FindFileWith(srms, ".logs", tProd.s.c_str(), &f));
    if (fProductionInfo != 0)
        CHECK(FindFileWith(spi, "_History.csv", tGrt.s.c_str(), &f));

    // ---- 3. nothing reached the real roots --------------------------------------------------------------------
    for (int i = 0; i < kRealN; ++i)
    {
        const bool now = Exists(kReal[i]);
        const bool created = now && !existed[i];
        if (created)
            std::printf("  created by this run: %s\n", kReal[i]);
        CHECK(!created);
        if (!now || created)
            continue;
        std::vector<std::string> hits;
        ScanNewer(kReal[i], t0, &hits, 0);
        for (size_t k = 0; k < hits.size(); ++k)
            std::printf("  written during this run: %s\n", hits[k].c_str());
        CHECK(hits.empty());
    }

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
