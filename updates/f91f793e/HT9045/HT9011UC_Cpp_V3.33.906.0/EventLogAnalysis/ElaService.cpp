// ===========================================================================
//  EventLogAnalysis/ElaService.cpp -- the one production ElaHub and the wb_serve entry points (ELA plan P3).
//  AI(W906-ELA-P3) 20260927 (St02).  Ledger docs/ELA_PORT_LEDGER.md; skill ht9045-eventlog-analyzer.
//
//  wb_serve calls (tools/wb_serve.cpp :439 / :2867 / :4165 / :5967; github-59 GO, FROM_STEVEN §1 7901a82f):
//    W906_ElaStart()   at boot (after the log objects / MyDBUpdateDB), and installs the Handler's W906_ElaPostHook
//                      (Interface/InterfaceSYS.cpp SendCommand_EventLog) = W906_ElaPost;
//    W906_ElaHttp()    from the HTTP route hook: GET /api/ela[?since=<seq>], POST /api/ela/query?... (ela::ServeHttp);
//    W906_ElaStop()    at shutdown, after clearing the hook.
//  Start and Stop sit outside the HTTP server's life (server.Start() :4367 is after :4165, server.Stop() :5966 is before
//  :5967), so g_hub never changes while a request runs.
//  SendCommand_EventLog calls the hook first and still sends golden's WM_COPYDATA: an external EventlogAnalyzer.exe may
//  still be installed and does the six report / upload jobs this port holds back (HOLD); retiring it is Jimmy's call.
//  Golden start-up (TfrmELA::FormShow :418-424 + Timer1Timer :443-485): sHandlerID from Gerneral.ini [Version] Machine ID
//  ("HT-90xx" when the file is missing), then one query for today 00:00 .. now (cbTopAlarmFilter 0, rgFilter 0, every
//  box checked), ReadConfig, and bEL_UPDATE_PARAMETER.  Here: the query is posted and EL_UPDATE_PARAMETER is raised;
//  the Hub runs the raised job first, so ReadConfig (and sCustCode) comes before that first query -- the only
//  difference: on a VTEST (915 / 919) machine golden's very first query still used the non-VTEST file names.
//  ⚠ #17 ELA-3, 暫照 golden（A）: at boot ReadConfig WRITES every missing key into the real D:\HT9045\config\config.ini
//    (golden CheckAndReadIniData), and Machine ID into Gerneral.ini if it is missing.  Options::jamWriteBack=false stops it.
//  HT9045_ELA=0 opts a run out (no hub, the route answers 503) -- for SIM regressions that must not read the logs.
// ===========================================================================
#include "EventLogAnalysis/ElaService.h"

#include <windows.h>   // SYSTEMTIME / GetLocalTime (ElaHub.h already has it through WebBridge/Sync.h)
#include <cstdio>
#include <cstdlib>
#include <string>

namespace {

ela::Hub* g_hub = 0;
std::string g_handlerId = "HT-90xx";

std::string QueryParam(const std::string& query, const char* name)
{
    const std::string key = std::string(name) + "=";
    size_t p = 0;
    while (p < query.size())
    {
        size_t e = query.find('&', p);
        if (e == std::string::npos) e = query.size();
        if (query.compare(p, key.size(), key) == 0)
        {
            std::string v = query.substr(p + key.size(), e - p - key.size()), o;
            for (size_t i = 0; i < v.size(); ++i)
            {
                if (v[i] == '+') o += ' ';
                else if (v[i] == '%' && i + 2 < v.size()) { o += (char)std::strtol(v.substr(i + 1, 2).c_str(), 0, 16); i += 2; }
                else o += v[i];
            }
            return o;
        }
        p = e + 1;
    }
    return std::string();
}

void TodayUntilNow(ela::QueryRequest* q)
{
    SYSTEMTIME t;
    ::GetLocalTime(&t);
    q->startDate = ela::EncodeDate(t.wYear, t.wMonth, t.wDay);
    q->startTime = 0.0;                                                       // dtpStartTime = EncodeTime(0,0,0,0)
    q->endDate = q->startDate;
    q->endTime = ela::EncodeTime(t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);   // dtpEndTime = Now()
}

}  // namespace

namespace ela {

bool ServeHttp(Hub* hub, const QueryRequest& base, const std::string& method, const std::string& path,
               const std::string& query, bool allowCmd, int* status, std::string* contentType, std::string* body)
{
    const std::string root = "/api/ela";
    if (path.compare(0, root.size(), root) != 0 || (path.size() > root.size() && path[root.size()] != '/'))
        return false;
    *contentType = "application/json; charset=utf-8";
    if (!hub)
    {
        *status = 503;
        *body = "{\"error\":\"event log analyzer not running (HT9045_ELA=0 or not started)\"}";
        return true;
    }
    const std::string sub = path.size() > root.size() + 1 ? path.substr(root.size() + 1) : std::string();
    if ((method == "GET" || method == "HEAD") && (sub.empty() || sub == "/"))
    {
        *status = 200;
        // ?since=<seq> (eventlog.html polls every second): seq moves after every job, so the same seq means history,
        // config and result are unchanged and only busy can differ -- answer without copying the snapshot (up to
        // 5,000 rows per grid).  AI(W906-ELA-P4) 20260927
        const std::string since = QueryParam(query, "since");
        const unsigned long seq = hub->Seq();
        if (!since.empty() && std::strtoul(since.c_str(), 0, 10) == seq)
        {
            char b[80];
            std::snprintf(b, sizeof(b), "{\"seq\":%lu,\"busy\":%s,\"unchanged\":true}", seq, hub->Busy() ? "true" : "false");
            *body = b;
            return true;
        }
        *body = hub->SnapshotJson(0);
        return true;
    }
    if (method == "POST" && sub == "query")
    {
        if (!allowCmd)
        {
            // a query may write JAM0000.dat defaults (#17 A), so it follows --allow-cmd like the other POSTs
            *status = 403;
            *body = "{\"error\":\"read-only server\"}";
            return true;
        }
        QueryRequest q = base;
        double from = 0.0, to = 0.0;
        if (!StrToDateTimeZhTw(QueryParam(query, "from"), &from) || !StrToDateTimeZhTw(QueryParam(query, "to"), &to))
        {
            *status = 400;
            *body = "{\"error\":\"from / to must be YYYY/MM/DD HH:NN:SS\"}";
            return true;
        }
        q.startDate = from;
        q.startTime = from;
        q.endDate = to;
        q.endTime = to;
        q.topFilter = std::atoi(QueryParam(query, "top").c_str());
        q.rgFilter = std::atoi(QueryParam(query, "filter").c_str());
        const std::string area = QueryParam(query, "area"), func = QueryParam(query, "func");
        for (int i = 0; i < eUnitNameTotal; ++i) q.area[i] = area.empty() || ((size_t)i < area.size() && area[i] == '1');
        for (int i = 0; i < eByFuncTotal; ++i) q.func[i] = func.empty() || ((size_t)i < func.size() && func[i] == '1');
        const int row = std::atoi(QueryParam(query, "row").c_str());
        q.top5Row = row > 0 ? row : 1;
        hub->PostQuery(q);
        *status = 202;
        *body = "{\"queued\":true}";
        return true;
    }
    *status = 404;
    *body = "{\"error\":\"GET /api/ela or POST /api/ela/query\"}";
    return true;
}

}  // namespace ela

void W906_ElaStart()
{
    if (g_hub) return;
    const char* optOut = std::getenv("HT9045_ELA");
    if (optOut && optOut[0] == '0' && optOut[1] == '\0') return;
    ela::Options o;                      // #15-#19 暫照 golden（A）
    ela::HubPaths p;
    int w = 0;
    g_handlerId = ela::FileExistsA(p.generalIni)
                      ? ela::IniCheckAndReadString(p.generalIni, "Version", "Machine ID", "HT-90xx", o.jamWriteBack, &w)
                      : std::string("HT-90xx");
    g_hub = new ela::Hub(o, p);
    ela::QueryRequest q;
    TodayUntilNow(&q);
    q.handlerId = g_handlerId;
    g_hub->PostQuery(q);
    g_hub->Post(ela::EL_UPDATE_PARAMETER);
    if (!g_hub->Start())
        std::printf("[ELA] worker thread did not start -- the hub answers /api/ela but runs nothing\n");
    else
        std::printf("[ELA] Event Log Analyzer hub started (EventlogAnalyzer Rev891.0 port; reports / uploads HOLD)\n");
}

void W906_ElaStop()
{
    if (!g_hub) return;
    ela::Hub* h = g_hub;
    g_hub = 0;
    h->Stop();
    delete h;
}

void W906_ElaPost(int cmd)
{
    // golden SendCommand_EventLog -> WM_COPYDATA -> OnMyCopyMsg (InterfaceSYS still sends that too, for an external exe);
    // no hub = no-op
    if (g_hub) g_hub->Post(cmd);
}

bool W906_ElaHttp(const std::string& method, const std::string& path, const std::string& query, bool allowCmd,
                  int* status, std::string* contentType, std::string* body)
{
    if (path.compare(0, 8, "/api/ela") != 0) return false;   // every request that falls this far passes here
    ela::QueryRequest base;      // golden labPath / lblProdPath (W906_EVENTLOG_ROOT / W906_PRODLOG_ROOT when set)
    base.handlerId = g_handlerId;
    return ela::ServeHttp(g_hub, base, method, path, query, allowCmd, status, contentType, body);
}
