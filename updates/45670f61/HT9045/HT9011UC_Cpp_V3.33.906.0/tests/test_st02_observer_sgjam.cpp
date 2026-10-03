// =============================================================================
//  test_st02_observer_sgjam.cpp -- E-019 OB-7: Data.Observer > System Message > SG_JamCount (ctest St02_ObserverSGJam)
//
//  AI(W906-ST02-OB7) 20261002 (St02-E helper) new file.
//    Plan D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\C7_FTPSAVE_SGJAM_PLAN_20261002.md section 2.4 (4).
//  golden 906_0625 = D:\HT9045\HT9011UC_Code_V3.33.906.0_20260625_Steven (cp950): cObserver.cpp:5361-5369 (the two
//    buttons), StatisticalJamCount :5060-5276, StatisticalJamCountEnable :5301-5329, constructor grid header :305-317,
//    the event-log CSV header main.cpp:1509-1511.
//  Under test: JsonBridge/actions/ObserverSGJam.cpp reached through the REAL act.* dispatch JsonBridge/ChanAction.cpp
//    HandleActionWithTag (:347 same-line dispatch), calling jimmychiu's TfObserver members (cObserver.cpp:3605 / :3610 ->
//    :3320) on the static fObserver (cObserver.cpp:3289).  "ok" = the reply carries "executed":true, as wb_serve.cpp does.
//    [0] containment: the ctest redirect roots are in machine_log_scratch (else exit 2, nothing called); the sandbox is
//        <W906_EVENTLOG_ROOT>\st02_ob7; W906_EVENTLOG_ROOT is pointed at the case's own root (getenv is read at call time)
//    [1] the static fObserver: strngrdJamLog has golden's constructor header (ColCount 6), labLoaderCount Caption empty
//    [2] refusals: not-open, unknown op, act.nope.nope and act.observer.* not taken by this dispatcher, bad payload,
//        slEventLog NULL (nothing written), dryRun (guards only, nothing written)
//    [3] queryNow: the grid (golden's trailing blank row), Rate, Loader Count, the RawData.csv path and lines
//    [4] state: read only
//    [5] queryYesterday: yesterday's CSV counted, iOneDayLoaderCount -> 0 (golden :5262-5263), LoaderCount column 100
//    [6] [N26] FTP tail reported as skipped (gated Q5a; S25)
//    [7] yesterday's CSV missing: golden's early return (:5106-5111), counter and grid untouched
//    [8] InitialOK false: golden returns at once (:5062-5065), nothing written
//    [9] JamCountEnable.ini 03=0 seeded where StatisticalJamCountEnable reads it: JAM03xx not counted (golden :5308-5328)
//    [10] source ratchets: ChanAction.cpp dispatch before the line's //, Data.Observer.html script order, the page js ops
//    [11] D:\HT9045_Log\EventLogTxt\SGJamCount unchanged before / after
//  The day is sampled once at start (GetTimeInfo / GetYesterdayInfo); a run across midnight can fail [3] / [5].
// =============================================================================
#include "JsonBridge/ChanAction.h"
#include "JsonBridge/actions/ObserverSGJam.h"
#include "forms/fObserver.h"
#include "Public/MyStringList.h"
#include "cmydef.h"
#include "cpublic.h"
#include "Config.h"
#include "Public/cJSON.h"
#include "w906_ctest_guard.h"

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);   // MinGW.org strict mode hides it (tests/test_agv_e84.cpp:158-163 guard)
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

extern AnsiString as9045LogPath;
extern AnsiString asSaveEventLogPath;
extern AnsiString asGeneralPath;
extern AnsiString AuthPath;

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool Has(const std::string& h, const char* n) { return h.find(n) != std::string::npos; }
bool AckOk(const std::string& r) { return Has(r, "\"executed\":true"); }

std::string Act(const char* op, const std::string& value)
{
    const std::string cmd = std::string("act.observerSG.") + op;
    const std::string r = ht9045::sjson::HandleActionWithTag(cmd, value, std::string());
    std::printf("    %s %s -> %s\n", cmd.c_str(), value.c_str(), r.substr(0, 300).c_str());
    return r;
}

// ---- tiny JSON reads -------------------------------------------------------------------------------------------------------
struct J {
    cJSON* root;
    explicit J(const std::string& s) : root(cJSON_Parse(s.c_str())) {}
    ~J() { if (root) cJSON_Delete(root); }
    J(const J&) = delete;
    J& operator=(const J&) = delete;
    const cJSON* at(const char* path) const {   // "a.b.c"
        const cJSON* n = root;
        std::string p = path;
        std::size_t i = 0;
        while (n) {
            const std::size_t d = p.find('.', i);
            const std::string seg = p.substr(i, d == std::string::npos ? std::string::npos : d - i);
            n = cJSON_GetObjectItemCaseSensitive(n, seg.c_str());
            if (d == std::string::npos) break;
            i = d + 1;
        }
        return n;
    }
    std::string str(const char* path) const { const cJSON* n = at(path); return (n && cJSON_IsString(n)) ? n->valuestring : "<none>"; }
    int num(const char* path) const { const cJSON* n = at(path); return (n && cJSON_IsNumber(n)) ? (int)n->valuedouble : -999; }
    bool isTrue(const char* path) const { const cJSON* n = at(path); return n && cJSON_IsTrue(n); }
    bool isFalse(const char* path) const { const cJSON* n = at(path); return n && cJSON_IsFalse(n); }
    bool has(const char* path) const { return at(path) != 0; }
    int count(const char* path) const { const cJSON* n = at(path); return (n && cJSON_IsArray(n)) ? cJSON_GetArraySize(n) : -1; }
    std::string cell(int r, int c) const {
        const cJSON* cells = at("grid.cells");
        const cJSON* row = (cells && cJSON_IsArray(cells)) ? cJSON_GetArrayItem(cells, r) : 0;
        const cJSON* v = (row && cJSON_IsArray(row)) ? cJSON_GetArrayItem(row, c) : 0;
        return (v && cJSON_IsString(v)) ? v->valuestring : "<none>";
    }
    std::string skipped0() const {
        const cJSON* a = at("skipped");
        const cJSON* v = (a && cJSON_IsArray(a)) ? cJSON_GetArrayItem(a, 0) : 0;
        return (v && cJSON_IsString(v)) ? v->valuestring : "";
    }
};

// ---- files -----------------------------------------------------------------------------------------------------------------
bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}
void WriteAll(const std::string& p, const std::string& s) { std::ofstream f(p.c_str(), std::ios::binary); f << s; }
bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
void MakeDirs(const std::string& p)
{
    for (std::size_t i = 3; i <= p.size(); ++i)
        if (i == p.size() || p[i] == '\\' || p[i] == '/') ::CreateDirectoryA(p.substr(0, i).c_str(), 0);
}
std::string Slashes(std::string s) { for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\'; return s; }
std::string DirOf(const std::string& p) { const std::size_t k = p.find_last_of("\\/"); return k == std::string::npos ? p : p.substr(0, k); }

// delete everything under dir (dir must be inside ctest's machine_log_scratch and be this test's st02_ob7 sandbox)
void Wipe(const std::string& dir)
{
    if (!W906CtestGuardInScratch(dir.c_str()) || dir.find("st02_ob7") == std::string::npos) { std::printf("  (wipe refused: %s)\n", dir.c_str()); return; }
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        const std::string p = dir + "\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { Wipe(p); ::RemoveDirectoryA(p.c_str()); }
        else ::DeleteFileA(p.c_str());
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

// every file under root (relative name -> size:mtime); a missing root = empty
void ListTree(const std::string& root, const std::string& rel, std::map<std::string, std::string>* out)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((root + "\\" + rel + "*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { (*out)[rel + name + "\\"] = "dir"; ListTree(root, rel + name + "\\", out); continue; }
        char b[64];
        std::snprintf(b, sizeof(b), "%lu:%lu:%lu", (unsigned long)fd.nFileSizeLow, (unsigned long)fd.ftLastWriteTime.dwHighDateTime,
                      (unsigned long)fd.ftLastWriteTime.dwLowDateTime);
        (*out)[rel + name] = b;
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

std::vector<std::string> SplitLines(const std::string& s)
{
    std::vector<std::string> v;
    std::string cur;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\r') continue;
        if (s[i] == '\n') { v.push_back(cur); cur.clear(); continue; }
        cur += s[i];
    }
    if (!cur.empty()) v.push_back(cur);
    return v;
}
int CountLinesWith(const std::string& s, const char* n)
{
    int k = 0;
    const std::vector<std::string> v = SplitLines(s);
    for (std::size_t i = 0; i < v.size(); ++i) if (Has(v[i], n)) ++k;
    return k;
}
std::string StripHtmlComments(const std::string& s)
{
    std::string o;
    std::size_t i = 0;
    while (i < s.size()) {
        const std::size_t a = s.find("<!--", i);
        if (a == std::string::npos) { o += s.substr(i); break; }
        o += s.substr(i, a - i);
        const std::size_t b = s.find("-->", a + 4);
        if (b == std::string::npos) break;
        i = b + 3;
    }
    return o;
}

// ---- golden's file names ---------------------------------------------------------------------------------------------------
struct Day { int y; int m; int d; };
std::string EventCsv(const std::string& evRoot, const Day& t)   // golden :5102-5105 with Path = evRoot, FileName = "EventLogTxt"
{
    char b[64];
    std::snprintf(b, sizeof(b), "\\%04d\\%02d\\EventLogTxt_%04d%02d%02d.csv", t.y, t.m, t.y, t.m, t.d);
    return evRoot + b;
}
std::string RawCsv(const std::string& evRoot, const Day& t)     // MySaveSGJamCountToFile, HandlerID "" -> "HandlerID" (:5191-5194)
{
    char b[80];
    std::snprintf(b, sizeof(b), "\\SGJamCount\\%04d\\%02d\\HandlerID_%04d%02d%02d_RawData.csv", t.y, t.m, t.y, t.m, t.d);
    return evRoot + b;
}
// golden main.cpp:1509-1511 header (non-SPIL); fields [2] UnitName, [3] AlarmCode, [7] Message.  Fields with spaces quoted
// (CommaText splits an unquoted field at spaces, as VCL).
const char* const kCsvHeader = "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n";
void SeedCsv(const std::string& path, const std::string& rows)
{
    MakeDirs(DirOf(path));
    WriteAll(path, std::string(kCsvHeader) + rows);
}
const char* const kTodayRows =
    "2026/10/02,09:00:00,InArm,JAM0301,1,12,0,\"Loader, jam near site A\",InArm1,R1\r\n"
    "2026/10/02,09:05:00,OutArm,WAR16102,1,3,0,\"Clean pad worn out\",OutArm2,R1\r\n"
    "2026/10/02,09:10:00,InArm,JAM0301,1,7,0,\"Loader, jam near site A\",InArm1,R1\r\n"
    "2026/10/02,09:15:00,Shuttle,JAM0302,1,5,0,\"Shuttle jam\",Shuttle1,R1\r\n"
    "2026/10/02,09:20:00,Index,JAM0001,1,5,0,\"Index jam\",Index1,R1\r\n"
    "2026/10/02,09:25:00,Tray,JAM2001,1,5,0,\"Tray jam\",Tray1,R1\r\n";
const char* const kYesterdayRows =
    "2026/10/01,23:00:00,OutArm,JAM1901,1,4,0,\"Out arm jam\",OutArm1,R1\r\n"
    "2026/10/01,23:05:00,OutArm,MES0001,1,0,0,\"Message only\",OutArm1,R1\r\n";

char g_envBuf[1024];
void SetEventLogRoot(const std::string& root)   // the string must stay alive: putenv keeps the pointer
{
    std::snprintf(g_envBuf, sizeof(g_envBuf), "W906_EVENTLOG_ROOT=%s", root.c_str());
    HT9045_TEST_PUTENV(g_envBuf);
}

}  // namespace

int main(int argc, char** argv)
{
    const char* ev = std::getenv("W906_EVENTLOG_ROOT");
    const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(),
                               "asGeneralPath", asGeneralPath.c_str(), "AuthPath", AuthPath.c_str(), 0 };
    if (!W906TestRequireCtestRedirects("St02_ObserverSGJam", rt))
        return 2;
    const std::string portRoot = argc > 1 ? argv[1] : "";
    const std::string webPage = argc > 2 ? argv[2] : "";
    const std::string sand = Slashes(std::string(ev ? ev : "")) + "\\st02_ob7";
    if (!W906CtestGuardInScratch(sand.c_str())) { std::printf("REFUSED: sandbox %s is not in machine_log_scratch\n", sand.c_str()); return 2; }
    static char s_origEnv[1024];
    std::snprintf(s_origEnv, sizeof(s_origEnv), "W906_EVENTLOG_ROOT=%s", ev ? ev : "");

    std::map<std::string, std::string> real0, real1;
    const std::string realSg = "D:\\HT9045_Log\\EventLogTxt\\SGJamCount";
    ListTree(realSg, "", &real0);

    std::printf("[0] containment\n");
    Wipe(sand);
    MakeDirs(sand);
    const std::string rootA = sand + "\\A\\EventLogTxt";   // case A: the event log and SGJamCount under one root, as golden's D:\HT9045_Log\EventLogTxt
    const std::string rootB = sand + "\\B\\EventLogTxt";   // case [9]: its own JamCountEnable.ini
    SetEventLogRoot(rootA);
    Check(std::getenv("W906_EVENTLOG_ROOT") != 0 && std::string(std::getenv("W906_EVENTLOG_ROOT")) == rootA,
          "W906_EVENTLOG_ROOT points at the case root " + rootA);

    GetTimeInfo();
    GetYesterdayInfo();
    const Day today = { (int)SystemYear, (int)SystemMonth, (int)SystemDate };
    const Day yday = { (int)SystemYearYesterday, (int)SystemMonthYesterday, (int)SystemDateYesterday };

    TfObserver* f = fObserver;
    const bool savedInitialOK = InitialOK;
    TMyStringList* const savedSl = slEventLog;
    const int savedLoader = iOneDayLoaderCount;
    const AnsiString savedHandlerID = IniConfig.asA32_1_HandlerID;
    const bool savedN26 = IniConfig.bN26_UseJamRawDataUpdataToFTP;
    const bool savedShow = f ? f->bShow : false;
    IniConfig.asA32_1_HandlerID = "";
    IniConfig.bN26_UseJamRawDataUpdataToFTP = false;

    std::printf("[1] the static fObserver (cObserver.cpp:3289)\n");
    Check(f != 0 && f->strngrdJamLog != 0 && f->labLoaderCount != 0, "fObserver, strngrdJamLog and labLoaderCount exist");
    if (f == 0 || f->strngrdJamLog == 0 || f->labLoaderCount == 0) { std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail); return 1; }
    {
        const char* const hdr[6] = { "No", "UnitName", "AlarmCode", "Message", "Count", "Rate (%)" };
        bool same = (int)f->strngrdJamLog->ColCount == 6;
        for (int c = 0; c < 6; ++c) same = same && f->strngrdJamLog->Cells[c][0] == hdr[c];
        Check(same, "golden constructor :305-317 ran at static init: ColCount 6 and the six header cells");
        Check((int)f->strngrdJamLog->RowCount == 5, "RowCount 5 (TStringGrid default; dfm sets none)");
        Check(f->labLoaderCount->Caption == "", "labLoaderCount Caption starts empty (golden writes it at :5268; the page keeps the dfm text)");
    }

    std::printf("[2] refusals\n");
    {
        f->bShow = false;
        std::string r = Act("queryNow", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-open\""), "Observer not open -> not-open");
        f->bShow = true;                                         // golden FormShow :350 (not run here)
        r = Act("nope", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"unknown-action\"") && Has(r, "queryYesterday"), "unknown op -> unknown-action with the op list");
        r = ht9045::sjson::HandleActionWithTag("act.nope.nope", "{}", std::string());
        Check(Has(r, "\"guard\":\"unknown-action\"") && !Has(r, "queryYesterday"), "act.nope.nope still ChanAction's unknown-action");
        r = ht9045::sjson::HandleActionWithTag("act.observer.nope", "{}", std::string());
        Check(Has(r, "unknown-action") && Has(r, "prSave") && !Has(r, "queryYesterday"),
              "act.observer.* still goes to E-021's W906_ObserverAct (the 13th character of act.observerSG. is 'S')");
        r = Act("queryNow", "[1]");
        Check(!AckOk(r) && Has(r, "\"guard\":\"bad-payload\""), "value not an object -> bad-payload");
        InitialOK = true;
        slEventLog = 0;
        r = Act("queryNow", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"log-objects-not-created\""), "slEventLog NULL -> log-objects-not-created");
        std::map<std::string, std::string> t;
        ListTree(sand, "", &t);
        Check(t.empty(), "nothing written under the sandbox by the refusals");

        slEventLog = new TMyStringList(AnsiString(rootA.c_str()), "EventLogTxt", "hdr");   // golden main.cpp:1509-1510 shape (Path, "EventLogTxt")
        SeedCsv(EventCsv(rootA, today), kTodayRows);
        r = Act("queryNow", "{\"dryRun\":true}");
        J j(r);
        Check(!AckOk(r) && j.str("guard") == "" && j.isTrue("dryRun") && j.isTrue("eventLogCsv.exists") &&
              j.str("eventLogCsv.path") == EventCsv(rootA, today), "dryRun: guards only, the CSV path golden reads (:5102-5105) and that it exists");
        Check(j.num("grid.rows") == 5 && !Exists(rootA + "\\SGJamCount") && !Exists(RawCsv(rootA, today)), "dryRun: StatisticalJamCount not called (grid 5 rows, no SGJamCount folder)");
    }

    std::printf("[3] queryNow (golden btnSG_QueryNowClick :5361-5364)\n");
    std::string raw3;
    {
        iOneDayLoaderCount = 100;
        const std::string r = Act("queryNow", "{}");
        J j(r);
        Check(AckOk(r) && j.str("early") == "" && j.isTrue("initialOk"), "executed, golden ran to the end");
        Check(j.num("grid.rows") == 4 && j.num("grid.cols") == 6 && j.count("grid.cells") == 4, "4 rows: header, JAM0301, JAM0302 and golden's trailing blank row (:5160)");
        Check(j.cell(0, 0) == "No" && j.cell(0, 5) == "Rate (%)", "row 0 is the header");
        Check(j.cell(1, 0) == "1" && j.cell(1, 1) == "InArm" && j.cell(1, 2) == "JAM0301" && j.cell(1, 3) == "Loader, jam near site A" &&
              j.cell(1, 4) == "2" && j.cell(1, 5) == "2.00", "row 1: JAM0301 counted twice, Rate 2/100 -> 2.00 (ChangeToFloat %)");
        Check(j.cell(2, 0) == "2" && j.cell(2, 1) == "Shuttle" && j.cell(2, 2) == "JAM0302" && j.cell(2, 3) == "Shuttle jam" &&
              j.cell(2, 4) == "1" && j.cell(2, 5) == "1.00", "row 2: JAM0302 once, 1.00");
        bool blank = true;
        for (int c = 0; c < 6; ++c) blank = blank && j.cell(3, c) == "";
        Check(blank, "row 3 blank; WAR16102 / JAM0001 / JAM2001 not counted (JAM01..JAM19 only, :5322-5324)");
        Check(j.str("labLoaderCount") == "100" && j.num("iOneDayLoaderCount") == 100 && j.num("loaderCountBefore") == 100 && iOneDayLoaderCount == 100,
              "Loader Count 100, unchanged by Query Now");
        Check(j.str("sgJamCountCsv") == RawCsv(rootA, today) && Exists(RawCsv(rootA, today)), "RawData.csv at " + RawCsv(rootA, today));
        Check(ReadAll(RawCsv(rootA, today), &raw3) && SplitLines(raw3).size() == 3 &&
              SplitLines(raw3)[0] == "Date, Time, No, UnitName, AlarmCode, Message, Count, Rate (%), LoaderCount" &&
              CountLinesWith(raw3, "JAM0301") == 1 && CountLinesWith(raw3, "JAM0302") == 1, "RawData.csv: golden's header (:5203) and one line per code");
        Check(j.count("skipped") == 0, "nothing skipped on Query Now");
        Check(Exists(rootA + "\\SGJamCount\\JamCountEnable.ini"), "JamCountEnable.ini seeded under the redirect root (CheckAndReadIniData)");
    }

    std::printf("[4] state (read only)\n");
    {
        const std::string r = Act("state", "");
        J j(r);
        std::string again;
        Check(AckOk(r) && j.num("grid.rows") == 4 && j.cell(1, 2) == "JAM0301" && j.str("labLoaderCount") == "100" && !j.has("dryRun"),
              "state returns the form's grid as it is");
        Check(ReadAll(RawCsv(rootA, today), &again) && again == raw3, "state wrote nothing");
    }

    std::printf("[5] queryYesterday (golden btnSG_QueryYesterdayClick :5366-5369)\n");
    std::string raw5;
    {
        SeedCsv(EventCsv(rootA, yday), kYesterdayRows);
        iOneDayLoaderCount = 100;
        const std::string r = Act("queryYesterday", "{}");
        J j(r);
        Check(AckOk(r) && j.str("early") == "" && j.str("eventLogCsv.path") == EventCsv(rootA, yday), "executed on yesterday's CSV");
        Check(j.num("grid.rows") == 3 && j.cell(1, 2) == "JAM1901" && j.cell(1, 1) == "OutArm" && j.cell(1, 4) == "1" && j.cell(1, 5) == "1.00" &&
              j.cell(2, 2) == "", "grid: JAM1901 once (Rate computed before the reset), the old JAM0302 row cleared");
        Check(iOneDayLoaderCount == 0 && j.num("iOneDayLoaderCount") == 0 && j.str("labLoaderCount") == "0" && j.num("loaderCountBefore") == 100,
              "iOneDayLoaderCount -> 0 and Loader Count 0 (golden :5262-5263, :5268)");
        Check(j.str("sgJamCountCsv") == RawCsv(rootA, yday) && ReadAll(RawCsv(rootA, yday), &raw5) && CountLinesWith(raw5, "JAM1901") == 1,
              "yesterday's RawData.csv written");
        const std::vector<std::string> ls = SplitLines(raw5);
        Check(ls.size() == 2 && ls[1].size() >= 4 && ls[1].substr(ls[1].size() - 4) == ",100", "its LoaderCount column is 100 (written before the reset)");
        Check(j.count("skipped") == 0, "[N26] off: no FTP tail");
    }

    std::printf("[6] [N26] FTP tail (S25, gated Q5a)\n");
    {
        IniConfig.bN26_UseJamRawDataUpdataToFTP = true;
        const std::string r = Act("queryYesterday", "{}");
        J j(r);
        Check(AckOk(r) && j.count("skipped") == 1 && Has(j.skipped0(), "Q5a"), "bIsNextDay && [N26]: the UploadFileFTP tail reported as skipped");
        IniConfig.bN26_UseJamRawDataUpdataToFTP = false;
        ReadAll(RawCsv(rootA, yday), &raw5);
    }

    std::printf("[7] yesterday's CSV missing (golden :5106-5111)\n");
    {
        std::remove(EventCsv(rootA, yday).c_str());
        Check(!Exists(EventCsv(rootA, yday)), "(setup) yesterday's CSV really is gone");
        iOneDayLoaderCount = 7;
        const std::string r = Act("queryYesterday", "{}");
        J j(r);
        std::string again;
        Check(AckOk(r) && j.str("early") == "eventlog-missing" && j.isFalse("eventLogCsv.exists") && !j.has("sgJamCountCsv"), "early return reported");
        Check(iOneDayLoaderCount == 7 && j.num("grid.rows") == 3 && j.cell(1, 2) == "JAM1901", "counter and grid untouched");
        Check(ReadAll(RawCsv(rootA, yday), &again) && again == raw5, "yesterday's RawData.csv not deleted (the delete is after the return)");
    }

    std::printf("[8] InitialOK false (golden :5062-5065)\n");
    {
        std::remove(RawCsv(rootA, today).c_str());
        InitialOK = false;
        const std::string r = Act("queryNow", "{}");
        J j(r);
        Check(AckOk(r) && j.str("early") == "initial-not-ok" && j.isFalse("initialOk"), "golden returns at once, reported");
        Check(!Exists(RawCsv(rootA, today)) && j.num("grid.rows") == 3 && j.cell(1, 2) == "JAM1901", "nothing written, grid untouched");
        InitialOK = true;
    }

    std::printf("[9] JamCountEnable.ini 03=0 (golden StatisticalJamCountEnable :5301-5329)\n");
    {
        MakeDirs(rootB + "\\SGJamCount");
        WriteAll(rootB + "\\SGJamCount\\JamCountEnable.ini", "[JamCountEnable]\r\n03=0\r\n");
        SetEventLogRoot(rootB);
        delete slEventLog;
        slEventLog = new TMyStringList(AnsiString(rootB.c_str()), "EventLogTxt", "hdr");
        SeedCsv(EventCsv(rootB, today), kTodayRows);
        iOneDayLoaderCount = 100;
        const std::string r = Act("queryNow", "{}");
        J j(r);
        std::string rawB;
        Check(AckOk(r) && j.str("early") == "" && j.num("grid.rows") == 2, "JAM0301 / JAM0302 not counted: header + golden's blank row only");
        Check(ReadAll(RawCsv(rootB, today), &rawB) && CountLinesWith(rawB, "JAM") == 0 && SplitLines(rawB).size() == 2,
              "RawData.csv: header + golden's empty line with the LoaderCount (:5227-5239)");
        std::string ini;
        Check(ReadAll(rootB + "\\SGJamCount\\JamCountEnable.ini", &ini) && Has(ini, "03=0"), "the seeded 03=0 stays (the other keys are seeded = 1)");
        SetEventLogRoot(rootA);
    }

    std::printf("[10] source ratchets\n");
    {
        std::string chan, html, js;
        const bool r1 = ReadAll(portRoot + "\\JsonBridge\\ChanAction.cpp", &chan);
        const bool r2 = ReadAll(webPage + "\\Data.Observer.html", &html);
        const bool r3 = ReadAll(webPage + "\\ht9045_observer_sgjam.js", &js);
        Check(r1 && r2 && r3, "read ChanAction.cpp, Data.Observer.html, ht9045_observer_sgjam.js (argv " + portRoot + " / " + webPage + ")");
        bool dispatchLive = false;
        const std::vector<std::string> ls = SplitLines(chan);
        for (std::size_t i = 0; i < ls.size(); ++i) {
            const std::size_t a = ls[i].find("cmd.compare(0, 15, \"act.observerSG.\")");
            if (a == std::string::npos) continue;
            const std::size_t call = ls[i].find("return W906_ObserverSGJamAct_St02(cmd, payloadJson);", a);
            const std::size_t cm = ls[i].find("//");
            dispatchLive = call != std::string::npos && (cm == std::string::npos || cm > call);
        }
        Check(dispatchLive, "ChanAction.cpp dispatches act.observerSG.* before that line's // comment");
        const std::string h = StripHtmlComments(html);
        const std::size_t a = h.find("<script src=\"ht9045_observer_ev.js\"></script>");
        const std::size_t b = h.find("<script src=\"ht9045_observer_sgjam.js\"></script>");
        Check(a != std::string::npos && b != std::string::npos && a < b, "Data.Observer.html loads ht9045_observer_sgjam.js after ht9045_observer_ev.js");
        Check(Has(js, "act.observerSG.") && Has(js, "queryNow") && Has(js, "queryYesterday") && Has(js, "'state'") &&
              Has(js, "btnSG_QueryNow") && Has(js, "btnSG_QueryYesterday"), "the page js binds both buttons and the state read");
    }

    // restore
    delete slEventLog;
    slEventLog = savedSl;
    InitialOK = savedInitialOK;
    iOneDayLoaderCount = savedLoader;
    IniConfig.asA32_1_HandlerID = savedHandlerID;
    IniConfig.bN26_UseJamRawDataUpdataToFTP = savedN26;
    f->bShow = savedShow;
    HT9045_TEST_PUTENV(s_origEnv);

    std::printf("[11] real files\n");
    ListTree(realSg, "", &real1);
    char nreal[32];
    std::snprintf(nreal, sizeof(nreal), "%u", (unsigned)real0.size());   // MinGW.org 6.3: no std::to_string
    Check(real0 == real1, std::string("D:\\HT9045_Log\\EventLogTxt\\SGJamCount unchanged (") + nreal + " entries)");

    if (g_fail == 0) Wipe(sand);
    std::printf("RESULT: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
