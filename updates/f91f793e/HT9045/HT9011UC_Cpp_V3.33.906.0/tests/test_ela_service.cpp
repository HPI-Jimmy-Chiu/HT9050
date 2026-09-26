// =============================================================================
//  test_ela_service.cpp -- ELA plan P3 / P4: the /api/ela handling (ela::ServeHttp) behind wb_serve's W906_ElaHttp.
//
//  AI(W906-ELA-P3) 20260927.  Suite name (add_test): ELA_Service
//
//    0. containment first: this process points W906_EVENTLOG_ROOT / W906_PRODLOG_ROOT / W906_GENERAL_INI_PATH and the
//       D5 pair (W906_HT9045LOG_ROOT / W906_SAVEEVENTLOG_ROOT) at its own %TEMP% sandbox, and the QueryRequest /
//       HubPaths defaults follow them (the unset case = the golden literals);
//    1. paths that are not /api/ela fall through (false);
//    2. no hub: 503; HT9045_ELA=0 keeps W906_ElaStart from making one (W906_ElaHttp answers 503);
//    3. GET: the snapshot; ?since=<current seq> -> the short "unchanged" answer; another seq -> the snapshot;
//    4. POST without --allow-cmd: 403; a bad date: 400; unknown sub-path / POST on the root: 404;
//    5. POST /api/ela/query with URL-encoded dates, top / filter / area / row: 202, the hub runs it and the snapshot
//       carries the result (range, handlerId from base, By-Area limited to the one area bit); an empty area = all;
//    6. the real D:\HT9045\config\config.ini, D:\HT9045\Error\English\JAM0000.dat, D:\HT9045\system\Gerneral.ini and
//       D:\HT9045_Log\EventLogTxt are untouched (existence, size, last-write time) -- the JAM0000.dat write-back and
//       ReadConfig all go to the sandbox.
//  The sandbox %TEMP%\ht9045_ela_service_<tick> is removed on a green run (kept for a look when something fails).
// =============================================================================
#include "EventLogAnalysis/ElaService.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

// same guard as tests/test_agv_e84.cpp:143-163 (MinGW.org 6.3 strict mode declares neither putenv nor _putenv)
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static void WriteFile(const std::string& p, const char* text)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs(text, f); std::fclose(f); }
}

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }

// MUST stay a CRT call: getenv() reads the CRT's copy, which SetEnvironmentVariableA does not update.
static void PutEnv(const std::string& nameValue)
{
    static std::string keep[16];
    static int n = 0;
    keep[n % 16] = nameValue;
    HT9045_TEST_PUTENV(keep[n % 16].c_str());
    ++n;
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0)
                continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                RemoveTree(p);
            else
                ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

// existence + size + last-write time of a production file / directory
struct Stamp
{
    bool exists;
    WIN32_FILE_ATTRIBUTE_DATA a;
};
static Stamp StampOf(const char* p)
{
    Stamp s;
    std::memset(&s, 0, sizeof(s));
    s.exists = ::GetFileAttributesExA(p, GetFileExInfoStandard, &s.a) != 0;
    return s;
}
static bool Same(const Stamp& x, const Stamp& y)
{
    if (x.exists != y.exists) return false;
    if (!x.exists) return true;
    return x.a.nFileSizeHigh == y.a.nFileSizeHigh && x.a.nFileSizeLow == y.a.nFileSizeLow &&
           std::memcmp(&x.a.ftLastWriteTime, &y.a.ftLastWriteTime, sizeof(FILETIME)) == 0;
}

struct Resp
{
    bool handled;
    int status;
    std::string ct, body;
};
static Resp Call(ela::Hub* h, const ela::QueryRequest& base, const char* method, const char* path, const char* query,
                 bool allowCmd)
{
    Resp r;
    r.status = 0;
    r.handled = ela::ServeHttp(h, base, method, path, query, allowCmd, &r.status, &r.ct, &r.body);
    return r;
}

int main()
{
    printf("ELA_Service\n");
    static const char* const kProd[] = { "D:\\HT9045\\config\\config.ini", "D:\\HT9045\\Error\\English\\JAM0000.dat",
                                         "D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045_Log\\EventLogTxt" };
    const int kProdN = (int)(sizeof(kProd) / sizeof(kProd[0]));
    Stamp before[4];
    for (int i = 0; i < kProdN; ++i) before[i] = StampOf(kProd[i]);

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string root = std::string(tmp) + "ht9045_ela_service_" + stamp;
    const std::string ev = root + "\\EventLogTxt", prod = root + "\\Production_Log", gen = root + "\\Gerneral.ini";
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA(ev.c_str(), 0);
    ::CreateDirectoryA((ev + "\\2026").c_str(), 0);
    ::CreateDirectoryA((ev + "\\2026\\04").c_str(), 0);
    ::CreateDirectoryA(prod.c_str(), 0);
    WriteFile(ev + "\\2026\\04\\EventLogTxt_20260401.csv",
              "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe\r\n"
              "2026/04/01,08:10:00,\"01 Input Arm\",JAM0101,1,30,0,\"Pick fail\",,R1\r\n"
              "2026/04/01,08:20:00,\"24 Motor\",WAR2401,1,20,0,\"Motor alarm\",,R1\r\n");
    const std::string jamIni = root + "\\JAM0000.dat";
    WriteFile(jamIni, "[01 Input Arm]\r\n");
    const std::string cfg = root + "\\config.ini";
    WriteFile(gen, "[System]\r\nCUSTOMER_CODE=910\r\n[Version]\r\nMachine ID=M1\r\n");

    // 0
    PutEnv("W906_EVENTLOG_ROOT=");
    PutEnv("W906_PRODLOG_ROOT=");
    PutEnv("W906_GENERAL_INI_PATH=");
    {
        ela::QueryRequest g;
        ela::HubPaths gp;
        CHECK(g.eventLogDir == "D:\\HT9045_Log\\EventLogTxt" && g.prodLogDir == "D:\\HT9045_Log\\Production_Log" &&
                  gp.generalIni == "D:\\HT9045\\system\\Gerneral.ini" && gp.configIni == "D:\\HT9045\\config\\config.ini",
              "0. seams unset: the golden literals");
    }
    PutEnv("W906_EVENTLOG_ROOT=" + ev);
    PutEnv("W906_PRODLOG_ROOT=" + prod);
    PutEnv("W906_GENERAL_INI_PATH=" + gen);
    PutEnv("W906_HT9045LOG_ROOT=" + root);
    PutEnv("W906_SAVEEVENTLOG_ROOT=" + root + "\\SaveEventLog");
    ela::QueryRequest base;
    base.handlerId = "H1";
    ela::HubPaths hp;
    hp.configIni = cfg;                  // no seam for config.ini: injected
    CHECK(base.eventLogDir == ev && base.prodLogDir == prod && hp.generalIni == gen,
          "0. QueryRequest / HubPaths defaults follow W906_EVENTLOG_ROOT / W906_PRODLOG_ROOT / W906_GENERAL_INI_PATH");
    ela::Options o;
    o.jamIniPath = jamIni;

    // 1
    Resp r = Call(0, base, "GET", "/api/elax", "", true);
    CHECK(!r.handled, "1. /api/elax falls through");
    r = Call(0, base, "GET", "/page/eventlog.html", "", true);
    CHECK(!r.handled, "1. a static file falls through");

    // 2
    r = Call(0, base, "GET", "/api/ela", "", true);
    CHECK(r.handled && r.status == 503 && Has(r.body, "not running") && r.ct.find("application/json") == 0,
          "2. no hub: 503 JSON");
    PutEnv("HT9045_ELA=0");
    const char* optOut = std::getenv("HT9045_ELA");
    if (optOut && std::strcmp(optOut, "0") == 0)
    {
        // guarded: without the opt-out W906_ElaStart would make the production hub (real config.ini)
        W906_ElaStart();
        int st = 0;
        std::string ct, bd;
        CHECK(W906_ElaHttp("GET", "/api/ela", "", true, &st, &ct, &bd) && st == 503,
              "2. HT9045_ELA=0: W906_ElaStart makes no hub, W906_ElaHttp answers 503");
        CHECK(!W906_ElaHttp("GET", "/index.html", "", true, &st, &ct, &bd), "2. W906_ElaHttp: other paths fall through");
        W906_ElaPost(ela::EL_UPDATE_PARAMETER);        // no hub: no-op
        W906_ElaStop();
    }
    else
        CHECK(false, "2. HT9045_ELA=0 did not reach getenv -- W906_ElaStart not called");

    {
        ela::Hub h(o, hp);
        // 3
        r = Call(&h, base, "GET", "/api/ela", "", false);
        CHECK(r.handled && r.status == 200 && Has(r.body, "{\"seq\":0,") && Has(r.body, "\"result\":null"),
              "3. GET: the snapshot (read-only server too)");
        r = Call(&h, base, "GET", "/api/ela/", "", true);
        CHECK(r.status == 200 && Has(r.body, "\"history\":"), "3. GET /api/ela/ is the same");
        r = Call(&h, base, "GET", "/api/ela", "since=0", true);
        CHECK(r.status == 200 && r.body == "{\"seq\":0,\"busy\":false,\"unchanged\":true}",
              "3. since=<current seq>: the short answer");
        r = Call(&h, base, "HEAD", "/api/ela", "since=0", true);
        CHECK(r.status == 200 && Has(r.body, "\"unchanged\":true"), "3. HEAD answers like GET (wb_serve drops the body)");
        r = Call(&h, base, "GET", "/api/ela", "since=7", true);
        CHECK(r.status == 200 && Has(r.body, "\"history\":") && !Has(r.body, "unchanged"), "3. another seq: the full snapshot");

        // 4
        r = Call(&h, base, "POST", "/api/ela/query", "from=2026/04/01 00:00:00&to=2026/04/01 23:59:59", false);
        CHECK(r.status == 403, "4. POST without --allow-cmd: 403");
        r = Call(&h, base, "POST", "/api/ela/query", "from=abc&to=2026/04/01 23:59:59", true);
        CHECK(r.status == 400, "4. a bad from: 400");
        r = Call(&h, base, "POST", "/api/ela/query", "from=2026/04/01 00:00:00", true);
        CHECK(r.status == 400, "4. a missing to: 400");
        r = Call(&h, base, "GET", "/api/ela/nope", "", true);
        CHECK(r.handled && r.status == 404, "4. unknown sub-path: 404");
        r = Call(&h, base, "POST", "/api/ela", "", true);
        CHECK(r.status == 404, "4. POST on the root: 404");
        CHECK(!h.RunOnce() && h.Seq() == 0, "4. none of those queued anything");

        // 5
        r = Call(&h, base, "POST", "/api/ela/query",
                 "from=2026%2F04%2F01+00%3A00%3A00&to=2026/04/01%2023:59:59&top=2&filter=0&area=1&func=&row=1", true);
        CHECK(r.status == 202 && Has(r.body, "\"queued\":true"), "5. POST /api/ela/query: 202 queued");
        CHECK(h.RunOnce() && h.Seq() == 1, "5. the hub runs the queued query");
        r = Call(&h, base, "GET", "/api/ela", "since=0", true);
        CHECK(Has(r.body, "\"range\":[\"2026/04/01 00:00:00\",\"2026/04/01 23:59:59\"]"),
              "5. URL-decoded from / to reach SetRange");
        CHECK(Has(r.body, "Handler ID:        H1") && Has(r.body, "JAM0101"), "5. handlerId from base; the event rows");
        CHECK(Has(r.body, "\"byAreaCount\":1"), "5. area=1: By Area holds only 01 Input Arm");
        r = Call(&h, base, "GET", "/api/ela", "since=1", true);
        CHECK(Has(r.body, "\"unchanged\":true"), "5. since=<new seq>: unchanged again");
    }
    {
        ela::Hub h2(o, hp);                  // a fresh hub: #18 A keeps ListByUnitName across queries on one hub
        r = Call(&h2, base, "POST", "/api/ela/query", "from=2026/04/01 00:00:00&to=2026/04/01 23:59:59", true);
        CHECK(r.status == 202 && h2.RunOnce(), "5. a query with no area / func / top / row");
        r = Call(&h2, base, "GET", "/api/ela", "", true);
        CHECK(Has(r.body, "\"byAreaCount\":2"), "5. empty area = every area");
    }

    // 6
    for (int i = 0; i < kProdN; ++i)
    {
        char msg[160];
        std::snprintf(msg, sizeof(msg), "6. %s untouched", kProd[i]);
        CHECK(Same(before[i], StampOf(kProd[i])), msg);
    }

    printf("ELA_Service: %d passed, %d failed\n", g_pass, g_fail);
    if (g_fail == 0)
        RemoveTree(root);
    else
        printf("  sandbox kept: %s\n", root.c_str());
    return g_fail ? 1 : 0;
}
