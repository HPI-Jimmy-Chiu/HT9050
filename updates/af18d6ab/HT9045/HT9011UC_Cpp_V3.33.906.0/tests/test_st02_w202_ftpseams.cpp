// ===========================================================================
//  tests/test_st02_w202_ftpseams.cpp -- W-202 (2) LI-9 F2-2: the KYEC FTP engine / exec seams (KYECFTP/KyecFtpSeams_St02.*) on
//  the golden lines of KYECFTP/FTPClient_Transfer.cpp, Gate #4 (F2-1) in its golden place, and D-9 (the 0618->912 hunks of
//  UploadFileToServer2).  AI(W906-W202) 20261009 (St02-E).
//
//  The real UploadFileToServer2 runs end to end against a FAKE FTP server: the prepare hook installs it on the engine the golden
//  line creates (the engine stays in SIM mode, no socket); the system / CopyFile hooks record the del / 7z / copy commands instead
//  of running them (a "7z a" just writes a small file so the STOR has something to send); Gate #4's own 7z goes through
//  W906_N06ExecHook (K1).  Every path is under %TEMP%\ht9045_w202f2_<tick>; the test stops before any call if one is under
//  D:\HT9045 or D:\rms.  golden's SetCurrentDirectory("D://") does run (it only changes this process's cwd; restored at the end).
//
//    1. bZip=true, [N06] on: connect -> pre-delete -> Gate #4 (a -tzip <Data>RCP1\OS_Setting.zip <Tester>\RCP1.ini) -> the recipe
//       zip -> the offset zip -> STOR RCP1.zip -> STOR RCP1.Offset -> the final dels; "Upload done"; TimeOut 20000 (golden :407-411).
//    2. [N06] off: no Gate #4, the upload is the same (golden 913 :564 + K1 :1157 flag check).
//    3. D-9 PTI: TimeOut 5000 (golden 913 :410); bZip=false connect failure -> log only, no message (golden :485-488); others
//       still get "FTP Server is not connected".
//    4. LoadFileFormServer2 calls the prepare hook too (:374).
//    5. source pins: no bare system( / CopyFile( left in UploadFileToServer2; the TSMC arm has no Gate #4; D-9 lines.
// ===========================================================================
#include "KYECFTP/FTPClient_Transfer.h"
#include "KYECFTP/FTPClient_EventHandlers.h"
#include "KYECFTP/MiniFtpEngine.h"
#include "KYECFTP/KyecFtpSeams_St02.h"
#include "Interface/TesterTCP_N06_St02.h"
#include "canary_support.h"
#include "cmydef.h"
#include "cprod.h"
#include "common.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "forms/fTemp_Set.h"   // the W-195 rule: create fTemp_Set first (see main)

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <sstream>
#include <string>
#include <vector>

#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif

static int g_fail = 0;
static int g_total = 0;
static void check(bool ok, const char* what)
{
    ++g_total;
    std::printf("%s %s\n", ok ? "PASS" : "FAIL", what);
    if (!ok) ++g_fail;
}

// ---- the event log of one run ------------------------------------------------------
static std::vector<std::string> g_seq;
static int Find(const std::string& prefix, int from = 0)
{
    for (int i = from; i < (int)g_seq.size(); ++i)
        if (g_seq[i].compare(0, prefix.size(), prefix) == 0) return i;
    return -1;
}
static void Dump()
{
    for (size_t i = 0; i < g_seq.size(); ++i) std::printf("     %2u %s\n", (unsigned)i, g_seq[i].c_str());
}

// ---- the fake FTP server (MiniFtpEngine SIM hook contract, MiniFtpEngine.h:286-330) -------------------------------------------
struct FakeServer {
    Nmftp::TNMFTP* engine;
    int timeOutAtConnect;
    FakeServer() : engine(0), timeOutAtConnect(-1) {}
    void Push(const char* s) { engine->DebugControlSocket()->Socket->SimPushReceive(s, (int)std::strlen(s)); }
    void operator()(const char* channel, const AnsiString& raw)
    {
        const std::string ch(channel), line(raw.c_str());
        if (ch == "CTRL-CONNECT") { g_seq.push_back("connect"); timeOutAtConnect = engine->TimeOut; Push("220 Fake FTP\r\n"); return; }
        if (ch == "DATA-OPEN") { g_seq.push_back("data-open"); return; }
        if (ch == "DATA-CLOSE") { g_seq.push_back("data-close"); Push("226 Transfer complete\r\n"); return; }
        g_seq.push_back("ctrl:" + line);
        if (line.compare(0, 4, "USER") == 0) Push("331 Password required\r\n");
        else if (line.compare(0, 4, "PASS") == 0) Push("230 Logged in\r\n");
        else if (line.compare(0, 4, "PASV") == 0) Push("227 Entering Passive Mode (127,0,0,1,15,161).\r\n");
        else if (line.compare(0, 4, "STOR") == 0) Push("150 Opening data connection\r\n");
        else if (line.compare(0, 4, "QUIT") == 0) Push("221 Bye\r\n");
        else Push("200 OK\r\n");
    }
};
static FakeServer g_srv;
static bool g_serve = true;     // false: the engine gets no fake server -> the connect fails (as with no server)
static int  g_prepared = 0;
static void Prepare(Nmftp::TNMFTP* e)
{
    ++g_prepared;
    if (!g_serve) return;
    g_srv.engine = e;
    e->SetSimServerHook(std::ref(g_srv));
}

// ---- the exec hooks -------------------------------------------------------------------
static std::string FirstQuoted(const std::string& s)
{
    const size_t a = s.find('"');
    if (a == std::string::npos) return "";
    const size_t b = s.find('"', a + 1);
    return b == std::string::npos ? "" : s.substr(a + 1, b - a - 1);
}
static int FakeSystem(const char* cmd)
{
    const std::string c(cmd ? cmd : "");
    g_seq.push_back("sys:" + c);
    if (c.find("7z.exe a -tzip") != std::string::npos) {           // stand-in for 7z: the archive appears
        std::ofstream f(FirstQuoted(c).c_str(), std::ios::binary);
        f << "fake zip for St02_W202FtpSeams";
    }
    return 0;
}
static bool FakeCopy(const char* a, const char* b, bool)
{
    g_seq.push_back(std::string("copy:") + (a ? a : "") + " -> " + (b ? b : ""));
    return true;
}
static bool FakeN06Exec(const char* s7z, const char* sParam, unsigned long* pdwExit)
{
    (void)s7z;
    g_seq.push_back(std::string("gate4:") + (sParam ? sParam : ""));
    if (pdwExit) *pdwExit = 0;
    return true;
}

// ---- message capture ------------------------------------------------------------------
static int g_msgs = 0;
static std::string g_msg;
static void Msg(const char* s1, const char*) { ++g_msgs; g_msg = s1 ? s1 : ""; g_seq.push_back(std::string("msg:") + g_msg); }

static bool Forbidden(const std::string& p)
{
    std::string s = p;
    for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a');
    for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\';
    return s.compare(0, 9, "d:\\ht9045") == 0 || s.compare(0, 6, "d:\\rms") == 0;
}
static void RemoveTree(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            if (std::strcmp(fd.cFileName, ".") == 0 || std::strcmp(fd.cFileName, "..") == 0) continue;
            const std::string p = dir + "\\" + fd.cFileName;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(dir.c_str());
}
static void Touch(const std::string& p) { std::ofstream f(p.c_str(), std::ios::binary); f << "x"; }
static std::string Read(const std::string& p)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static void Run(const char* name, bool bZip)
{
    g_seq.clear();
    g_srv.timeOutAtConnect = -1;
    FTPClientTransfer_ResetStateForTest();
    UploadFileToServer2("/incoming/", name, bZip);
}

int main(int argc, char** argv)
{
    char tmp[MAX_PATH];
    ::GetTempPathA(sizeof(tmp), tmp);
    char cwd[MAX_PATH] = { 0 };
    ::GetCurrentDirectoryA(MAX_PATH, cwd);
    char stamp[32];
    std::snprintf(stamp, sizeof(stamp), "%lu", (unsigned long)::GetTickCount());
    const std::string tmpRoot = std::string(tmp) + "ht9045_w202f2_";
    const std::string root = tmpRoot + stamp;
    const std::string data = root + "\\Data\\", offs = root + "\\Offset\\", def = root + "\\Default\\";
    const std::string tester = root + "\\Tester", bin = root + "\\bin";
    ::CreateDirectoryA(root.c_str(), 0);
    ::CreateDirectoryA((root + "\\Data").c_str(), 0);
    ::CreateDirectoryA((root + "\\Offset").c_str(), 0);
    ::CreateDirectoryA((root + "\\Default").c_str(), 0);
    ::CreateDirectoryA((data + "RCP1").c_str(), 0);
    ::CreateDirectoryA((offs + "RCP1").c_str(), 0);
    ::CreateDirectoryA(tester.c_str(), 0);
    ::CreateDirectoryA(bin.c_str(), 0);
    Touch(data + "RCP1\\Recipe.Data");
    Touch(offs + "RCP1\\Offset.Data");
    Touch(data + "RCP1\\OS_Setting.zip");
    Touch(tester + "\\RCP1.ini");
    Touch(bin + "\\7z.exe");
    static char env7z[600];
    std::snprintf(env7z, sizeof(env7z), "W906_7Z_EXE=%s\\7z.exe", bin.c_str());
    HT9045_TEST_PUTENV(env7z);

    DataPath = AnsiString(data.c_str());
    OffsetPath = AnsiString(offs.c_str());
    DefaultPath = AnsiString(def.c_str());
    as9045LogPath = AnsiString(root.c_str());
    asSaveEventLogPath = AnsiString((root + "\\SaveEventLog").c_str());
    IniConfig.asN06_TesterPath = AnsiString(tester.c_str());
    const char* paths[] = { DataPath.c_str(), OffsetPath.c_str(), DefaultPath.c_str(), as9045LogPath.c_str(), asSaveEventLogPath.c_str(),
                            IniConfig.asN06_TesterPath.c_str() };
    for (size_t i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i)
        if (Forbidden(paths[i]) || std::string(paths[i]).compare(0, root.size(), root) != 0) {
            std::printf("STOP: %s is not under the sandbox %s -- nothing called\n", paths[i], root.c_str());
            return 1;
        }
    if (fTemp_Set == 0) fTemp_Set = new TfTemp_Set();   // W-195 rule (laptop batch 136)

    W906_KyecFtpPrepareHook = &Prepare;
    W906_KyecFtpSystemHook = &FakeSystem;
    W906_KyecFtpCopyFileHook = &FakeCopy;
    W906_N06ExecHook = &FakeN06Exec;
    W906_ShowMyMessage_Hook = &Msg;
    FTPClientTransfer_SetFastDelayForTest(true);

    CUSTOMER_CODE = 0;
    bSigurdUpload_Recipe = false; bSigurdUpload_Jamcode = false;
    IniConfig.FtpHost = "127.0.0.1"; IniConfig.FtpUserName = "st02-test"; IniConfig.FtpPassword = "st02-test"; IniConfig.N06_FtpPort = "21";
    IniConfig.FtpTransMode = 0;
    CosFunction.bUseFTPDownLoadATCRecipe = false; CosFunction.bUseATCFileTransfer = false;
    IniConfig.bN06_CopyTesterFile = true;
    const bool b7zOnD = ::GetFileAttributesA("d:\\HT9045\\7z.exe") != INVALID_FILE_ATTRIBUTES;

    // ---- 1. the happy path ---------------------------------------------------------------
    g_msgs = 0;
    Run("RCP1", true);
    Dump();
    const int iConn = Find("connect"), iPre = Find("sys:del " + def + "RCP1.zip"), iGate = Find("gate4:");
    const int iZip = Find("sys:d:\\HT9045\\7z.exe a -tzip \"" + data + "RCP1.zip\"");
    const int iOff = Find("sys:d:\\HT9045\\7z.exe a -tzip \"" + offs + "RCP1.Offset\"");
    const int iStorZ = Find("ctrl:STOR /incoming/RCP1.zip"), iStorO = Find("ctrl:STOR /incoming/RCP1.Offset");
    const int iDelZ = Find("sys:del \"" + data + "RCP1.zip\""), iDelO = Find("sys:del \"" + offs + "RCP1.Offset\"");
    check(g_prepared >= 1 && bError == false, "1: the prepare hook installed the fake server; the upload ends with bError false");
    check(iConn >= 0 && iConn < iPre && iPre < iGate && iGate < iZip && iZip < iOff && iOff < iStorZ && iStorZ < iStorO && iStorO < iDelZ &&
          iDelZ < iDelO,
          "1: connect -> pre-delete -> Gate #4 -> recipe zip -> offset zip -> STOR zip -> STOR Offset -> dels (golden 913 :440-708)");
    check(iGate >= 0 && g_seq[iGate] == "gate4:a -tzip \"" + data + "RCP1\\OS_Setting.zip\" \"" + tester + "\\RCP1.ini\"",
          "1: Gate #4 = K1's CopyRecipeFromTester: a -tzip <Data>RCP1\\OS_Setting.zip <Tester>\\RCP1.ini (golden 913 FTPClient.cpp:564)");
    check(b7zOnD ? Find("copy:") < 0 : (Find("copy:") >= 0 && Find("copy:") < iGate),
          "1: the d:\\HT9045\\7z.exe copy goes through the seam (recorded, never run) only when that file is missing");
    check(g_msg == "Upload done" && g_srv.timeOutAtConnect == 20000, "1: \"Upload done\"; TimeOut 20000 for a non-PTI customer");

    // ---- 2. [N06] off ----------------------------------------------------------------------
    IniConfig.bN06_CopyTesterFile = false;
    Run("RCP1", true);
    check(Find("gate4:") < 0 && Find("ctrl:STOR /incoming/RCP1.zip") >= 0 && bError == false,
          "2: [N06] off -> no Gate #4 7z, the upload itself unchanged");
    IniConfig.bN06_CopyTesterFile = true;

    // ---- 3. D-9 (PTI) ---------------------------------------------------------------------
    CUSTOMER_CODE = CC_PTI;
    Run("RCP1", true);
    check(g_srv.timeOutAtConnect == 5000 && Find("ctrl:STOR /incoming/RCP1.zip") >= 0, "3: PTI TimeOut 5000 (golden 913 :410), upload still done");
    g_serve = false;                                         // no server: the connect fails
    int m0 = g_msgs;
    Run("JAMLOG", false);
    check(bError == true && g_msgs == m0, "3: PTI, bZip=false, connect fails -> bError, no message box (golden 913 :485-488)");
    CUSTOMER_CODE = 0;
    m0 = g_msgs;
    Run("JAMLOG", false);
    check(bError == true && g_msgs > m0 && g_msg == "FTP Server is not connected", "3: other customers still get \"FTP Server is not connected\"");

    // ---- 4. LoadFileFormServer2 uses the prepare seam too ----------------------------------------------
    const int p0 = g_prepared;
    FTPClientTransfer_ResetStateForTest();
    LoadFileFormServer2("/incoming", "RCP1");
    check(g_prepared == p0 + 1, "4: LoadFileFormServer2's engine goes through W906_KyecFtpPrepare (:374)");
    g_serve = true;

    // ---- 5. source pins ---------------------------------------------------------------------
    if (argc > 1) {
        const std::string src = Read(std::string(argv[1]) + "/KYECFTP/FTPClient_Transfer.cpp");
        const size_t a = src.find("\nvoid UploadFileToServer2(");
        const size_t b = src.find("\nvoid Download_2DSortingList(", a == std::string::npos ? 0 : a);
        std::string body = (a == std::string::npos || b == std::string::npos) ? "" : src.substr(a, b - a);
        std::string code;                                     // the body without // comments
        { std::istringstream in(body); std::string l;
          while (std::getline(in, l)) {
              if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
              const size_t c = l.find("//");
              code += (c == std::string::npos ? l : l.substr(0, c)) + "\n";
          } }
        size_t bare = 0;
        for (size_t p = code.find("system("); p != std::string::npos; p = code.find("system(", p + 1))
            ++bare;                                           // the seam is W906_KyecFtpSystem( (capital S): any lower-case system( is bare
        for (size_t p = code.find("CopyFile("); p != std::string::npos; p = code.find("CopyFile(", p + 1))
            if (p < 12 || code.compare(p - 12, 12, "W906_KyecFtp") != 0) ++bare;
        check(!code.empty() && bare == 0, "5: UploadFileToServer2 has no bare system( / CopyFile( left -- every exec goes through the seams");
        const size_t t0 = code.find("if (CUSTOMER_CODE == CC_TSMC_TAINAN)"), t1 = code.find("TesterTCP_CopyRecipeFromTester(");
        const size_t tElse = code.find("\n            else\n", t0 == std::string::npos ? 0 : t0);
        check(t0 != std::string::npos && tElse != std::string::npos && t1 != std::string::npos && t0 < tElse && tElse < t1,
              "5: Gate #4 sits after the TSMC arm (golden 913 :513-551 has none), in the generic zip arm");
        check(body.find("NMFTP2->TimeOut = (CUSTOMER_CODE == CC_PTI) ? 5000 : 20000;") != std::string::npos &&
              body.find("if (CUSTOMER_CODE == CC_AMD_M && iAMD_Function == 1) // AI(W906-W202) 20261009 (St02-E) D-9") != std::string::npos &&
              body.find("(Source.Pos(\"_FT\") > 0 || Source.Pos(\"_QA\") > 0)") != std::string::npos &&
              body.find("int iPos = (Source.Pos(\"_FT\") > 0) ? Source.AnsiPos(\"_FT\") : Source.AnsiPos(\"_QA\");") != std::string::npos &&
              body.find("else if (CUSTOMER_CODE == CC_PTI) { MyDBIProcess(\"Exception\", \"TfFTPClient::UploadFileToServer2\", e.Message); } else") != std::string::npos &&
              body.find("if (CUSTOMER_CODE == CC_PTI) { MyDBIProcess(\"Exception\", \"TfFTPClient::UploadFileToServer2 unknown error\"); }") != std::string::npos,
              "5: D-9 lines (golden 913 :410 / :617 kept CC_AMD_M per C-1 / :720-723 / :825-829 / :845-853)");
    } else
        check(argc > 1, "argv[1] = the tree root (part 5)");

    W906_KyecFtpPrepareHook = 0; W906_KyecFtpSystemHook = 0; W906_KyecFtpCopyFileHook = 0; W906_N06ExecHook = 0; W906_ShowMyMessage_Hook = 0;
    ::SetCurrentDirectoryA(cwd);                              // golden SetCurrentDirectory("D://") changed it
    if (g_fail == 0 && root.compare(0, tmpRoot.size(), tmpRoot) == 0) RemoveTree(root);
    std::printf("test_st02_w202_ftpseams: %d/%d passed%s\n", g_total - g_fail, g_total, g_fail ? (" (kept " + root + ")").c_str() : "");
    return g_fail == 0 ? 0 : 1;
}
