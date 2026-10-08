// =============================================================================
//  test_st02_w184_sitelog.cpp -- W-184 (POOL-6 LINK-22): handlerlog.cpp is linked -- `TMyLog myLog;` is defined (golden 913
//  cmydef.cpp:3478; here at handlerlog.cpp's end), its site-status ChangeLog (golden 913 TMyLog::Save_SiteStatusLog, handlerlog.cpp
//  :99-270) follows the log-root seam, and the recipe change calls it (golden 913 main.cpp:25925, TfMain::ChangeSetUpFile).
//
//  AI(W906-W184) 20261008 (St02-E).  Suite name (add_test): St02_W184SiteStatusLog.  argv[1] = port root (source pins).
//  Every write goes to a per-run folder under ctest's log root (W906_HT9045LOG_ROOT) or %TEMP%: as9045LogPath is pointed there before
//  the first call (myLog.sFolder takes it once, handlerlog.cpp:290); a root under D:\HT9045 or D:\HT9045_Log stops the test before
//  anything is called.  Removed when green.
//    [1] W906_SaveSiteStatusLog writes <root>\ChangeLog\yyyy_mm\log_yyyy_mm_dd.ini with golden's two fSiteStatusArm lines built from
//        LastSet.bUseTestSocket[0/1] over TestSocket.iShtRow x iShtCol (1x2 here: "1,0" / "0,1").
//    [2] golden's month clean-up: the ChangeLog\<two months back> folder is deleted, the previous month's stays.
//    [3] source: WebRecipeChange.cpp calls W906_SaveSiteStatusLog() (no Gap("main.cpp:25781") left); handlerlog.cpp defines
//        `TMyLog myLog;`; cmydef.cpp still keeps its copy in #if 0 (one definition).
//    [4] (MR B) the Setup save calls it too: the generated FileRW/TestIF_File_SetUp.gen.inc (St01 tools/editlist/TestIF_File_SetUp.py
//        :394 'replace') has the live call where golden 913 cSetUp.cpp:4129 TfSetup::SaveSetupFile has myLog.Save_SiteStatusLog(),
//        and no ELTodo for it any more.
// =============================================================================
#include "aHotPlateSubstrate.h"
#include "LastSet.h"
#include "cprod.h"
#include "cmydef.h"
#include "common.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>

void W906_SaveSiteStatusLog();   // handlerlog.cpp:757

static int g_pass = 0, g_fail = 0;
static void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}
static std::string Lower(std::string s)
{
    for (size_t i = 0; i < s.size(); ++i) { if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); if (s[i] == '/') s[i] = '\\'; }
    return s;
}
static bool Forbidden(const std::string& p)
{
    const std::string s = Lower(p);
    if (s.find("\\obj\\v906\\") != std::string::npos) return false;
    return s.compare(0, 9, "d:\\ht9045") == 0;   // D:\HT9045 and D:\HT9045_Log
}
static std::string Slurp(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
static bool DirExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}
static std::string Ym(int y, int m) { char b[16]; std::snprintf(b, sizeof(b), "%04d_%02d", y, m); return b; }

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("St02_W184SiteStatusLog -- handlerlog.cpp linked: myLog defined, site-status ChangeLog, recipe-change caller\n");
    const std::string src = argc > 1 ? argv[1] : std::string();
    std::string base;
    if (const char* e = std::getenv("W906_HT9045LOG_ROOT")) base = e;
    if (base.empty()) { char tmp[MAX_PATH]; ::GetTempPathA(sizeof(tmp), tmp); base = tmp; }
    while (!base.empty() && (base[base.size() - 1] == '\\' || base[base.size() - 1] == '/')) base.erase(base.size() - 1);
    const std::string root = base + "\\w184_" + std::to_string((unsigned long)::GetTickCount());
    if (Forbidden(root)) {
        std::printf("  ABORT: %s is not a sandbox -- nothing was called\n", root.c_str());
        return 2;
    }
    ::CreateDirectoryA(root.c_str(), 0);
    as9045LogPath = AnsiString(root.c_str());

    std::time_t now = std::time(0);
    std::tm lt = *std::localtime(&now);
    const int y = lt.tm_year + 1900, m = lt.tm_mon + 1, d = lt.tm_mday;
    int y2 = y, m2 = m - 2;
    if (m2 < 1) { m2 += 12; --y2; }
    int y1 = y, m1 = m - 1;
    if (m1 < 1) { m1 += 12; --y1; }
    const std::string cl = root + "\\ChangeLog";
    ::CreateDirectoryA(cl.c_str(), 0);
    ::CreateDirectoryA((cl + "\\" + Ym(y2, m2)).c_str(), 0);
    ::CreateDirectoryA((cl + "\\" + Ym(y1, m1)).c_str(), 0);

    // a 1x2 site grid: arm 1 = on,off; arm 2 = off,on; mapping unchanged (no MES2105 event)
    TestSocket.iShtRow = 1;
    TestSocket.iShtCol = 2;
    LastSet.bUseTestSocket[0][0][0] = true;  LastSet.bUseTestSocket[0][0][1] = false;
    LastSet.bUseTestSocket[1][0][0] = false; LastSet.bUseTestSocket[1][0][1] = true;
    for (int j = 0; j < 2; ++j) iRecordSiteMapOrder[0][j] = TestIF_File.iSiteMap[0][j];

    // ---------------------------------------------------------------- [1]
    std::printf("[1] W906_SaveSiteStatusLog\n");
    W906_SaveSiteStatusLog();
    char name[64];
    std::snprintf(name, sizeof(name), "\\log_%04d_%02d_%02d.ini", y, m, d);
    const std::string log = cl + "\\" + Ym(y, m) + name;
    const std::string body = Slurp(log);
    Check(!body.empty(), "[1] " + log + " written (" + std::to_string(body.size()) + " bytes)");
    Check(body.find("fSiteStatusArm1\t\t\t1,0") != std::string::npos && body.find("fSiteStatusArm2\t\t\t0,1") != std::string::npos,
          "[1] golden's fSiteStatusArm1 / fSiteStatusArm2 lines: \"1,0\" / \"0,1\"");

    // ---------------------------------------------------------------- [2]
    std::printf("[2] month clean-up\n");
    Check(!DirExists(cl + "\\" + Ym(y2, m2)), "[2] ChangeLog\\" + Ym(y2, m2) + " (two months back) deleted, as golden");
    Check(DirExists(cl + "\\" + Ym(y1, m1)), "[2] ChangeLog\\" + Ym(y1, m1) + " (last month) kept");

    // ---------------------------------------------------------------- [3]
    std::printf("[3] source pins\n");
    {
        const std::string rc = Slurp(src + "/WebRecipeChange.cpp");
        const size_t at = rc.find("{ extern void W906_SaveSiteStatusLog(); W906_SaveSiteStatusLog(); }");
        const size_t bol = at == std::string::npos ? 0 : rc.rfind('\n', at);
        Check(at != std::string::npos && rc.substr(bol + 1, at - bol - 1).find("//") == std::string::npos &&
                  rc.find("Gap(\"main.cpp:25781\"") == std::string::npos,
              "[3] WebRecipeChange.cpp calls W906_SaveSiteStatusLog() (golden 913 main.cpp:25925), no Gap left");
        const std::string hl = Slurp(src + "/handlerlog.cpp");
        Check(hl.find("\nTMyLog myLog;") != std::string::npos, "[3] handlerlog.cpp defines `TMyLog myLog;`");
        const std::string cm = Slurp(src + "/cmydef.cpp");
        const size_t def = cm.find("\nTMyLog myLog;");
        const size_t prev = def == std::string::npos ? std::string::npos : cm.rfind("\n#if 0", def);
        Check(def != std::string::npos && prev != std::string::npos && def - prev < 12,
              "[3] cmydef.cpp's copy stays inside #if 0 (one definition)");
    }

    // ---------------------------------------------------------------- [4]
    std::printf("[4] Setup save (generated TestIF_File_SetUp.gen.inc)\n");
    {
        const std::string gi = Slurp(src + "/FileRW/TestIF_File_SetUp.gen.inc");
        const std::string call = "    { extern void W906_SaveSiteStatusLog(); W906_SaveSiteStatusLog(); }";
        const size_t at = gi.find("\n" + call);
        const size_t next = at == std::string::npos ? std::string::npos : gi.find('\n', at + 1);
        const std::string gate = "#if 0 // GATE (S12-C save) golden cSetUp.cpp:4129-4129";
        const bool gateAfter = next != std::string::npos && gi.compare(next + 1, gate.size(), gate) == 0;
        Check(at != std::string::npos && gateAfter && gi.find("ELTodo(\"golden cSetUp.cpp:4129") == std::string::npos,
              "[4] TestIF_File_SetUp.gen.inc: live W906_SaveSiteStatusLog() at golden cSetUp.cpp:4129 (golden text kept under #if 0), no ELTodo");
    }

    if (g_fail == 0) {
        ::DeleteFileA(log.c_str());
        ::RemoveDirectoryA((cl + "\\" + Ym(y, m)).c_str());
        ::RemoveDirectoryA((cl + "\\" + Ym(y1, m1)).c_str());
        ::RemoveDirectoryA(cl.c_str());
        ::RemoveDirectoryA(root.c_str());
    }
    std::printf("St02_W184SiteStatusLog: %d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
