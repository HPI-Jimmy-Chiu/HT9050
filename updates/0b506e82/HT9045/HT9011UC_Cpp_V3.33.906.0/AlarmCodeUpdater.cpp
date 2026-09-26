// ===========================================================================
//  AlarmCodeUpdater.cpp -- CSV-only remainder of golden MDB Updater TfMain::UpdateAlarmList().
//  AI(W906-CSVONLY) 20260926: see AlarmCodeCatalog.h for scope, sources and what is not carried over.
//
//  Golden reference (D:\MDB Updater\MDB_Updater_Rev902.0_20260410):
//    Main.cpp:103-142  DoInsertAlarmCode -- map lookup; repeat -> tsList "@@Error!!" + "Alarm Code: X is duplicate!!",
//                      else AlarmCodeList->Add("Code=Message") (+ sqlite INSERT, retired)
//    Main.cpp:318-371  UpdateAlarmList   -- tsList first line = file version; ...; SaveUpdateLog();
//                      AlarmCodeList->SaveToFile("D:\\HT9045\\Error\\AlarmCodeList.txt")
//    CreatDatabase.cpp:44-58 SaveUpdateLog -- ForceDirectories("D:\\HT9045_Log\\MDB_UpdateLog"),
//                      tsList->SaveToFile("...\\SQLiteUpdateLog_yyyy_mm_dd_hh_nn_ss.logs")
//
//  Deliberate deviations (recorded in docs/CMYDB_PORT_LEDGER.md):
//    * log file name "AlarmCodeUpdateLog_..." instead of "SQLiteUpdateLog_...": there is no sqlite any more,
//      and the golden log body is mostly the SQL text it executed, which this version does not have.
//    * the log body is: version line, one "@@Error!!" / "Alarm Code: X is duplicate!!" pair per repeat
//      (same wording as golden), then a summary line.
// ===========================================================================
#include "AlarmCodeCatalog.h"

#include <cerrno>
#include <ctime>
#include <set>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

const char* const kAlarmCodeListPath    = "D:\\HT9045\\Error\\AlarmCodeList.txt";
const char* const kAlarmCodeUpdateLogDir = "D:\\HT9045_Log\\MDB_UpdateLog";

namespace {

struct BuildCtx
{
    std::set<std::string> seen;
    AlarmCodeUpdateResult* r;
};

void Collect(const char* code, const char* msg, void* ctx)
{
    BuildCtx* b = static_cast<BuildCtx*>(ctx);
    ++b->r->rawCount;
    if (!b->seen.insert(code).second)
    {
        b->r->duplicates.push_back(code);   // golden: first occurrence kept
        return;
    }
    AlarmCodeListEntry e;
    e.code = code;
    e.message = msg;
    b->r->list.push_back(e);
}

// ForceDirectories equivalent: create every missing component of dir.
bool MakeDirs(const std::string& dir)
{
    if (dir.empty())
        return false;
    for (std::size_t i = 0; i <= dir.size(); ++i)
    {
        if (i == dir.size() || dir[i] == '\\' || dir[i] == '/')
        {
            std::string part = dir.substr(0, i);
            if (part.empty() || (part.size() == 2 && part[1] == ':'))
                continue;
#ifdef _WIN32
            int rc = _mkdir(part.c_str());
#else
            int rc = mkdir(part.c_str(), 0777);
#endif
            if (rc != 0 && errno != EEXIST)
                return false;
        }
    }
    return true;
}

std::string DirOf(const std::string& path)
{
    std::size_t p = path.find_last_of("\\/");
    return p == std::string::npos ? std::string() : path.substr(0, p);
}

bool WriteAll(const std::string& path, const std::string& data)
{
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f)
        return false;
    bool ok = std::fwrite(data.data(), 1, data.size(), f) == data.size();
    ok = (std::fclose(f) == 0) && ok;
    return ok;
}

}  // namespace

AlarmCodeUpdateResult AlarmCodeCatalog_Build()
{
    AlarmCodeUpdateResult r;
    r.rawCount = 0;
    r.listWritten = false;
    r.logWritten = false;
    BuildCtx b;
    b.r = &r;
    AlarmCodeCatalog_Replay(&Collect, &b);
    return r;
}

AlarmCodeUpdateResult AlarmCodeCatalog_UpdateList(const std::string& listPath,
                                                  const std::string& logDir,
                                                  const std::string& versionLine)
{
    AlarmCodeUpdateResult r = AlarmCodeCatalog_Build();

    if (!listPath.empty())
    {
        std::string data;
        for (std::size_t i = 0; i < r.list.size(); ++i)
            data += r.list[i].code + "=" + r.list[i].message + "\r\n";
        std::string dir = DirOf(listPath);
        if (dir.empty() || MakeDirs(dir))
            r.listWritten = WriteAll(listPath, data);
    }

    if (!logDir.empty() && MakeDirs(logDir))
    {
        std::time_t now = std::time(0);
        std::tm* lt = std::localtime(&now);
        char stamp[32] = "0000_00_00_00_00_00";
        if (lt)
            std::strftime(stamp, sizeof(stamp), "%Y_%m_%d_%H_%M_%S", lt);
        std::string log = versionLine + "\r\n";
        for (std::size_t i = 0; i < r.duplicates.size(); ++i)
        {
            log += "@@Error!!\r\n";
            log += "Alarm Code: " + r.duplicates[i] + " is duplicate!!\r\n";
        }
        char summary[128];
        std::snprintf(summary, sizeof(summary), "AlarmCodeList: raw %u, written %u, duplicates %u\r\n",
                      (unsigned)r.rawCount, (unsigned)r.list.size(), (unsigned)r.duplicates.size());
        log += summary;
        std::string sep = (logDir[logDir.size() - 1] == '\\' || logDir[logDir.size() - 1] == '/') ? "" : "\\";
        r.logPath = logDir + sep + "AlarmCodeUpdateLog_" + stamp + ".logs";
        r.logWritten = WriteAll(r.logPath, log);
    }
    return r;
}
