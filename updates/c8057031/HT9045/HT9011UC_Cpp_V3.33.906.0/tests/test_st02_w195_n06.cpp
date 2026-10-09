// ===========================================================================
//  tests/test_st02_w195_n06.cpp -- W-195 (3) KYEC K1: the N06 recipe sync to the OS Tester
//  (golden 913 Interface/TesterTCP.cpp:1057-1191, V906 Interface/TesterTCP_N06_St02.cpp) and Steven 1009's fail-and-stop
//  (a failed sync -> bilingual alarm, START refused until a successful re-change).
//  AI(W906-W195) 20261009 (St02-E).
//
//  Real code only (the machine archives, RESCAN group).  NEVER touches D:\HT9045, D:\HT9045_Log or D:\rms: every path the
//  exercised code uses (DataPath, asN06_TesterPath, as9045LogPath, asSaveEventLogPath, W906_7Z_EXE) points into
//  %TEMP%\ht9045_w195n06_<tick> BEFORE the first call, and the test stops if one of them is under D:\HT9045 or D:\rms.
//  7z is never started: parts 1-6 use the W906_N06ExecHook seam; part 7 runs the real CreateProcess path against THIS exe as a
//  fake 7z (W906_N06_FAKE7Z=1 in the child's environment: it prints 7z-like lines, writes its command line into the sandbox and
//  exits with W906_N06_FAKE_EXIT after W906_N06_FAKE_SLEEP ms).
//
//    1. flag [N06] off: TesterTCP_CopyRecipeToTester / FromTester return false before any side effect (golden :1112-1113);
//       W906_N06SyncRecipeOrBlock is true whatever the state (the tester commands go out as today) and never blocks START.
//    2. every failure reason of To / From in golden order (SRC_ZIP_MISSING, DEST_PATH_MISSING, 7Z_MISSING,
//       EXEC_FAIL_OR_TIMEOUT, 7Z_EXIT_n, SRC_INI_MISSING) -> one N06000002 line each in the EventTracker csv.
//    3. success -> N06000001, and golden's 7z parameters (e "<zip>" -o"<dest>\" -y / a -tzip "<zip>" "<ini>").
//    4. the dedup (golden :1061-1064): the same FileName|result twice -> one line.
//    5. W906_N06SyncRecipeOrBlock / W906_N06RecipeSyncBlocked / W906_N06CheckStartAllowed (Steven 1009 answers 3, 4) and the
//       alarm text, word for word (answer 6 = A), with the plain reason of every failure code.
//    6. the call sites (source pins, argv[1] = the tree root, read only): WebRecipeChange.cpp ChangeSetUpFile,
//       HandlerBridgeCtl.cpp SyncBridgeSettings (skips WORKFILE / GETOSSETUP / SET2DID on failure) and WebStart.cpp
//       StartFromWeb (the START refusal between the MES lot check and LatchCycleTime).
//    7. the real CreateProcess path: " -bsp1 -bb1" appended, 7z's lines in the day file <as9045LogPath>\N06\N06_7z_<date>.log
//       (answer 1 = B), the exit-code rule, 7z's last error line in the alarm, only the pipe inherited (an inheritable probe
//       event must NOT reach the child), and the 10 s timeout: 7z terminated, the day file names the last file (answer 2 = B).
// ===========================================================================
#include "Interface/TesterTCP.h"
#include "Interface/TesterTCP_N06_St02.h"
#include "canary_support.h"
#include "cmydef.h"
#include "common.h"
#include "cpublic.h"
#include "cprod.h"
#include "Config.h"
#include "forms/fTemp_Set.h"   // the W-195 rule: create fTemp_Set first (see main)

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

// tests/test_agv_e84.cpp:159-164: MinGW.org strict mode does not declare _putenv (cMyDB.h is not included: with
// canary_support.h it re-declares RecordProcess default arguments)
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

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

// ---- the fake 7z (child mode) ----------------------------------------------
static int FakeSevenZip()
{
    const char* ex = std::getenv("W906_N06_FAKE_EXIT");
    const char* sl = std::getenv("W906_N06_FAKE_SLEEP");
    const char* lg = std::getenv("W906_N06_FAKE_CMDLOG");
    const char* pr = std::getenv("W906_N06_PROBE_HANDLE");
    const char* pn = std::getenv("W906_N06_PROBE_NAME");   // the probe event's name (W-211: compare the object, not the value)
    const int code = ex ? std::atoi(ex) : 0;
    if (lg && lg[0])
    {
        FILE* f = std::fopen(lg, "wb");
        if (f) { std::fputs(::GetCommandLineA(), f); std::fclose(f); }
    }
    if (lg && lg[0] && pr && pr[0])                     // did the parent's inheritable (signalled) probe event reach us?
    {
        const HANDLE h = (HANDLE)(UINT_PTR)std::strtoul(pr, 0, 10);
        DWORD fl = 0;
        bool got = false;
        if (::GetHandleInformation(h, &fl) != 0)        // a valid handle with that value -- ours by inheritance, or one we opened ourselves
        {
            // AI(W906-W195) 20261009 (St02-E) W-211 (laptop, batch 149 SHIP flake "probe: [INHERITED]"): the same VALUE can be a handle
            //   this process opened itself; ask whether it is the parent's named event OBJECT.  The name is opened only after the
            //   value is known valid, so the new handle cannot take that value.  CompareObjectHandles = kernelbase.dll, Windows 10
            //   1607+; without it the old value check stays.
            typedef BOOL (WINAPI* CompareObjectHandlesFn)(HANDLE, HANDLE);
            const HMODULE kb = ::GetModuleHandleA("kernelbase.dll");
            const CompareObjectHandlesFn cmp = kb ? (CompareObjectHandlesFn)(void*)::GetProcAddress(kb, "CompareObjectHandles") : 0;
            const HANDLE hn = (pn && pn[0]) ? ::OpenEventA(SYNCHRONIZE, FALSE, pn) : 0;
            if (cmp && hn) got = cmp(h, hn) != FALSE;
            else           got = ::WaitForSingleObject(h, 0) == WAIT_OBJECT_0;
            if (hn) ::CloseHandle(hn);
        }
        FILE* f = std::fopen((std::string(lg) + ".probe").c_str(), "wb");
        if (f) { std::fputs(got ? "INHERITED" : "NOT INHERITED", f); std::fclose(f); }
    }
    std::printf("\r\n7-Zip (fake for St02_W195N06)\r\n\r\n");
    std::printf("  0%%\r 50%%\r100%%\r");               // the -bsp1 progress: lines ended by \r only
    std::printf("- OS_Setting.ini\n");
    if (code != 0) std::printf("ERROR: fake 7z exit %d\n", code);
    else           std::printf("Everything is Ok\n");
    std::fflush(stdout);
    if (sl && std::atoi(sl) > 0)
    {
        ::Sleep((DWORD)std::atoi(sl));
        if (lg && lg[0])                                 // still alive after the sleep: the parent did NOT terminate us
        {
            FILE* f = std::fopen((std::string(lg) + ".alive").c_str(), "wb");
            if (f) { std::fputs("alive", f); std::fclose(f); }
        }
    }
    return code;
}

// ---- the exec hook (parts 1-6) ---------------------------------------------
static int           g_hookCalls = 0;
static std::string   g_hook7z, g_hookParam;
static bool          g_hookRet = true;
static unsigned long g_hookExit = 0;
static bool FakeExec(const char* s7z, const char* sParam, unsigned long* pdwExit)
{
    ++g_hookCalls;
    g_hook7z = s7z ? s7z : "";
    g_hookParam = sParam ? sParam : "";
    if (pdwExit) *pdwExit = g_hookRet ? g_hookExit : 0xFFFFFFFFUL;
    return g_hookRet;
}

// ---- the message capture ---------------------------------------------------
static int         g_msgs = 0;
static std::string g_msgS1, g_msgS2;
static void MsgHook(const char* s1, const char* s2)
{
    ++g_msgs;
    g_msgS1 = s1 ? s1 : "";
    g_msgS2 = s2 ? s2 : "";
}

// ---- files -------------------------------------------------------------------
static std::string ReadAll(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::ostringstream o;
    o << f.rdbuf();
    return o.str();
}

static int CountIn(const std::string& s, const char* what)
{
    int n = 0;
    for (size_t at = s.find(what); at != std::string::npos; at = s.find(what, at + 1)) ++n;
    return n;
}

// sum of `what` in every file under `dir` whose name contains `nameHas`
static int CountInTree(const std::string& dir, const char* nameHas, const char* what)
{
    int n = 0;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    do
    {
        const std::string nm = fd.cFileName;
        if (nm == "." || nm == "..") continue;
        const std::string p = dir + "\\" + nm;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) n += CountInTree(p, nameHas, what);
        else if (nm.find(nameHas) != std::string::npos)     n += CountIn(ReadAll(p), what);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return n;
}

static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}

static void Touch(const std::string& p)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) { std::fputs("x", f); std::fclose(f); }
}

static bool Forbidden(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}

static void PutEnv(const std::string& kv)
{
    static char bufs[8][1024];
    static int next = 0;
    char* b = bufs[next++ % 8];
    std::snprintf(b, sizeof(bufs[0]), "%s", kv.c_str());
    HT9045_TEST_PUTENV(b);                      // the parent's getenv (Get7zPath)
    const size_t eq = kv.find('=');
    ::SetEnvironmentVariableA(kv.substr(0, eq).c_str(), kv.substr(eq + 1).c_str());   // the child's environment
}

static int Ng() { return CountInTree(std::string(as9045LogPath.c_str()), "EventTracker", "N06000002"); }
static int Ok() { return CountInTree(std::string(as9045LogPath.c_str()), "EventTracker", "N06000001"); }
static bool Has(const char* what) { return CountInTree(std::string(as9045LogPath.c_str()), "EventTracker", what) > 0; }

// ---- source pins (part 6) ----------------------------------------------------
static std::string Src(const std::string& root, const char* rel)
{
    return ReadAll(root + "/" + rel);
}

static size_t LineAt(const std::string& s, size_t pos) { return pos == std::string::npos ? 0 : (size_t)CountIn(s.substr(0, pos), "\n") + 1; }

int main(int argc, char** argv)
{
    if (std::getenv("W906_N06_FAKE7Z") && std::string(std::getenv("W906_N06_FAKE7Z")) == "1")
        return FakeSevenZip();

    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string tmpRoot = std::string(tmp) + "ht9045_w195n06_";
    const std::string root = tmpRoot + stamp;
    const std::string data = root + "\\Data\\";
    const std::string dest = root + "\\OSTester";
    const std::string bin  = root + "\\bin";
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((data + "RCP1").c_str(), 0);
    ::CreateDirectoryA(dest.c_str(), 0);
    ::CreateDirectoryA(bin.c_str(), 0);
    const std::string zip = data + "RCP1\\OS_Setting.zip";
    const std::string fake7z = bin + "\\7z.exe";        // an existing file, never run (the hook runs instead)
    Touch(zip);
    Touch(fake7z);

    // --- redirect every path BEFORE the first call (see banner) ---
    DataPath = AnsiString(data.c_str());
    as9045LogPath = AnsiString(root.c_str());
    asSaveEventLogPath = AnsiString((root + "\\SaveEventLog").c_str());
    IniConfig.asN06_TesterPath = AnsiString(dest.c_str());
    IniConfig.bO06SaveLogTimePeriod = false;
    IniConfig.bSPILFunction = false;
    PutEnv("W906_7Z_EXE=" + fake7z);
    GetTimeInfo();
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136): golden boot creates TfTemp_Set
    W906_ShowMyMessage_Hook = MsgHook;

    char exe[MAX_PATH] = { 0 };
    ::GetModuleFileNameA(NULL, exe, MAX_PATH);
    std::string exeDir = exe;
    exeDir = exeDir.substr(0, exeDir.find_last_of("\\/") + 1);
    const bool b7zNextToExe = ::GetFileAttributesA((exeDir + "7z.exe").c_str()) != INVALID_FILE_ATTRIBUTES;
    if (b7zNextToExe)
        std::printf("STOP: a 7z.exe sits next to this test exe (%s7z.exe); Get7zPath would pick it before W906_7Z_EXE\n", exeDir.c_str());
    CHECK(b7zNextToExe == false);

    const char* paths[] = { DataPath.c_str(), as9045LogPath.c_str(), asSaveEventLogPath.c_str(),
                            IniConfig.asN06_TesterPath.c_str(), std::getenv("W906_7Z_EXE") };
    bool bad = b7zNextToExe;
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
        if (paths[i] == 0 || Forbidden(paths[i]) || std::string(paths[i]).compare(0, root.size(), root) != 0)
        {
            std::printf("STOP: path %s is not under the sandbox %s\n", paths[i] ? paths[i] : "(null)", root.c_str());
            bad = true;
        }
    if (bad)
    {
        std::printf("test_st02_w195_n06: stopped before any call\n");
        return 1;
    }

    W906_N06ExecHook = FakeExec;

    // ---- 1. flag off --------------------------------------------------------
    IniConfig.bN06_CopyTesterFile = false;
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == false);
    CHECK(TesterTCP_CopyRecipeFromTester("RCP1") == false);
    CHECK(g_hookCalls == 0);
    CHECK(Ng() == 0 && Ok() == 0);
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);   // Steven 1009 answer 3: off -> go on, nothing shown
    CHECK(W906_N06RecipeSyncBlocked() == false);
    CHECK(W906_N06CheckStartAllowed() == true);
    CHECK(g_msgs == 0);
    // St02-M 1009 13:3x: with the flag OFF nothing changes from today -- the sync is true whatever the state (a missing zip, a
    // failing 7z), so SyncBridgeSettings still sends WORKFILE / GETOSSETUP / SET2DID (part 6 pins them inside the true
    // branch), and the START block flag is never set.
    g_hookRet = false;
    CHECK(W906_N06SyncRecipeOrBlock("NOZIP") == true);
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);
    g_hookRet = true;
    CHECK(g_hookCalls == 0 && Ng() == 0 && Ok() == 0 && g_msgs == 0);
    CHECK(::GetFileAttributesA((root + "\\N06").c_str()) == INVALID_FILE_ATTRIBUTES);   // no 7z run, no day file
    IniConfig.bN06_CopyTesterFile = true;                  // the stored flag, read with the switch on: still not set
    CHECK(W906_N06RecipeSyncBlocked() == false);
    IniConfig.bN06_CopyTesterFile = false;

    // ---- 2 + 4. every failure reason, the dedup ------------------------------
    IniConfig.bN06_CopyTesterFile = true;
    CHECK(TesterTCP_CopyRecipeToTester("NOZIP") == false);
    CHECK(Ng() == 1 && Has("SRC_ZIP_MISSING"));
    CHECK(TesterTCP_CopyRecipeToTester("NOZIP") == false);
    CHECK(Ng() == 1);                                      // golden :1063: the same FileName|reason -> logged once

    IniConfig.asN06_TesterPath = AnsiString((root + "\\NoSuchShare").c_str());
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == false);
    CHECK(Ng() == 2 && Has("DEST_PATH_MISSING"));
    IniConfig.asN06_TesterPath = AnsiString(dest.c_str());

    PutEnv("W906_7Z_EXE=" + bin + "\\none.exe");
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == false);
    CHECK(Ng() == 3 && Has("7Z_MISSING"));
    PutEnv("W906_7Z_EXE=" + fake7z);
    CHECK(g_hookCalls == 0);                               // nothing ran so far

    g_hookRet = false;
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == false);
    CHECK(Ng() == 4 && Has("EXEC_FAIL_OR_TIMEOUT"));
    CHECK(g_hookCalls == 1);

    g_hookRet = true; g_hookExit = 2;
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == false);
    CHECK(Ng() == 5 && Has("7Z_EXIT_2"));

    // ---- 3. success and golden's parameters --------------------------------
    g_hookExit = 0;
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == true);
    CHECK(Ok() == 1);
    CHECK(g_hook7z == fake7z);
    const std::string wantTo = "e \"" + zip + "\" -o\"" + dest + "\\\" -y";   // golden 913 :1136
    CHECK(g_hookParam == wantTo);
    if (g_hookParam != wantTo) std::printf("  To param: [%s]\n  want:     [%s]\n", g_hookParam.c_str(), wantTo.c_str());
    CHECK(TesterTCP_CopyRecipeToTester("RCP1") == true);
    CHECK(Ok() == 1);                                      // dedup of the OK line too

    CHECK(TesterTCP_CopyRecipeFromTester("RCP1") == false);
    CHECK(Ng() == 6 && Has("SRC_INI_MISSING"));
    const std::string ini = dest + "\\RCP1.ini";
    Touch(ini);
    CHECK(TesterTCP_CopyRecipeFromTester("RCP1") == true);
    CHECK(Ok() == 2);
    const std::string wantFrom = "a -tzip \"" + zip + "\" \"" + ini + "\"";   // golden 913 :1176
    CHECK(g_hookParam == wantFrom);
    if (g_hookParam != wantFrom) std::printf("  From param: [%s]\n  want:       [%s]\n", g_hookParam.c_str(), wantFrom.c_str());

    // ---- 5. fail-and-stop (Steven 1009 answers 3 / 4) --------------------------
    g_hookExit = 3;
    int m0 = g_msgs;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == false);
    CHECK(g_msgs == m0 + 1);
    CHECK(g_msgS1 == "N06 recipe sync to OS Tester FAILED: 7Z_EXIT_3 -- RCP1. Fix it and change the recipe again. START is blocked until then.");
    CHECK(g_msgS2 == "OS 測試機工作檔同步失敗：7z 回報錯誤（代碼 3）（7Z_EXIT_3，RCP1）。請排除後重新換工作檔；完成前不能按 START。");
    std::printf("  alarm S1: %s\n  alarm S2: %s\n", g_msgS1.c_str(), g_msgS2.c_str());
    CHECK(W906_N06RecipeSyncBlocked() == true);
    CHECK(W906_N06CheckStartAllowed() == false);
    CHECK(g_msgs == m0 + 2);
    CHECK(g_msgS1 == "START refused: the last N06 recipe sync to OS Tester FAILED: 7Z_EXIT_3 -- RCP1. Fix it and change the recipe again.");
    CHECK(g_msgS2.find("7z 回報錯誤（代碼 3）（7Z_EXIT_3，RCP1）") != std::string::npos);
    g_hookExit = 0;                                        // AI(W906-W195) K1 fix: a later N06 call with another result (an OK upload)
    CHECK(TesterTCP_CopyRecipeFromTester("RCP1") == true); //   must not change the reason the START refusal names
    CHECK(W906_N06CheckStartAllowed() == false && g_msgS1.find("FAILED: 7Z_EXIT_3 -- RCP1") != std::string::npos &&
          g_msgS2.find("（7Z_EXIT_3，RCP1）") != std::string::npos);
    m0 = g_msgs - 2;                                       // one more START refusal than before: keep the "m0 + 2" counts below true
    g_hookExit = 3;
    IniConfig.bN06_CopyTesterFile = false;                 // the flag is read at START time: off -> no block, no message
    CHECK(W906_N06RecipeSyncBlocked() == false);
    CHECK(W906_N06CheckStartAllowed() == true);
    CHECK(g_msgs == m0 + 2);
    IniConfig.bN06_CopyTesterFile = true;                  // on again: still blocked (no successful re-change yet)
    CHECK(W906_N06RecipeSyncBlocked() == true);
    g_hookExit = 0;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);      // the successful re-change clears it
    CHECK(W906_N06RecipeSyncBlocked() == false);
    CHECK(W906_N06CheckStartAllowed() == true);
    CHECK(g_msgs == m0 + 2);
    g_hookExit = 3;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == false);
    CHECK(W906_N06RecipeSyncBlocked() == true);
    IniConfig.bN06_CopyTesterFile = false;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);      // a change with the flag off also clears it (golden: nothing to sync)
    IniConfig.bN06_CopyTesterFile = true;
    CHECK(W906_N06RecipeSyncBlocked() == false);

    // the plain reason of every failure code (answer 6 = A)
    struct { const char* file; int mode; const char* code; const char* plain; } rs[] = {
        { "NOZIP", 0, "SRC_ZIP_MISSING",      "找不到要複製的 OS_Setting.zip" },
        { "RCP1",  1, "DEST_PATH_MISSING",    "測試機共用資料夾不存在或沒連上" },
        { "RCP1",  2, "7Z_MISSING",           "找不到 7z.exe" },
        { "RCP1",  3, "EXEC_FAIL_OR_TIMEOUT", "7z 無法執行或超過 10 秒（已強制結束）" },
        { "RCP1",  4, "7Z_EXIT_9",            "7z 回報錯誤（代碼 9）" },
    };
    for (size_t i = 0; i < sizeof(rs) / sizeof(rs[0]); ++i)
    {
        if (rs[i].mode == 1) IniConfig.asN06_TesterPath = AnsiString((root + "\\NoSuchShare").c_str());
        if (rs[i].mode == 2) PutEnv("W906_7Z_EXE=" + bin + "\\none.exe");
        g_hookRet = rs[i].mode != 3;
        g_hookExit = rs[i].mode == 4 ? 9 : 0;
        CHECK(W906_N06SyncRecipeOrBlock(rs[i].file) == false);
        const std::string want = std::string("OS 測試機工作檔同步失敗：") + rs[i].plain + "（" + rs[i].code + "，" + rs[i].file +
                                 "）。請排除後重新換工作檔；完成前不能按 START。";
        CHECK(g_msgS2 == want);
        if (g_msgS2 != want) std::printf("  S2:   [%s]\n  want: [%s]\n", g_msgS2.c_str(), want.c_str());
        IniConfig.asN06_TesterPath = AnsiString(dest.c_str());
        PutEnv("W906_7Z_EXE=" + fake7z);
    }
    g_hookRet = true;
    g_hookExit = 0;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);
    CHECK(W906_N06RecipeSyncBlocked() == false);

    // ---- 6. the call sites (source pins) -----------------------------------------
    if (argc > 1)
    {
        const std::string tree = argv[1];
        const std::string wrc = Src(tree, "WebRecipeChange.cpp");
        const size_t a = wrc.find("    W906_N06SyncRecipeOrBlock(FileName);");
        CHECK(a != std::string::npos && wrc.find(":25811", a) == wrc.find(":25811", a) && wrc.find(":25811", a) < wrc.find("\n", a));
        CHECK(wrc.find("\n    TesterTCP_CopyRecipeToTester(FileName);") == std::string::npos);

        const std::string hbc = Src(tree, "TesterComm/Handler/HandlerBridgeCtl.cpp");
        const size_t b = hbc.find("if(W906_N06SyncRecipeOrBlock(sOSRecipe)) {");
        const size_t bWork = hbc.find("\"WORKFILE,%s,\"", b == std::string::npos ? 0 : b);
        const size_t bSet2 = hbc.find("\"SET2DID,0\"", b == std::string::npos ? 0 : b);
        const size_t bEnd = hbc.find("MySleep(100); }", b == std::string::npos ? 0 : b);
        CHECK(b != std::string::npos && bWork != std::string::npos && bSet2 != std::string::npos && bEnd != std::string::npos);
        CHECK(b < bWork && bWork < bSet2 && bSet2 < bEnd && LineAt(hbc, bEnd) - LineAt(hbc, b) == 11);
        CHECK(hbc.find("TesterTCP_CopyRecipeToTester(sOSRecipe);") == std::string::npos);

        const std::string ws = Src(tree, "WebStart.cpp");
        const size_t fn = ws.find("bool TfMainWeb::StartFromWeb(");
        const size_t mes = ws.find("if (fMesSystem->CheckLotInfor() == false)", fn == std::string::npos ? 0 : fn);
        const size_t gate = ws.find("if (W906_N06CheckStartAllowed() == false) { iStartIn = 0; return false; }", fn == std::string::npos ? 0 : fn);
        const size_t latch = ws.find("lHandlerStopTime.LatchCycleTime(true);", fn == std::string::npos ? 0 : fn);
        CHECK(fn != std::string::npos && mes != std::string::npos && gate != std::string::npos && latch != std::string::npos);
        CHECK(fn < mes && mes < gate && gate < latch);
        std::printf("pins: WebRecipeChange.cpp:%u  HandlerBridgeCtl.cpp:%u-%u  WebStart.cpp:%u\n", (unsigned)LineAt(wrc, a),
                    (unsigned)LineAt(hbc, b), (unsigned)LineAt(hbc, bEnd), (unsigned)LineAt(ws, gate));
    }
    else
        CHECK(argc > 1);

    // ---- 7. the real CreateProcess path, THIS exe as the fake 7z ------------------
    W906_N06ExecHook = 0;
    const std::string cmdlog = root + "\\fake7z_cmdline.txt";
    PutEnv(std::string("W906_7Z_EXE=") + exe);
    PutEnv("W906_N06_FAKE7Z=1");
    PutEnv("W906_N06_FAKE_CMDLOG=" + cmdlog);
    PutEnv("W906_N06_FAKE_EXIT=0");
    PutEnv("W906_N06_FAKE_SLEEP=0");
    SECURITY_ATTRIBUTES saInh;
    std::memset(&saInh, 0, sizeof(saInh));
    saInh.nLength = sizeof(saInh);
    saInh.bInheritHandle = TRUE;                           // like a Winsock socket: inheritable by default
    char probeName[96];                                    // W-211: a named event, so the child can compare the object
    std::snprintf(probeName, sizeof(probeName), "Local\\st02_w195_n06_probe_%lu_%lu", (unsigned long)::GetCurrentProcessId(),
                  (unsigned long)::GetTickCount());
    const HANDLE hProbe = ::CreateEventA(&saInh, TRUE, TRUE, probeName);
    char probe[32];
    std::snprintf(probe, sizeof(probe), "%lu", (unsigned long)(UINT_PTR)hProbe);
    PutEnv(std::string("W906_N06_PROBE_HANDLE=") + probe);
    PutEnv(std::string("W906_N06_PROBE_NAME=") + probeName);
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == true);
    const std::string pv = ReadAll(cmdlog + ".probe");
    CHECK(pv == "NOT INHERITED");                          // 7z gets the pipe only (handle list), not the Handler's other handles
    if (pv != "NOT INHERITED") std::printf("  probe: [%s]\n", pv.c_str());
    PutEnv("W906_N06_PROBE_HANDLE=");
    PutEnv("W906_N06_PROBE_NAME=");
    ::CloseHandle(hProbe);
    const std::string cl = ReadAll(cmdlog);
    const std::string wantTail = "e \"" + zip + "\" -o\"" + dest + "\\\" -y -bsp1 -bb1";
    CHECK(cl.find(std::string("\"") + exe + "\" ") == 0);
    CHECK(cl.size() >= wantTail.size() && cl.compare(cl.size() - wantTail.size(), wantTail.size(), wantTail) == 0);
    if (cl.find(wantTail) == std::string::npos) std::printf("  child command line: [%s]\n  want tail:          [%s]\n", cl.c_str(), wantTail.c_str());
    const std::string n06Dir = root + "\\N06";
    CHECK(CountInTree(n06Dir, "N06_7z_", "  7z e \"") == 1);                  // one block per run: the header line
    CHECK(CountInTree(n06Dir, "N06_7z_", "    - OS_Setting.ini") == 1);         // 7z's file line (answer 1 = B: the day file)
    CHECK(CountInTree(n06Dir, "N06_7z_", "    100%") == 1);                     // the \r-ended progress line is a line of its own
    CHECK(CountInTree(n06Dir, "N06_7z_", "    Everything is Ok") == 1);
    CHECK(CountInTree(n06Dir, "N06_7z_", "  result: exit code 0") == 1);

    PutEnv("W906_N06_FAKE_EXIT=2");
    m0 = g_msgs;
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == false);
    CHECK(Has("7Z_EXIT_2"));
    CHECK(g_msgs == m0 + 1);
    CHECK(g_msgS1 == "N06 recipe sync to OS Tester FAILED: 7Z_EXIT_2 -- RCP1. Fix it and change the recipe again. START is blocked until then."
                     " (7z: ERROR: fake 7z exit 2)");                  // "Append 7z's last error line when there is one"
    CHECK(g_msgS2 == "OS 測試機工作檔同步失敗：7z 回報錯誤（代碼 2）（7Z_EXIT_2，RCP1）。請排除後重新換工作檔；完成前不能按 START。"
                     "（7z：ERROR: fake 7z exit 2）");
    CHECK(CountInTree(n06Dir, "N06_7z_", "  result: exit code 2") == 1);
    CHECK(W906_N06RecipeSyncBlocked() == true);

    PutEnv("W906_N06_FAKE_EXIT=0");
    PutEnv("W906_N06_FAKE_SLEEP=12500");
    const DWORD t0 = ::GetTickCount();
    CHECK(W906_N06SyncRecipeOrBlock("RCP1") == false);
    const DWORD dt = ::GetTickCount() - t0;
    CHECK(dt >= 9500 && dt <= 12400);                      // golden: wait 10 s; then terminate (+ up to 2 s for it to end)
    CHECK(g_msgS1 == "N06 recipe sync to OS Tester FAILED: EXEC_FAIL_OR_TIMEOUT -- RCP1. Fix it and change the recipe again. START is blocked until then.");
    CHECK(g_msgS2.find("7z 無法執行或超過 10 秒（已強制結束）（EXEC_FAIL_OR_TIMEOUT，RCP1）") != std::string::npos);
    CHECK(CountInTree(n06Dir, "N06_7z_", "  result: TIMEOUT -- no exit after 10 s, 7z terminated; last file: OS_Setting.ini") == 1);
    std::printf("timeout case: %lu ms\n", (unsigned long)dt);
    const DWORD left = ::GetTickCount() - t0;
    ::Sleep(left < 14000 ? 14000 - left : 0);              // past the fake child's own 12.5 s
    CHECK(::GetFileAttributesA((cmdlog + ".alive").c_str()) == INVALID_FILE_ATTRIBUTES);   // answer 2 = B: it was terminated
    PutEnv("W906_N06_FAKE7Z=0");

    W906_ShowMyMessage_Hook = 0;
    std::printf("event tracker: N06000001 x%d, N06000002 x%d; N06 day file: %d runs\n", Ok(), Ng(), CountInTree(n06Dir, "N06_7z_", "  7z "));
    if (g_fail == 0 && root.compare(0, tmpRoot.size(), tmpRoot) == 0)
    {
        RemoveTree(root);
        std::printf("test_st02_w195_n06: %d/%d passed (temp folder %s removed)\n", g_total - g_fail, g_total, root.c_str());
    }
    else
        std::printf("test_st02_w195_n06: %d/%d passed (files kept under %s for diagnosis)\n", g_total - g_fail, g_total, root.c_str());
    return g_fail == 0 ? 0 : 1;
}
