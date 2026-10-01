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
//  be started by hand and does the three report / upload jobs this port still holds back (#22 R3 and later;
//  EL_VTEST_MTBF_SUM runs here since R2, AI(W906-ELA-R2) 20260927; EL_UPLOAD_JAMWEEK / EL_UPLOAD_BYFILE_N10 since R4,
//  AI(W906-ELA-R4) 20260927).  #31 = A (user 20260927): V906 never launches it.
//  Golden start-up (TfrmELA::FormShow :418-424 + Timer1Timer :443-485): sHandlerID from Gerneral.ini [Version] Machine ID
//  ("HT-90xx" when the file is missing), then one query for today 00:00 .. now (cbTopAlarmFilter 0, rgFilter 0, every
//  box checked), ReadConfig, and bEL_UPDATE_PARAMETER.  Here: the query is posted and EL_UPDATE_PARAMETER is raised;
//  the Hub runs the raised job first, so ReadConfig (and sCustCode) comes before that first query -- the only
//  difference: on a VTEST (915 / 919) machine golden's very first query still used the non-VTEST file names (moot with
//  the default W19 B file rule, which has no VTEST branch).
//  ⚠ #17 W17 = A (Steven 20260927): at boot ReadConfig WRITES every missing key into the real D:\HT9045\config\config.ini
//    (golden CheckAndReadIniData), and Machine ID into Gerneral.ini if it is missing.  Options::jamWriteBack=false stops it.
//  HT9045_ELA=0 opts a run out (no hub, the route answers 503) -- for SIM regressions that must not read the logs.
// ===========================================================================
#include "EventLogAnalysis/ElaService.h"
#include "EventLogAnalysis/ElaReports.h"    // AutoJobsEnabled / EffectiveO10Source (R6 schedule route)
#include "EventLogAnalysis/ElaSchedule.h"   // ScheduleStatusJson / ScheduleRunNow (R6)
#include "EventLogAnalysis/ElaFileUtil.h"   // AI(W906-ELA-R4) 20260927 (St02-E): SetDbiProcessHook (MyDBIProcess -> FTP_Log)
#include "JamRules.h"                        // AI(W906-ELA-W45) 20260927 (St02-E): Jam0000Path(), the W906_JAM0000_PATH seam
#include "EventLogAnalysis/ElaSchedule.h"   // AI(W906-ELA-R5) 20260927 (St02-E): R5 ElaSchedule wiring (ScheduleInstall / UseHubJobs / Uninstall)
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

std::string JStr(const std::string& s)   // AI(W906-ELA-P7a) 20260928: a JSON string (paths have backslashes)
{
    std::string o = "\"";
    for (size_t i = 0; i < s.size(); ++i)
    {
        const unsigned char c = (unsigned char)s[i];
        if (c == '"' || c == '\\') { o += '\\'; o += (char)c; }
        else if (c < 0x20) { char b[8]; std::snprintf(b, sizeof(b), "\\u%04x", c); o += b; }
        else o += (char)c;
    }
    return o + "\"";
}

// Save Summary's file-name part: golden's SaveDialog takes any name; from the web only a plain name (no folder, so no
// path can be sent) of ASCII letters / digits / space - _ . ( ), up to 100, and no ".." (AI(W906-ELA-P7a) 20260928)
bool SummaryNameOk(const std::string& n)
{
    if (n.size() > 100 || n.find("..") != std::string::npos) return false;
    for (size_t i = 0; i < n.size(); ++i)
    {
        const unsigned char c = (unsigned char)n[i];
        const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == ' ' || c == '-' ||
                        c == '_' || c == '.' || c == '(' || c == ')';
        if (!ok) return false;
    }
    return true;
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
            // a query may write JAM0000.dat defaults (#17 A).  Only a caller that passes allowCmd=false gets here: wb_serve
            // passes g_tcAllowCmd = allowCmd, which is always true since ZEROARG (wb_serve.cpp:3670; no flag turns it off).
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
    // AI(W906-ELA-R6) 20260928 (St02-E): the automatic-jobs panel of eventlog.html.  A route of its own, not in the
    //   snapshot: the schedule moves without the hub's seq, so ?since would hide it.
    if ((method == "GET" || method == "HEAD") && sub == "schedule")
    {
        const std::string sched = ScheduleStatusJson();                  // "null" when the scheduler is not installed
        // AI(W906-ELA-W36) 20260928 (St02-E helper): "build" comes from the build flag and "ftp" from the transport; they
        //   were the same bit while ★W36 A held (the sim build logged only).  ★W36 = C: "WinINet" in both builds.
#ifdef W906_NO_SOFT_SIMULTE
        const bool ship = true;
#else
        const bool ship = false;
#endif
        const bool wininet = BuildFtpTransport() == kFtpTransportWinInet;
        *status = 200;
        *body = "{\"installed\":" + std::string(sched == "null" ? "false" : "true") + ",\"build\":\"" +
                (ship ? "ship" : "sim") + "\",\"ftp\":\"" + (wininet ? "WinINet" : "log-only") + "\",\"o10\":" +
                (AutoJobsEnabled(hub->Config()) ? "true" : "false") + ",\"o10Source\":\"" + EffectiveO10Source() +
                "\",\"schedule\":" + sched + "}";
        return true;
    }
    // POST /api/ela/job?id=<O06-4 | N10-3 | N25-3 | O19-VTEST | N17-UploadProdLog | N25-4 | N25-5> -- once at the next tick.  D-a:
    //   the page's own actions are not O10-gated; the job's switch and customer code still are (JobEnabled automatic=false).
    if (method == "POST" && sub == "job")
    {
        if (!allowCmd)
        {
            *status = 403;
            *body = "{\"error\":\"read-only server\"}";
            return true;
        }
        const std::string id = QueryParam(query, "id");
        int job = -1;
        for (int j = 0; j < JOB_TOTAL; ++j)
            if (id == JobName(j)) job = j;
        if (job < 0)
        {
            *status = 400;
            *body = "{\"error\":\"id must be O06-4, N10-3, N25-3, O19-VTEST, N17-UploadProdLog, N25-4 or N25-5\"}";   // W22
            return true;
        }
        if (!ScheduleRunNow(job))
        {
            *status = 503;
            *body = "{\"error\":\"the ELA scheduler is not installed\"}";
            return true;
        }
        *status = 202;
        *body = "{\"queued\":true,\"job\":\"" + std::string(JobName(job)) + "\"}";
        return true;
    }
    // AI(W906-ELA-P7a) 20260928 (St02-E): the Save Summary button (golden btnSaveSummaryClick, Rev891 Analyzer.cpp:2557-2570)
    //   -- queued for the hub worker (Hub::PostSaveSummary), never run on the HTTP thread
    if (method == "POST" && sub == "summary")
    {
        if (!allowCmd)
        {
            *status = 403;
            *body = "{\"error\":\"read-only server\"}";
            return true;
        }
        const std::string name = QueryParam(query, "name");
        if (!SummaryNameOk(name))
        {
            *status = 400;
            *body = "{\"error\":\"name: up to 100 of A-Z a-z 0-9 space - _ . ( ), no folder, no ..\"}";
            return true;
        }
        hub->PostSaveSummary(name);
        *status = 202;
        *body = "{\"queued\":true,\"folder\":" + JStr(hub->SummaryDir()) + ",\"name\":" + JStr(name) + "}";
        return true;
    }
    *status = 404;
    *body = "{\"error\":\"GET /api/ela, GET /api/ela/schedule, POST /api/ela/query, /api/ela/job or /api/ela/summary\"}";
    return true;
}

}  // namespace ela

void W906_ElaStart()
{
    if (g_hub) return;
    const char* optOut = std::getenv("HT9045_ELA");
    if (optOut && optOut[0] == '0' && optOut[1] == '\0') return;
    ela::Options o;                      // defaults = Steven's 20260927 rulings: W15 B, W17 A, W18 B, W19 B (ElaCore.h)
    o.jamIniPath = jamrules::Jam0000Path();   // AI(W906-ELA-W45): W906_JAM0000_PATH when set, else golden's literal
    ela::HubPaths p;
    int w = 0;
    g_handlerId = ela::FileExistsA(p.generalIni)
                      ? ela::IniCheckAndReadString(p.generalIni, "Version", "Machine ID", "HT-90xx", o.jamWriteBack, &w)
                      : std::string("HT-90xx");
    g_hub = new ela::Hub(o, p);
    // AI(W906-ELA-W36) 20260928 (St02-E helper): ★W36 = C (Steven) -- both builds install WinINet.  Until then the sim
    //   build installed NewLogOnlyFtp here (#if W906_ELA_FTP_WININET; ElaFtp.h keeps the history).
    g_hub->SetFtpFactory(&ela::NewWinInetFtp);   // AI(W906-ELA-R4) 20260927 (St02-E): the only place WinINet is installed
    ela::SetDbiProcessHook(&ela::FtpLogDbiLine);   ela::ScheduleInstall(ela::ScheduleSetup());   ela::ScheduleUseHubJobs(g_hub, o);   // AI(W906-ELA-R4): golden MyDBIProcess also writes FTP_Log (Common.cpp:39)  |   // AI(W906-ELA-R5) 20260927 (St02-E): R5 ElaSchedule wiring: installed before the worker starts
    ela::QueryRequest q;
    TodayUntilNow(&q);
    q.handlerId = g_handlerId;
    g_hub->PostQuery(q);
    g_hub->Post(ela::EL_UPDATE_PARAMETER);
    if (!g_hub->Start())
        std::printf("[ELA] worker thread did not start -- the hub answers /api/ela but runs nothing\n");
    else
        std::printf("[ELA] Event Log Analyzer hub started (EventlogAnalyzer Rev891.0 port; VTEST MTBF summary on; N25-3/4/5 / "
                    "N10 BYFILE uploads: %s transport; timed jobs: ElaSchedule, gated by the auto-save switch)\n",
                    ela::BuildFtpTransport() == ela::kFtpTransportWinInet ? "WinINet" : "log-only");   // AI(W906-ELA-R2) + AI(W906-ELA-R4) 20260927; AI(W906-ELA-W36) 20260928: WinINet in both builds
}

void W906_ElaStop()
{
    if (!g_hub) return;
    ela::Hub* h = g_hub;
    g_hub = 0;
    h->Stop();   ela::ScheduleUninstall();   // AI(W906-ELA-R5) 20260927 (St02-E): R5 ElaSchedule wiring: after the worker is joined
    delete h;
    ela::SetDbiProcessHook(0);   // AI(W906-ELA-R4) 20260927 (St02-E)
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

// AI(W906-B20-ELAWININET) 20261001: ela::IniBoolOverride and W906_ElaSetBoolOverride (St02 W58, AI(W906-SIM-W36-1) 20260928 /
//   W58 20260930) moved, unchanged, to EventLogAnalysis/ElaIniOverride.cpp.  Here they dragged this object -- and with
//   it ela::NewWinInetFtp (W906_ElaStart) and WININET.DLL -- into every program that links ElaHub / ElaSchedule
//   (ELA_Ftp "wininet.dll is not loaded", gate b20i).  Do not put them back.
