// ===========================================================================
//  JsonBridge/actions/ObserverSGJam.cpp
//
//  AI(W906-ST02-OB7) 20261002 (St02-E helper).  act.observerSG.queryNow / queryYesterday / state -- notes in the .h head.
//  golden 906_0625 cObserver.cpp:5361-5369 (the two buttons) -> StatisticalJamCount :5060-5276; the bodies are
//  jimmychiu's members cObserver.cpp:3605 / :3610 (-> :3320), called as they are.
// ===========================================================================
#include "JsonBridge/actions/ObserverSGJam.h"

#include <cstdio>
#include <cstdlib>
#include <exception>
#include <string>

#include "JsonBridge/EventLog.h"   // LogAppend (SKILL 4.7 rule 1: every act.* button logs once before the body)
#include "WebBridge/JsonWriter.h"
#include "Public/cJSON.h"
#include "vclcompat/vcl_compat.h"
#include "forms/fObserver.h"       // TfObserver, fObserver (cObserver.cpp:3289, ht9045_sm)
#include "Public/MyStringList.h"   // TMyStringList full type (slEventLog->Path / ->FileName)
#include "cmydef.h"                // slEventLog, InitialOK, iOneDayLoaderCount, SystemYear.. / SystemYearYesterday..
#include "cpublic.h"               // GetTimeInfo / GetYesterdayInfo
#include "Config.h"                // IniConfig.asA32_1_HandlerID / bN26_UseJamRawDataUpdataToFTP
#include "W906FormShowing.h"       // W906_FormShowing (St01 page table; no hook = the member, as in ctest)

namespace {

const char* const kOpList = "queryNow, queryYesterday, state";
const char* const kGoldenNow =
    "906_0625 cObserver.cpp:5361-5364 btnSG_QueryNowClick -> StatisticalJamCount(false) :5060-5276 (912 :5592)";
const char* const kGoldenYesterday =
    "906_0625 cObserver.cpp:5366-5369 btnSG_QueryYesterdayClick -> StatisticalJamCount(true) :5060-5276 (912 :5597)";
const char* const kGoldenState =
    "(no golden handler: strngrdJamLog / labLoaderCount keep their contents on the form, 906_0625 cObserver.dfm:1851 / :1898)";
const char* const kFtpTailSkipped =
    "fFTPClient->UploadFileFTP(...) (906_0625 cObserver.cpp:5241-5260, bIsNextDay && [N26] bN26_UseJamRawDataUpdataToFTP): "
    "GATE (Q5a) cObserver.cpp:3524-3530, no TfFTPClient in the port; [N26] is shown only for SIGURD (S25)";

const int kMaxGridRows = 5000;   // a reply cap only; one row per distinct JAM01..JAM19 code of one day

struct QueryDay { int y; int m; int d; };

void Refuse(webbridge::JsonWriter& w, const char* guard, const std::string& detail)
{
    w.Key("executed").Bool(false);
    w.Key("guard").String(guard);
    w.Key("detail").String(detail);
    w.EndObject();
}

// Same rule as ChanAction.cpp / MainRecordClear.cpp DryRun (RULINGS_20260926 #12): no payload, no key or false = execute;
// true (or any other value) = guards only.  A value that is not a JSON object is refused (bad-payload).
bool ParseDryRun(const std::string& payloadJson, bool* bad)
{
    *bad = false;
    if (payloadJson.empty()) return false;
    cJSON* root = cJSON_Parse(payloadJson.c_str());
    if (root == 0 || !cJSON_IsObject(root)) {
        if (root) cJSON_Delete(root);
        *bad = true;
        return true;
    }
    const cJSON* j = cJSON_GetObjectItemCaseSensitive(root, "dryRun");
    const bool dry = !(j == 0 || (cJSON_IsBool(j) && cJSON_IsFalse(j)));
    cJSON_Delete(root);
    return dry;
}

// The day StatisticalJamCount picks: the same two clock calls in the same order (golden :5086-5087), then :5089-5100.
QueryDay PickDay(bool bIsNextDay)
{
    GetYesterdayInfo();
    GetTimeInfo();
    QueryDay t;
    if (bIsNextDay) { t.y = (int)SystemYearYesterday; t.m = (int)SystemMonthYesterday; t.d = (int)SystemDateYesterday; }
    else            { t.y = (int)SystemYear;          t.m = (int)SystemMonth;          t.d = (int)SystemDate; }
    return t;
}

// golden :5075-5076 + :5102-5105: <slEventLog->Path>\YYYY\MM\<slEventLog->FileName>_YYYYMMDD.csv
std::string EventLogCsvPath(const QueryDay& t)
{
    const AnsiString htPath = slEventLog->Path;
    const AnsiString htFile = slEventLog->FileName;
    char dir[32];
    char tail[40];
    std::snprintf(dir, sizeof(dir), "\\%04d\\%02d\\", t.y, t.m);
    std::snprintf(tail, sizeof(tail), "_%04d%02d%02d.csv", t.y, t.m, t.d);
    return std::string(htPath.c_str()) + dir + htFile.c_str() + tail;
}

// The RawData file: TMyStringList(W906_EventLogRootQ5(), asHandlerID, ...) (cObserver.cpp:3466, golden :5201) then
// MySaveSGJamCountToFile (Public/MyStringList.cpp:1040, golden MyStringList.cpp:742-804; TByDay / TByHour name).
// The root mirrors cObserver.cpp:3314-3318 W906_EventLogRootQ5 (static there): getenv non-NULL, else golden's literal.
std::string SgJamCsvPath(const QueryDay& t)
{
    const char* env = std::getenv("W906_EVENTLOG_ROOT");
    const std::string root = env ? std::string(env) : std::string("D:\\HT9045_Log\\EventLogTxt");
    const std::string id = (IniConfig.asA32_1_HandlerID == "") ? std::string("HandlerID")            // golden :5191-5198
                                                               : std::string(IniConfig.asA32_1_HandlerID.c_str());
    char dir[48];
    char tail[40];
    std::snprintf(dir, sizeof(dir), "\\SGJamCount\\%04d\\%02d\\", t.y, t.m);
    std::snprintf(tail, sizeof(tail), "_%04d%02d%02d_RawData.csv", t.y, t.m, t.d);
    return root + dir + id + tail;
}

void WriteGrid(webbridge::JsonWriter& w, TfObserver* f)
{
    TfObserverGrid* g = f->strngrdJamLog;
    const int rows = g->RowCount;
    const int cols = g->ColCount;
    const int n = rows < kMaxGridRows ? rows : kMaxGridRows;
    w.Key("grid").BeginObject();
    w.Key("rows").Number((wb_int64)rows);
    w.Key("cols").Number((wb_int64)cols);
    w.Key("fixedRows").Number((wb_int64)1);   // dfm sets no FixedRows (TStringGrid default 1), FixedCols = 0 (cObserver.dfm:1909)
    w.Key("fixedCols").Number((wb_int64)0);
    w.Key("truncated").Bool(n < rows);
    w.Key("cells").BeginArray();
    for (int r = 0; r < n; ++r) {
        w.BeginArray();
        for (int c = 0; c < cols; ++c) w.String(std::string(g->Cells[c][r].c_str()));
        w.EndArray();
    }
    w.EndArray();
    w.EndObject();
    w.Key("labLoaderCount").String(std::string(f->labLoaderCount->Caption.c_str()));   // "" = never set (dfm caption shows)
    w.Key("iOneDayLoaderCount").Number((wb_int64)iOneDayLoaderCount);
}

}  // namespace

namespace ht9045 {
namespace sjson {

std::string W906_ObserverSGJamAct_St02(const std::string& cmd, const std::string& payloadJson)
{
    const std::string op = cmd.size() > 15 ? cmd.substr(15) : std::string();   // "act.observerSG." = 15 characters
    const char* golden = op == "queryNow" ? kGoldenNow : (op == "queryYesterday" ? kGoldenYesterday : (op == "state" ? kGoldenState : 0));
    webbridge::JsonWriter w;
    w.BeginObject();
    w.Key("op").String(op);
    if (golden == 0) {
        Refuse(w, "unknown-action", std::string("act.observerSG.* has: ") + kOpList);
        return w.Str();
    }
    w.Key("goldenLine").String(golden);

    TfObserver* f = fObserver;
    if (f == 0 || f->strngrdJamLog == 0 || f->labLoaderCount == 0) {
        Refuse(w, "no-form", "fObserver / strngrdJamLog / labLoaderCount is NULL");
        return w.Str();
    }
    if (!W906_FormShowing("fObserver", f->bShow)) {
        Refuse(w, "not-open", "the Observer is not open (observer.get open = golden FormShow, cObserver.cpp bShow=true)");
        return w.Str();
    }
    bool bad = false;
    const bool dry = ParseDryRun(payloadJson, &bad);
    if (bad) {
        Refuse(w, "bad-payload", "value must be a JSON object string");
        return w.Str();
    }

    if (op == "state") {                                       // read only: no log line, no golden code
        w.Key("executed").Bool(true);
        w.Key("guard").String("");
        WriteGrid(w, f);
        w.EndObject();
        return w.Str();
    }

    const bool bIsNextDay = (op == "queryYesterday");
    w.Key("dryRun").Bool(dry);
    if (slEventLog == 0) {
        Refuse(w, "log-objects-not-created",
               "slEventLog is NULL (golden builds it in the TfMain constructor, main.cpp:1501-1512; StatisticalJamCount reads "
               "slEventLog->Path at :5075): nothing called, nothing written");
        return w.Str();
    }
    const bool initialOk = InitialOK;
    const QueryDay t = PickDay(bIsNextDay);
    const std::string csv = EventLogCsvPath(t);
    const bool csvExists = FileExists(AnsiString(csv.c_str()));
    w.Key("initialOk").Bool(initialOk);
    w.Key("eventLogCsv").BeginObject();
    w.Key("path").String(csv);
    w.Key("exists").Bool(csvExists);
    w.EndObject();

    if (dry) {
        w.Key("executed").Bool(false);
        w.Key("guard").String("");                             // no guard stopped it: dryRun did
        w.Key("note").String("dryRun: guards only, StatisticalJamCount not called");
        WriteGrid(w, f);
        w.EndObject();
        return w.Str();
    }

    LogAppend(kLogProcess, "act.observerSG." + op + " pressed", "", "", "act");
    const int before = iOneDayLoaderCount;
    try {
        if (bIsNextDay) f->btnSG_QueryYesterdayClick(nullptr);   // cObserver.cpp:3610 (golden :5366-5369 as is)
        else            f->btnSG_QueryNowClick(nullptr);         // cObserver.cpp:3605 (golden :5361-5364 as is)
    } catch (const std::exception& e) {
        Refuse(w, "handler-failed", std::string("exception: ") + e.what());
        return w.Str();
    } catch (...) {
        Refuse(w, "handler-failed", "non-std exception");
        return w.Str();
    }

    // golden returns silently on both early exits; the page needs to know which way it went.
    const char* early = !initialOk ? "initial-not-ok" : (!csvExists ? "eventlog-missing" : "");
    std::string result;
    if (!initialOk)
        result = "golden returns at once: InitialOK==false (:5062-5065)";
    else if (!csvExists)
        result = "golden :5106-5111: the event-log CSV is not there -> RecordProcess(\"JamRawData is error. EventLog is not exist. "
                 + csv + "\") and return; the grid keeps its cells";
    else
        result = std::string("counted JAM01..JAM19 codes of ") + csv + (bIsNextDay ? "; iOneDayLoaderCount = 0 (:5262-5263)" : "");
    w.Key("executed").Bool(true);
    w.Key("guard").String("");
    w.Key("early").String(early);
    w.Key("result").String(result);
    w.Key("loaderCountBefore").Number((wb_int64)before);
    if (early[0] == '\0') w.Key("sgJamCountCsv").String(SgJamCsvPath(t));
    WriteGrid(w, f);
    w.Key("skipped").BeginArray();
    if (early[0] == '\0' && bIsNextDay && IniConfig.bN26_UseJamRawDataUpdataToFTP) w.String(kFtpTailSkipped);
    w.EndArray();
    w.EndObject();
    std::printf("[ST02-OB7] act.observerSG.%s -> golden %s: %s\n", op.c_str(), golden, result.c_str());
    return w.Str();
}

}  // namespace sjson
}  // namespace ht9045
