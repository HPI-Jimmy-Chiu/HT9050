// =============================================================================
//  test_li9_ftpclient.cpp -- card LI-9 F1: golden btnFtpServerClick (906_0625_Steven uLotInfo.cpp:5001-5126) and the KYEC
//  FTP dialog TfFTPClient's HD / Tester pages (KYECFTP/FTPClient.cpp), through WS act.lotInfoFtp.<op>.
//
//  AI(W906-LI9-F1) 20261002 (St02-E helper).  Suite name (add_test): LI9_FtpClient
//  Under test: JsonBridge/actions/LotInfoFtp.cpp (dispatcher), WebLotInfoFtp_St02.cpp (the op body), forms/fLotInfo_Ftp_St02.cpp,
//  KYECFTP/FTPClientForm_St02.cpp.  NOT linked: WebLotInfoFtpInstall_St02.cpp (wb_serve only) -- the hooks are fakes here:
//  ChangeSetUpFile records the name (no recipe is loaded, SetUp.inf is not written), RecipeChainsReady says true, LocalIp
//  answers 10.0.0.7 (no socket API is called).
//
//  NEVER THE NETWORK.  Server (tag 0) runs on MiniFtpEngine in SIM (no prepare hook = no server = "not connected"; [7] / [8]
//  install a fake server through the prepare hook -- still SIM, no socket); the only network-shaped path, a Taster map under
//  \\host\share, is run only in the SIM build where W58 Q5 skips it by the path string alone (W906_SimNetPathBlocked), and
//  only when W906_SIM_NET_PATHS is not 1.
//  CONTAINMENT FIRST: exit 2 before any Handler code unless ctest's redirect roots are set (st02_test_containment.h) and
//  DataPath / OffsetPath / AuthPath are ctest's scratch.  Then DataPath, OffsetPath, AuthPath and the two Taster files are
//  pointed at <as9045LogPath>\LI9_FtpClient_<tick>\ (inside machine_log_scratch) and restored at the end.  Fails if the
//  machine's D:\HT9045\config\config.ini or D:\HT9045\SetUp.inf changed (size / write time).
//    1  dispatcher: not-installed, bad-op
//    2  dfm defaults of the Lot Info FTP buttons; the body installs
//    3  btnFtpServerClick guards: tab-hidden, button-disabled, access-level (button back on), system-start, bad-tag,
//       tag 3 (non-TSMC returns), tag 2 + SPIL (golden quirk: stays disabled), tag 0 = server-list-failed (no server; POOL-14 MR-A)
//    4  HD: the list (fallback = DataPath folders when fMain->cbSetupFileName is empty; else its Items), FilterList exact /
//       case-insensitive, pick, "FTP Form already Opened!!", loadHD -> ChangeSetUpFile + close + the btnFtpServerClick tail,
//       refused loadHD (未選擇這路徑), mode 1 (golden :1648 quirk), enterHD, boot-read-chain guard, hook missing, close, not-open
//    5  Tester: list file -> combos, cbTesterTypeChange / cbTesterIDChange, Save writes the map + config.ini only in the
//       sandbox, a bad name (Tester Name 錯誤, nothing closes), Manually reads config.ini, exit, missing list file = VCL
//       EFOpenError, W58 Q5 network map skipped (SIM build)
//    6  memoClear
//    7  Upload to Server (card LI-9 F2-3, AI(W906-W202) 20261009 (St02-E)): golden 913 plUnloadClick -> the real UploadFileToServer2
//       against a FAKE FTP server on the engine golden creates (the F2-2 prepare hook; SIM, no socket), del / 7z / CopyFile recorded
//       by the F2-2 exec hooks (nothing runs); guards (not-open, wrong-page, 未選擇這路徑, PE model, AMD gated, running, disabled),
//       no server -> "FTP Server is not connected", the KYEC ATC arm with no ATC link (golden 913 :1210-1221).
//    8  Server list (POOL-14 MR-A, AI(W906-P14) 20261010 (St02-E)): golden 913 ShowFTPModal case 0 against a FAKE FTP server that
//       answers NLST (same SIM hook contract, no socket): the list (golden's `||` quirk kept), CWD / NLST order, NMFTP3 kept for
//       the dialog, filter / pick / Copy File Name / Clean File Name, barcode keys, close -> ABOR, empty directory, no server.
//    [3] tag 0 (no server hook) = golden's "not connected" path; [4] the Server edit on the HD page = wrong-page.
// =============================================================================
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "vclcompat/vcl_compat.h"
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "CosFunction.h"
#include "common.h"
#include "canary_support.h"
#include "forms/fLotInfo.h"
#include "forms/fMain.h"
#include "Public/cJSON.h"
#include "KYECFTP/FTPClientForm_St02.h"
#include "forms/fLotInfo_Ftp_St02.h"
#include "JsonBridge/actions/LotInfoFtp.h"

#include "st02_test_containment.h"
#include "KYECFTP/FTPClient_Transfer.h"     // [7] F2-3: FTPClientTransfer_SetFastDelayForTest, bPIDTransferErr
#include "KYECFTP/MiniFtpEngine.h"
#include "KYECFTP/KyecFtpSeams_St02.h"      // [7] F2-3: the prepare / exec hooks
#include "forms/fLotInfo_Download_St02.h"   // [9] POOL-14 MR-B1: DownloadFromServer
#include "forms/fSetup.h"                   // [11] POOL-14 MR-B3: fSetup->bFirstTime
#include "SECSGEM/SecsEventReport.h"        // [12] POOL-14 MR-C: g_SimLastEventReportCeid / ResetSimEventReport
#include "SECSGEM/SecsEventType.h"          // [12] SECS_EVENT.DownLoadRecipeByFTPOK / NG
#include <functional>

// JsonBridge/FormJson.cpp is a wb_serve source, not in an archive: the op body's FormLock / FormUnlock are stand-ins here
//   (the same as tests/test_note_auth.cpp).
namespace ht9045 { namespace formjson { void FormLock() {} void FormUnlock() {} } }

void W906_St02_LotInfoFtpRegisterBody();   // WebLotInfoFtp_St02.cpp

static int g_pass = 0, g_fail = 0;
static void Check(bool cond, const char* msg, int line)   // one line per piece: no line ends in a backslash
{
    if (cond) { std::printf("  PASS: %s\n", msg); ++g_pass; }
    else      { std::printf("  FAIL: %s  (line %d)\n", msg, line); ++g_fail; }
}
#define CHECK(cond, msg) Check((cond), (msg), __LINE__)

static bool Has(const std::string& s, const char* t) { return s.find(t) != std::string::npos; }
static std::string Act(const char* op, const char* value) { return ht9045::sjson::W906_LotInfoFtpAct(std::string("act.lotInfoFtp.") + op, value); }

// ---- reply parsing ----
struct Reply {
    cJSON* root;
    explicit Reply(const std::string& s) : root(cJSON_Parse(s.c_str())) {}
    ~Reply() { if (root) cJSON_Delete(root); }
    const cJSON* At(const char* path) const          // "state.hd.lstHDFile.items"
    {
        const cJSON* n = root;
        std::string p(path);
        std::size_t a = 0;
        while (n && a <= p.size())
        {
            std::size_t b = p.find('.', a);
            if (b == std::string::npos) b = p.size();
            n = cJSON_GetObjectItemCaseSensitive(n, p.substr(a, b - a).c_str());
            a = b + 1;
            if (b == p.size()) break;
        }
        return n;
    }
    std::string S(const char* path) const { const cJSON* n = At(path); return (n && cJSON_IsString(n) && n->valuestring) ? n->valuestring : std::string(); }
    bool B(const char* path) const { const cJSON* n = At(path); return n && cJSON_IsTrue(n); }
    int I(const char* path) const { const cJSON* n = At(path); return (n && cJSON_IsNumber(n)) ? n->valueint : -999; }
    std::vector<std::string> L(const char* path) const
    {
        std::vector<std::string> v;
        const cJSON* n = At(path);
        if (n && cJSON_IsArray(n)) for (const cJSON* it = n->child; it; it = it->next) if (cJSON_IsString(it)) v.push_back(it->valuestring);
        return v;
    }
    bool Msg(const char* s1) const
    {
        const cJSON* n = At("messages");
        if (n && cJSON_IsArray(n)) for (const cJSON* it = n->child; it; it = it->next)
        {
            const cJSON* m = cJSON_GetObjectItemCaseSensitive(it, "s1");
            if (m && cJSON_IsString(m) && std::strstr(m->valuestring, s1)) return true;
        }
        return false;
    }
};
static std::string Join(const std::vector<std::string>& v) { std::string s; for (std::size_t i = 0; i < v.size(); ++i) { if (i) s += "|"; s += v[i]; } return s; }

// ---- fakes ----
static std::vector<std::string> g_changes;
static int  g_fakeMode = 0;
static bool g_chainsReady = true;
static int  FakeChangeSetUpFile(AnsiString FileName) { g_changes.push_back(FileName.c_str()); return g_fakeMode; }
static bool FakeChainsReady() { return g_chainsReady; }
static AnsiString FakeLocalIp() { return AnsiString("10.0.0.7"); }

// ---- files ----
struct FileMark { bool exists; unsigned long long size, mtime; };
static FileMark FileMarkOf(const char* path)
{
    FileMark m = { false, 0, 0 };
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (GetFileAttributesExA(path, GetFileExInfoStandard, &d))
    {
        m.exists = true;
        m.size = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        m.mtime = ((unsigned long long)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
    }
    return m;
}
static bool Same(const FileMark& a, const FileMark& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }
static std::string ReadAll(const std::string& path)
{
    std::ifstream in(path.c_str(), std::ios::binary);
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}
static void WriteAll(const std::string& path, const std::string& text)
{
    std::ofstream out(path.c_str(), std::ios::binary);
    out << text;
}

// ---- [7] F2-3: the fake FTP server + the exec recorder (as tests/test_st02_w202_ftpseams.cpp; MiniFtpEngine.h SIM hook contract) ----
static std::vector<std::string> g_seq7;
static int Find7(const std::string& prefix)
{
    for (int i = 0; i < (int)g_seq7.size(); ++i)
        if (g_seq7[i].compare(0, prefix.size(), prefix) == 0) return i;
    return -1;
}
struct FakeServer7 {
    Nmftp::TNMFTP* engine;
    std::vector<std::string> stor;
    FakeServer7() : engine(0) {}
    void Push(const char* s) { engine->DebugControlSocket()->Socket->SimPushReceive(s, (int)std::strlen(s)); }
    void operator()(const char* channel, const AnsiString& raw)
    {
        const std::string ch(channel), line(raw.c_str());
        if (ch == "CTRL-CONNECT") { g_seq7.push_back("connect"); Push("220 Fake FTP\r\n"); return; }
        if (ch == "DATA-OPEN") { g_seq7.push_back("data-open"); return; }
        if (ch == "DATA-CLOSE") { g_seq7.push_back("data-close"); Push("226 Transfer complete\r\n"); return; }
        std::string l = line;
        while (!l.empty() && (l[l.size() - 1] == '\r' || l[l.size() - 1] == '\n')) l.erase(l.size() - 1);
        g_seq7.push_back("ctrl:" + l);
        if (l.compare(0, 4, "USER") == 0) Push("331 Password required\r\n");
        else if (l.compare(0, 4, "PASS") == 0) Push("230 Logged in\r\n");
        else if (l.compare(0, 4, "PASV") == 0) Push("227 Entering Passive Mode (127,0,0,1,15,161).\r\n");
        else if (l.compare(0, 4, "STOR") == 0) { stor.push_back(l.size() > 5 ? l.substr(5) : std::string()); Push("150 Opening data connection\r\n"); }
        else if (l.compare(0, 4, "QUIT") == 0) Push("221 Bye\r\n");
        else Push("200 OK\r\n");
    }
};
static FakeServer7 g_srv7;
static bool g_serve7 = true;     // false: no fake server on the engine -> the connect fails (as with no server)
static void Prepare7(Nmftp::TNMFTP* e)
{
    if (!g_serve7) return;
    g_srv7.engine = e;
    e->SetSimServerHook(std::ref(g_srv7));
}
static std::string FirstQuoted7(const std::string& s)
{
    const size_t a = s.find('"');
    if (a == std::string::npos) return "";
    const size_t b = s.find('"', a + 1);
    return b == std::string::npos ? "" : s.substr(a + 1, b - a - 1);
}
static int FakeSystem7(const char* cmd)
{
    const std::string c(cmd ? cmd : "");
    g_seq7.push_back("sys:" + c);
    if (c.find("7z.exe a -tzip") != std::string::npos) {           // stand-in for 7z: the archive appears (inside the sandbox)
        const std::string out = FirstQuoted7(c);
        if (W906TestSafeLower(out.c_str()).find("machine_log_scratch") != std::string::npos) WriteAll(out, "fake zip for LI9_FtpClient [7]");
    }
    return 0;
}
static bool FakeCopy7(const char* a, const char* b, bool)
{
    g_seq7.push_back(std::string("copy:") + (a ? a : "") + " -> " + (b ? b : ""));
    return true;
}
static bool HasMd5(const std::string& dir)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*.MD5").c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return false;
    ::FindClose(h);
    return true;
}

// ---- [8] POOL-14 MR-A: a fake FTP server that answers NLST (MiniFtpEngine.h SIM hook contract: the listing is pushed on
//      DATA-OPEN, which fires before NLST is sent; "226" on DATA-CLOSE) ----
static std::vector<std::string> g_seq8;
struct FakeServer8 {
    Nmftp::TNMFTP* engine;
    std::vector<std::string> listing;         // NLST lines
    // [12] POOL-14 MR-C: byCmd -> the data bytes are pushed when the command arrives (NLST: listing; RETR: retr, plus the RETR count when
    //   vary, so the three downloads LoadFileFormServer2 compares differ in size); DATA-OPEN then pushes nothing
    bool byCmd; std::string retr; bool vary; int retrCount;
    FakeServer8() : engine(0), byCmd(false), vary(false), retrCount(0) {}
    void PushData(const std::string& d) { if (!d.empty()) engine->DebugDataSocket()->Socket->SimPushReceive(d.c_str(), (int)d.size()); }
    void Push(const char* s) { engine->DebugControlSocket()->Socket->SimPushReceive(s, (int)std::strlen(s)); }
    void operator()(const char* channel, const AnsiString& raw)
    {
        const std::string ch(channel), line(raw.c_str());
        if (ch == "CTRL-CONNECT") { g_seq8.push_back("connect"); Push("220 Fake FTP\r\n"); return; }
        if (ch == "DATA-OPEN")
        {
            g_seq8.push_back("data-open");
            if (byCmd) return;
            std::string payload;
            for (std::size_t i = 0; i < listing.size(); ++i) payload += listing[i] + "\r\n";
            if (!payload.empty()) engine->DebugDataSocket()->Socket->SimPushReceive(payload.c_str(), (int)payload.size());
            return;
        }
        if (ch == "DATA-CLOSE") { g_seq8.push_back("data-close"); Push("226 Transfer complete\r\n"); return; }
        std::string l = line;
        while (!l.empty() && (l[l.size() - 1] == '\r' || l[l.size() - 1] == '\n')) l.erase(l.size() - 1);
        g_seq8.push_back("ctrl:" + l);
        if (l.compare(0, 4, "USER") == 0) Push("331 Password required\r\n");
        else if (l.compare(0, 4, "PASS") == 0) Push("230 Logged in\r\n");
        else if (l.compare(0, 3, "PWD") == 0) Push("257 \"/\" is the current directory\r\n");
        else if (l.compare(0, 3, "CWD") == 0) Push("250 CWD ok\r\n");
        else if (l.compare(0, 4, "PASV") == 0) Push("227 Entering Passive Mode (127,0,0,1,15,161).\r\n");
        else if (l.compare(0, 4, "NLST") == 0)
        {
            if (byCmd) { std::string p; for (std::size_t i = 0; i < listing.size(); ++i) p += listing[i] + "\r\n"; PushData(p); }
            Push("150 Opening data connection\r\n");
        }
        else if (l.compare(0, 4, "RETR") == 0)                                  // [9i] MR-B1: every RETR gets `listing` as its bytes (byCmd off)
        {
            if (byCmd) PushData(vary ? retr + std::string((std::size_t)(++retrCount), '#') : retr);
            Push("150 Opening data connection\r\n");
        }
        else if (l.compare(0, 4, "QUIT") == 0) Push("221 Bye\r\n");
        else Push("200 OK\r\n");
    }
};
static FakeServer8 g_srv8;
static bool g_serve8 = true;
static void Prepare8(Nmftp::TNMFTP* e)
{
    if (!g_serve8) return;
    g_srv8.engine = e;
    // [12] POOL-14 MR-C: the Server list's engine (NMFTP3) stays open while LoadFileFormServer2 makes and deletes its own, so each
    //   engine's hook names itself before the shared fake answers (it pushes replies into that engine's sockets)
    e->SetSimServerHook([e](const char* ch, const AnsiString& raw) { g_srv8.engine = e; g_srv8(ch, raw); });
}
static int Find8(const std::string& prefix)
{
    for (int i = 0; i < (int)g_seq8.size(); ++i)
        if (g_seq8[i].compare(0, prefix.size(), prefix) == 0) return i;
    return -1;
}

// ---- [9] POOL-14 MR-B1: a fake 7z behind the exec seams -- `7z.exe e "<zip>" -o"<dir>\" -y` drops the recipe files into <dir>,
//      but only inside the sandbox; everything else (the 7z.exe copy into d:\HT9045, a copy from outside) is only recorded ----
static std::vector<std::string> g_seq9;
static std::vector<std::string> g_unzip9 = { "ArmCondition.Data", "Binasgn.Data", "Contact.Data", "HandlerCondition.Data",
                                             "HotPlate.Data", "Temperature.Data", "Tester.Data", "Tray.Data" };
static int g_shellRet9 = 42;
static bool InSandbox9(const std::string& p) { return W906TestSafeLower(p.c_str()).find("machine_log_scratch") != std::string::npos; }
static void FakeUnzip9(const std::string& args)
{
    const size_t a = args.find("-o\"");
    const size_t b = a == std::string::npos ? a : args.find("\\\"", a + 3);
    if (a == std::string::npos || b == std::string::npos) return;
    const std::string dir = args.substr(a + 3, b - a - 3) + "\\";
    if (!InSandbox9(dir)) return;
    MyForceDirectories(AnsiString(dir.c_str()));
    for (std::size_t i = 0; i < g_unzip9.size(); ++i) WriteAll(dir + g_unzip9[i], "fake recipe file for LI9_FtpClient [9]");
}
static int FakeSystem9(const char* cmd)
{
    const std::string c(cmd ? cmd : "");
    g_seq9.push_back("sys:" + c);
    if (c.find("7z.exe e ") != std::string::npos) FakeUnzip9(c);
    return 0;
}
static int FakeShell9(const char* file, const char* params)
{
    const std::string p(params ? params : "");
    g_seq9.push_back(std::string("shell:") + (file ? file : "") + " " + p);
    if (p.compare(0, 2, "e ") == 0) FakeUnzip9(p);
    return g_shellRet9;
}
static bool FakeCopy9(const char* a, const char* b, bool f)
{
    g_seq9.push_back(std::string("copy:") + (a ? a : "") + " -> " + (b ? b : ""));
    if (a && b && InSandbox9(a) && InSandbox9(b)) return ::CopyFileA(a, b, f ? TRUE : FALSE) != FALSE;
    return true;
}
static int Find9(const std::string& prefix)
{
    for (int i = 0; i < (int)g_seq9.size(); ++i)
        if (g_seq9[i].compare(0, prefix.size(), prefix) == 0) return i;
    return -1;
}
static int g_dfsHookCalls = 0;
static bool CountingDfsHook(const AnsiString& s, bool b) { ++g_dfsHookCalls; return W906_St02_LotInfo_DownloadFromServer(s, b); }

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("LI9_FtpClient\n");

    // ---- 0. containment first ----
    if (!W906TestInsideCtestRoots("LI9_FtpClient"))
        return 2;
    {
        const std::string d = W906TestSafeLower(DataPath.c_str()), o = W906TestSafeLower(OffsetPath.c_str()), a = W906TestSafeLower(AuthPath.c_str());
        const std::string e = W906TestSafeLower(std::getenv("W906_INIDATA_ROOT"));
        if (e.find("machine_log_scratch") == std::string::npos || d.find("machine_log_scratch") == std::string::npos ||
            o.find("machine_log_scratch") == std::string::npos || a.find("machine_config_scratch") == std::string::npos)
        {
            std::printf("  DataPath = %s\n  OffsetPath = %s\n  AuthPath = %s\n  ABORT: IniData / config are not ctest's scratch -- nothing was called\n",
                        DataPath.c_str(), OffsetPath.c_str(), AuthPath.c_str());
            return 2;
        }
    }
    const FileMark realCfg0 = FileMarkOf("D:\\HT9045\\config\\config.ini");
    const FileMark realInf0 = FileMarkOf("D:\\HT9045\\SetUp.inf");

    // ---- the sandbox ----
    char tick[32];
    std::snprintf(tick, sizeof(tick), "%lu", (unsigned long)GetTickCount());
    const std::string root = std::string(as9045LogPath.c_str()) + "\\LI9_FtpClient_" + tick + "\\";
    const AnsiString oldData = DataPath, oldOffset = OffsetPath, oldAuth = AuthPath;
    const AnsiString oldListFile = IniConfig.N06_TasterListFile, oldMap = IniConfig.N06_TasterListMap;
    const AnsiString oldTType = IniConfig.TasterType, oldTNo = IniConfig.TasterNo, oldTName = IniConfig.TasterName;
    const AnsiString oldMType = IniConfig.sMachineType, oldHID = IniConfig.SocketHandlerID;
    const int oldTIM = IniConfig.TasterInputMethod, oldSE = IniConfig.iServerEnable, oldHE = IniConfig.iHDEnable;
    const bool oldSPIL = IniConfig.bSPILFunction, oldFTPF = CosFunction.bFTPFunction;
    const int oldCC = CUSTOMER_CODE, oldAL = AccessLevel;
    const bool oldSS = SystemStart;
    const AnsiString oldRecipe = fMain->cbSetupFileName->Text;

    DataPath = AnsiString((root + "Data\\").c_str());
    OffsetPath = AnsiString((root + "Offset\\").c_str());
    AuthPath = AnsiString((root + "config\\").c_str());
    MyForceDirectories(AnsiString((root + "Data\\AAA_01").c_str()));
    MyForceDirectories(AnsiString((root + "Data\\BBB_02").c_str()));
    MyForceDirectories(AnsiString((root + "Data\\BBB_02X").c_str()));
    MyForceDirectories(OffsetPath);
    MyForceDirectories(AuthPath);
    MyForceDirectories(AnsiString((root + "taster").c_str()));
    IniConfig.N06_TasterListFile = AnsiString((root + "taster\\TasterList.csv").c_str());
    IniConfig.N06_TasterListMap  = AnsiString((root + "taster\\map\\TasterMap.csv").c_str());
    std::printf("  sandbox = %s\n", root.c_str());
    CUSTOMER_CODE = CC_HONPREC_QC;
    AccessLevel = 3;
    SystemStart = false;
    IniConfig.iServerEnable = 1;
    IniConfig.iHDEnable = 1;
    IniConfig.bSPILFunction = false;
    CosFunction.bFTPFunction = true;                                            // SaveTasterInfo writes only when this is on (cprod.cpp:3416)
    fMain->cbSetupFileName->Items->Clear();
    fMain->cbSetupFileName->Text = "OLD_RECIPE";

    // ---- 1. dispatcher ----
    std::printf("[1] dispatcher\n");
    std::string r = Act("state", "{}");
    CHECK(Has(r, "\"executed\":false") && Has(r, "\"guard\":\"not-installed\""), "1. nothing installed -> not-installed");
    r = ht9045::sjson::W906_LotInfoFtpAct("act.lotInfoFtp.", "{}");
    CHECK(Has(r, "\"guard\":\"bad-op\""), "1. empty op -> bad-op");
    r = ht9045::sjson::W906_LotInfoFtpAct("act.other.x", "{}");
    CHECK(Has(r, "\"guard\":\"bad-op\""), "1. another prefix -> bad-op");

    // ---- 2. defaults + install ----
    std::printf("[2] dfm defaults + install\n");
    W906_St02_LotInfoFtpDfmDefaults();
    CHECK(fLotInfo->btnFtpServer->Caption == "Server" && fLotInfo->btnFtpServer->Tag == 0 && fLotInfo->btnFtpServer->Enabled,
          "2. btnFtpServer: 'Server', Tag 0, enabled (uLotInfo.dfm:1830)");
    CHECK(fLotInfo->btnFtpHD->Tag == 1 && fLotInfo->btnFtpTester->Tag == 2 && fLotInfo->btnDataFTPSaveToData->Tag == 3,
          "2. Tags 1 / 2 / 3");
    CHECK(fLotInfo->btnDataFTPSaveToData->Visible == false && fLotInfo->btnFTPTryConnect->Visible == false,
          "2. DataFTP Save to Data hidden (non-TSMC, golden FormShow :566-581); Connection test hidden (dfm)");
    W906_St02_LotInfoFtpRegisterBody();
    W906_St02_Ftp.ChangeSetUpFile = &FakeChangeSetUpFile;
    W906_St02_Ftp.RecipeChainsReady = &FakeChainsReady;
    W906_St02_Ftp.LocalIp = &FakeLocalIp;
    r = Act("state", "{}");
    {
        Reply y(r);
        const cJSON* b = y.At("state.lotInfo.buttons");
        CHECK(y.B("executed") && !y.B("state.open") && b != 0 && cJSON_GetArraySize(b) == 5,
              "2. installed: state answers (the five Lot Info FTP buttons), the dialog is closed");
    }

    // ---- 3. guards ----
    std::printf("[3] btnFtpServerClick guards\n");
    fLotInfo->tsFTP->TabVisible = false;
    r = Act("open", "{\"tag\":1}");
    CHECK(Has(r, "\"guard\":\"tab-hidden\"") && !fFTPClient->bShow, "3. tsFTP hidden -> tab-hidden");
    fLotInfo->tsFTP->TabVisible = true;
    fLotInfo->btnFtpHD->Enabled = false;
    r = Act("open", "{\"tag\":1}");
    CHECK(Has(r, "\"guard\":\"button-disabled\"") && !fFTPClient->bShow, "3. a disabled button -> button-disabled");
    fLotInfo->btnFtpHD->Enabled = true;
    IniConfig.iHDEnable = 5;
    r = Act("open", "{\"tag\":1}");
    CHECK(Has(r, "\"guard\":\"access-level\"") && fLotInfo->btnFtpHD->Enabled && !fFTPClient->bShow,
          "3. iHDEnable > AccessLevel -> access-level, the button is enabled again (golden :5062)");
    IniConfig.iHDEnable = 1;
    SystemStart = true;
    r = Act("open", "{\"tag\":1}");
    CHECK(Has(r, "\"guard\":\"system-start\"") && fLotInfo->btnFtpHD->Enabled && !fFTPClient->bShow, "3. SystemStart -> system-start (golden :5103)");
    SystemStart = false;
    r = Act("open", "{\"tag\":7}");
    CHECK(Has(r, "\"guard\":\"bad-tag\""), "3. tag 7 -> bad-tag");
    fLotInfo->btnDataFTPSaveToData->Enabled = true;
    r = Act("open", "{\"tag\":3}");
    CHECK(Has(r, "\"guard\":\"tag3-return\"") && fLotInfo->btnDataFTPSaveToData->Enabled && !fFTPClient->bShow,
          "3. tag 3, not TSMC: golden only returns (:5091-5092)");
    IniConfig.bSPILFunction = true;
    r = Act("open", "{\"tag\":2}");
    CHECK(Has(r, "\"guard\":\"spil\"") && fLotInfo->btnFtpTester->Enabled == false, "3. tag 2 + bSPILFunction: returns, the button stays disabled (golden quirk)");
    IniConfig.bSPILFunction = false;
    fLotInfo->btnFtpTester->Enabled = true;
    // POOL-14 MR-A (AI(W906-P14) 20261010 (St02-E)): tag 0 now runs golden 913 ShowFTPModal case 0; no prepare hook here, so the
    //   engine stays in SIM with no fake server.
    const AnsiString oPort3 = IniConfig.N06_FtpPort;
    IniConfig.N06_FtpPort = "";                                                 // golden StrToInt("") raises (VCL EConvertError, vclcompat runtime_error)
    int memo0 = fFTPClient->memoFTP->Lines->Count;
    r = Act("open", "{\"tag\":0}");
    {
        Reply y(r);
        CHECK(y.S("guard") == "server-list-failed" && y.Msg("FTP Server is disconnect") && fFTPClient->memoFTP->Lines->Count == memo0 + 2 &&
              NMFTP3 == NULL && !y.B("state.open"),
              "3. tag 0, empty [FTP] Port: StrToInt raises -> golden 913 :1440-1449 'FTP Server is disconnect', NMFTP3 deleted + NULL, Close()");
    }
    IniConfig.N06_FtpPort = "21";
    memo0 = fFTPClient->memoFTP->Lines->Count;
    r = Act("open", "{\"tag\":0}");
    {
        Reply y(r);
        CHECK(y.S("guard") == "server-list-failed" && !y.B("state.open") && !fFTPClient->bShow && fLotInfo->btnFtpServer->Enabled,
              "3. tag 0 = Server, no server: server-list-failed, not open, the button enabled again");
        CHECK(fFTPClient->memoFTP->Lines->Count == memo0 + 3 && W906_St02_FtpTmpList() == 0 && NMFTP3 == NULL,
              "3. tag 0: golden 913 :1319 'Connecting' line, :1335-1343 not connected (NMFTP3 deleted + NULL), :1608-1612 bShow=false; Close()");
        CHECK(y.Msg("FTP Server is not connected") && !y.B("opened"), "3. tag 0: golden's box \"FTP Server is not connected\" (913 :1338)");
    }
    IniConfig.N06_FtpPort = oPort3;

    // ---- 4. HD ----
    std::printf("[4] HD page\n");
    r = Act("open", "{\"tag\":1}");
    {
        Reply y(r);
        CHECK(y.B("executed") && y.B("opened") && y.B("state.open") && y.S("state.page") == "hd", "4. tag 1 opens the HD page");
        CHECK(Join(y.L("state.hd.lstHDFile.items")) == "AAA_01|BBB_02|BBB_02X", "4. empty cbSetupFileName -> DataPath's folders (read only)");
        CHECK(Has(r, "read only") && fMain->cbSetupFileName->Items->Count == 0, "4. the fallback is reported, fMain's combo untouched");
        CHECK(fLotInfo->btnFtpHD->Enabled == false, "4. while open: btnFtpHD stays disabled (golden :5049; :5117 runs when it closes)");
    }
    r = Act("open", "{\"tag\":2}");
    {
        Reply y(r);
        CHECK(y.S("guard") == "already-open" && y.Msg("FTP Form already Opened!!") && fFTPClient->bShow && y.B("state.open"),
              "4. second press: FTP Form already Opened!!, the open dialog stays open (W906: :5110 skipped)");
        CHECK(fLotInfo->btnFtpTester->Enabled, "4. second press: its own button is enabled again (:5117)");
    }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"aaa_01\"}");
    { Reply y(r); CHECK(Join(y.L("state.hd.lstHDFile.items")) == "AAA_01", "4. FilterList: exact, case-insensitive (golden :2108)"); }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"BBB\"}");
    { Reply y(r); CHECK(y.L("state.hd.lstHDFile.items").empty(), "4. FilterList: a prefix is not enough (Sam 20190925)"); }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"\"}");
    { Reply y(r); CHECK(y.L("state.hd.lstHDFile.items").size() == 3, "4. empty text -> everything"); }
    r = Act("filter", "{\"edit\":\"server\",\"text\":\"x\"}");
    CHECK(Has(r, "\"guard\":\"wrong-page\""), "4. the Server edit on the HD page -> wrong-page (POOL-14 MR-A)");
    r = Act("pick", "{\"list\":\"hd\",\"index\":1,\"name\":\"WRONG\"}");
    CHECK(Has(r, "\"guard\":\"stale-list\""), "4. pick with a name that is not that item -> stale-list");
    r = Act("pick", "{\"list\":\"hd\",\"index\":1,\"name\":\"BBB_02\"}");
    {
        Reply y(r);
        CHECK(y.S("state.hd.edtHDWaferName.text") == "BBB_02" && Join(y.L("state.hd.lstHDFile.items")) == "BBB_02",
              "4. double click: the name goes to Setup File, OnChange filters to it (golden :2144)");
    }
    g_changes.clear(); g_fakeMode = 0;
    r = Act("loadHD", "{}");
    {
        Reply y(r);
        CHECK(g_changes.size() == 1 && g_changes[0] == "BBB_02", "4. Load from HD: ChangeSetUpFile(BBB_02) (golden :1643)");
        CHECK(fMain->cbSetupFileName->Text == "BBB_02" && !y.B("state.open") && !fFTPClient->bShow && fLotInfo->btnFtpHD->Enabled,
              "4. Load from HD: combo text, the dialog closed, btnFtpServerClick's tail ran (:5110 / :5117)");
        CHECK(y.I("state.lastLoadMode") == 0 && W906_St02_FtpTmpList() == 0, "4. mode 0; FormClose freed tmpList");
    }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"x\"}");
    CHECK(Has(r, "\"guard\":\"not-open\""), "4. after close: ops answer not-open");

    fMain->cbSetupFileName->Items->Add("X1");
    fMain->cbSetupFileName->Items->Add("X2");
    r = Act("open", "{\"tag\":1}");
    { Reply y(r); CHECK(Join(y.L("state.hd.lstHDFile.items")) == "X1|X2", "4. a filled cbSetupFileName -> its Items (golden :1473-1477)"); }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"zzz\"}");
    g_changes.clear();
    r = Act("loadHD", "{}");
    {
        Reply y(r);
        CHECK(y.Msg("未選擇這路徑") && y.B("state.open") && g_changes.empty(), "4. nothing left in the list: 未選擇這路徑, stays open (golden :1587)");
    }
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"X2\"}");
    g_fakeMode = 1;
    r = Act("loadHD", "{}");
    {
        Reply y(r);
        CHECK(g_changes.size() == 1 && y.I("state.lastLoadMode") == 1 && fMain->cbSetupFileName->Text == "X2" && !y.B("state.open"),
              "4. mode 1: golden :1646 then :1648 -- the combo still shows the requested name (golden quirk), closed");
    }
    g_fakeMode = 0;
    r = Act("open", "{\"tag\":1}");
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"X1\"}");
    g_changes.clear();
    r = Act("enterHD", "{}");
    CHECK(g_changes.size() == 1 && g_changes[0] == "X1" && !fFTPClient->bShow, "4. Enter in Setup File -> plLoadClick (golden :2217)");
    r = Act("open", "{\"tag\":1}");
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"X1\"}");
    g_chainsReady = false; g_changes.clear();
    r = Act("loadHD", "{}");
    CHECK(g_changes.empty() && Has(r, "boot-read-chain-not-run") && fMain->cbSetupFileName->Text == "X1",
          "4. boot read chain not run: ChangeSetUpFile not called, reported, treated as mode 1");
    g_chainsReady = true;
    r = Act("open", "{\"tag\":1}");
    r = Act("filter", "{\"edit\":\"hd\",\"text\":\"X2\"}");
    W906_St02_Ftp.ChangeSetUpFile = 0;
    r = Act("loadHD", "{}");
    CHECK(Has(r, "not installed") && !fFTPClient->bShow, "4. no ChangeSetUpFile hook: reported as not installed (mode 1)");
    W906_St02_Ftp.ChangeSetUpFile = &FakeChangeSetUpFile;
    r = Act("open", "{\"tag\":1}");
    r = Act("close", "{}");
    CHECK(Has(r, "\"executed\":true") && !fFTPClient->bShow && fLotInfo->btnFtpHD->Enabled, "4. close box: Close -> FormClose -> the tail");
    r = Act("nope", "{}");
    CHECK(Has(r, "\"guard\":\"unknown-op\""), "4. unknown op");
    r = Act("state", "not json");
    CHECK(Has(r, "\"guard\":\"bad-payload\""), "4. a value that is not a JSON object -> bad-payload");

    // ---- 5. Tester ----
    std::printf("[5] Taster page\n");
    WriteAll(IniConfig.N06_TasterListFile.c_str(), "Type,ID,IP\r\nT93K,01,10.1.1.1\r\nT93K,02,10.1.1.2\r\nJ750,07,10.1.2.7\r\n");
    IniConfig.TasterType = "T93K"; IniConfig.TasterNo = "02"; IniConfig.TasterName = "T93K-02"; IniConfig.TasterInputMethod = 0;
    IniConfig.sMachineType = "HT9045W2"; IniConfig.SocketHandlerID = "H-77";
    r = Act("open", "{\"tag\":2}");
    {
        Reply y(r);
        CHECK(y.B("state.open") && y.S("state.page") == "tester", "5. tag 2 opens the Taster page");
        CHECK(Join(y.L("state.tester.cbTesterType.items")) == "T93K|J750" && y.S("state.tester.cbTesterType.text") == "T93K",
              "5. GetTesterType: the types, the saved one selected");
        CHECK(Join(y.L("state.tester.cbTesterID.items")) == "01|02" && y.S("state.tester.cbTesterID.text") == "02" &&
              y.S("state.tester.cbTasterIp.text") == "10.1.1.2" && y.S("state.tester.edTesterName.text") == "T93K-02",
              "5. cbTesterTypeChange + cbTesterIDChange: IDs, IP, the name");
        CHECK(y.S("state.tester.edHandlerType.text") == "HT9045W2" && y.S("state.tester.edHandlerID.text") == "H-77", "5. Handler type / ID");
    }
    r = Act("testerType", "{\"index\":1}");
    {
        Reply y(r);
        CHECK(Join(y.L("state.tester.cbTesterID.items")) == "07" && y.S("state.tester.cbTesterID.text") == "" &&
              y.S("state.tester.edTesterName.text") == "J750-", "5. pick J750: its IDs, ID text cleared, name J750-");
    }
    r = Act("testerId", "{\"index\":0}");
    { Reply y(r); CHECK(y.S("state.tester.edTesterName.text") == "J750-07" && y.S("state.tester.cbTasterIp.text") == "10.1.2.7", "5. pick 07: J750-07, its IP"); }
    r = Act("testerName", "{\"text\":\"x\"}");
    CHECK(Has(r, "\"guard\":\"input-method\""), "5. By List: Taster Name is not editable");
    r = Act("saveTester", "{}");
    {
        Reply y(r);
        const std::string map = ReadAll(IniConfig.N06_TasterListMap.c_str());
        CHECK(map == "Handler Name, Handler Address, Tester Name, Tester Address\r\nH-77,10.0.0.7,J750-07,10.1.2.7\r\n",
              "5. Save: the Taster map in the sandbox (golden :1916-1938)");
        CHECK(IniConfig.TasterName == "J750-07" && IniConfig.TasterType == "J750" && IniConfig.TasterNo == "07" && !y.B("state.open"),
              "5. Save: IniConfig.Taster*, a good name closes the dialog (:1951)");
        const std::string cfg = ReadAll(std::string(AuthPath.c_str()) + "config.ini");
        CHECK(cfg.find("J750-07") != std::string::npos, "5. SaveTasterInfo wrote the sandbox config.ini (AuthPath)");
    }
    r = Act("open", "{\"tag\":2}");
    r = Act("inputMethod", "{\"index\":1}");
    {
        Reply y(r);
        CHECK(y.B("state.tester.grpTesterName.enabled") && !y.B("state.tester.grpTesterMap.visible") &&
              y.S("state.tester.edTesterName.text") == "J750-07", "5. Manually: the name from config.ini (golden :2082)");
    }
    r = Act("saveTester", "{\"edTesterName\":\"BOGUS\"}");
    {
        Reply y(r);
        CHECK(y.Msg("Tester Name 錯誤") && IniConfig.TasterName == "" && y.B("state.open"),
              "5. a name without '-': Tester Name 錯誤, saved empty, the dialog stays open (golden :1909-1951)");
        CHECK(ReadAll(IniConfig.N06_TasterListMap.c_str()).find("H-77,10.0.0.7,,") != std::string::npos, "5. the map is written anyway (golden)");
    }
    r = Act("exit", "{}");
    CHECK(Has(r, "\"executed\":true") && !fFTPClient->bShow, "5. Exit (Button3Click) closes");
    r = Act("open", "{\"tag\":2}");
    r = Act("inputMethod", "{\"index\":0}");
    const AnsiString keepList = IniConfig.N06_TasterListFile;
    IniConfig.N06_TasterListFile = AnsiString((root + "taster\\missing.csv").c_str());
    r = Act("testerType", "{\"text\":\"T93K\"}");
    CHECK(Has(r, "\"guard\":\"vcl-exception\"") && Has(r, "Cannot open file"), "5. list file missing: VCL EFOpenError (golden :1837)");
    IniConfig.N06_TasterListFile = keepList;
#ifdef SOFT_SIMULTE
    {
        const char* sn = std::getenv("W906_SIM_NET_PATHS");
        if (sn == 0 || std::strcmp(sn, "1") != 0)
        {
            const AnsiString keepMap = IniConfig.N06_TasterListMap;
            IniConfig.N06_TasterListMap = "\\\\li9-no-such-host\\share\\TasterMap.csv";   // never opened: W58 Q5 decides on the string
            r = Act("saveTester", "{}");
            CHECK(Has(r, "W58 Q5") && !fFTPClient->bShow, "5. SIM: a network Taster map is skipped (W58 Q5); the rest of Save ran");
            IniConfig.N06_TasterListMap = keepMap;
        }
        else std::printf("  (W906_SIM_NET_PATHS=1: the network-map case is not run)\n");
    }
#endif
    if (fFTPClient->bShow) Act("close", "{}");

    // ---- 6. memo ----
    std::printf("[6] memo\n");
    r = Act("open", "{\"tag\":1}");
    r = Act("memoClear", "{}");
    { Reply y(r); CHECK(y.L("state.memo").empty(), "6. memoFTP double click clears it (golden :2064)"); }
    Act("close", "{}");

    // ---- 7. Upload to Server (F2-3, AI(W906-W202) 20261009 (St02-E)) ----
    //   golden 913 plUnloadClick (KYECFTP/FTPClientUpload_St02.cpp) -> the real UploadFileToServer2 (ht9045_kyecftp) against a FAKE
    //   FTP server (the prepare hook installs it on the engine golden creates; the engine stays in SIM, no socket), the del / 7z /
    //   CopyFile lines recorded by the F2-2 exec hooks (a "7z a" writes a small file so the STOR has data), [N06] Copy Tester File
    //   off (Gate #4 itself is St02_W202FtpSeams').  golden's SetCurrentDirectory("D://") runs; the cwd is restored.
    std::printf("[7] Upload to Server (F2-3)\n");
    {
        char cwd7[MAX_PATH] = { 0 };
        ::GetCurrentDirectoryA(MAX_PATH, cwd7);
        const AnsiString oFH = IniConfig.FtpHost, oFU = IniConfig.FtpUserName, oFP = IniConfig.FtpPassword, oPort = IniConfig.N06_FtpPort;
        const AnsiString oUp = IniConfig.FtpUplaodPath, oATCDir = asATCFileTransferPath, oDef = DefaultPath;
        const int oMode = IniConfig.FtpTransMode, oSys = ATC_SYSTEM, oAMDf = iAMD_Function;
        const bool oN06 = IniConfig.bN06_CopyTesterFile, oATC = CosFunction.bUseATCFileTransfer, oPE = bHasEnteredPEModel;
        const bool oPEen = bEnablePEModel, oAMD = IniConfig.bAMDFunction, oSig = bSigurdUpload_Recipe, oPID = bPIDTransferErr;
        IniConfig.FtpHost = "li9-fake-ftp"; IniConfig.FtpUserName = "li9"; IniConfig.FtpPassword = "li9"; IniConfig.N06_FtpPort = "21";
        IniConfig.FtpUplaodPath = "/up/"; IniConfig.FtpTransMode = 1; IniConfig.bN06_CopyTesterFile = false;
        DefaultPath = AnsiString((root + "Default\\").c_str());
        MyForceDirectories(DefaultPath);
        MyForceDirectories(AnsiString((root + "Offset\\AAA_01").c_str()));
        WriteAll(root + "Data\\AAA_01\\Recipe.Data", "li9 recipe");
        CosFunction.bUseATCFileTransfer = false; bHasEnteredPEModel = false; IniConfig.bAMDFunction = false; bSigurdUpload_Recipe = false;
        W906_KyecFtpPrepareHook = &Prepare7;
        W906_KyecFtpSystemHook = &FakeSystem7;
        W906_KyecFtpCopyFileHook = &FakeCopy7;
        FTPClientTransfer_SetFastDelayForTest(true);

        r = Act("upload", "{}");
        CHECK(Has(r, "\"guard\":\"not-open\""), "7. upload with the dialog closed -> not-open");
        r = Act("open", "{\"tag\":2}");
        r = Act("upload", "{}");
        CHECK(Has(r, "\"guard\":\"wrong-page\""), "7. upload on the Taster page -> wrong-page");
        Act("close", "{}");

        fMain->cbSetupFileName->Items->Clear();                                 // the HD list = DataPath's folders (section 4's fallback)
        r = Act("open", "{\"tag\":1}");
        r = Act("filter", "{\"edit\":\"hd\",\"text\":\"AAA_01\"}");
        CHECK(Join(Reply(r).L("state.hd.lstHDFile.items")) == "AAA_01", "7. the HD list filtered to AAA_01");
        g_seq7.clear(); g_srv7.stor.clear();
        r = Act("upload", "{}");
        {
            Reply y(r);
            std::printf("  %s\n", Join(g_seq7).c_str());
            CHECK(y.B("executed") && y.Msg("Upload done") && !y.Msg("not connected"),
                  "7. HD AAA_01 -> upload runs to golden's \"Upload done\" (golden 913 :1225 -> UploadFileToServer2 :802-810)");
            CHECK(g_srv7.stor.size() == 2 && g_srv7.stor[0] == "/up/AAA_01.zip" && g_srv7.stor[1] == "/up/AAA_01.Offset",
                  "7. the fake server got STOR /up/AAA_01.zip then /up/AAA_01.Offset ([FTP] FTP Upload Path + the recipe, sRootPath empty)");
            CHECK(Find7("sys:del ") >= 0 && Find7("sys:d:\\HT9045\\7z.exe a -tzip") > Find7("sys:del ") && Find7("ctrl:USER li9") >= 0,
                  "7. connect (USER), the pre-delete, then the 7z zips -- all through the exec hooks, nothing ran");
            CHECK(y.B("state.hd.plUnload.enabled") && y.S("state.hd.edtHDWaferName.text").empty() && y.B("state.open"),
                  "7. the tail: plUnload enabled again, edtHDWaferName cleared, the dialog stays open (golden :1227-1228)");
            CHECK(HasMd5(root + "Data\\AAA_01"), "7. SetMD5ByFolder wrote the recipe's .MD5 in the sandbox (golden :1173)");
        }

        g_serve7 = false; g_seq7.clear(); g_srv7.stor.clear();
        r = Act("filter", "{\"edit\":\"hd\",\"text\":\"AAA_01\"}");
        r = Act("upload", "{}");
        {
            Reply y(r);
            CHECK(y.B("executed") && y.Msg("FTP Server is not connected") && g_srv7.stor.empty() && y.B("state.hd.plUnload.enabled"),
                  "7. no server: golden \"FTP Server is not connected\", nothing stored, plUnload enabled again");
        }
        g_serve7 = true;

        r = Act("filter", "{\"edit\":\"hd\",\"text\":\"AAA\"}");
        g_srv7.stor.clear();
        r = Act("upload", "{}");
        CHECK(Has(r, "\"executed\":true") && Reply(r).Msg("未選擇這路徑") && g_srv7.stor.empty(), "7. no exact match selected -> 未選擇這路徑 (golden 913 :1153-1159)");

        r = Act("filter", "{\"edit\":\"hd\",\"text\":\"AAA_01\"}");
        bHasEnteredPEModel = true; bEnablePEModel = true;
        r = Act("upload", "{}");
        CHECK(Reply(r).Msg("PE engineering model can not upload files") && g_srv7.stor.empty(), "7. PE engineering model -> refused message (golden 913 :1144-1150)");
        bHasEnteredPEModel = false; bEnablePEModel = oPEen;

        IniConfig.bAMDFunction = true; iAMD_Function = 1;
        r = Act("upload", "{}");
        CHECK(Has(r, "DoCheckPassword") && g_srv7.stor.empty() && Reply(r).B("state.hd.plUnload.enabled"),
              "7. AMD group: DoCheckPassword is not in the port -> gated refusal, nothing uploaded (D-6 A)");
        IniConfig.bAMDFunction = oAMD; iAMD_Function = oAMDf;

        SystemStart = true;
        r = Act("upload", "{}");
        CHECK(Has(r, "\"guard\":\"running\"") && g_srv7.stor.empty(), "7. SystemStart -> running (port-only: golden's dialog is modal)");
        SystemStart = false;

        fFTPClient->plUnload->Enabled = false;
        r = Act("upload", "{}");
        CHECK(Has(r, "\"guard\":\"button-hidden\""), "7. plUnload disabled -> button-hidden");
        fFTPClient->plUnload->Enabled = true;

        // KYEC ATC file transfer, ATC not connected (the port has no ATC link): golden 913 :1210-1221
        const std::string atcDir = root + "ATCSaveFile\\";
        MyForceDirectories(AnsiString(atcDir.c_str()));
        WriteAll(atcDir + "old.pid", "x");
        asATCFileTransferPath = AnsiString(atcDir.c_str());
        CosFunction.bUseATCFileTransfer = true; ATC_SYSTEM = eNewATCSystem; bPIDTransferErr = false;
        g_srv7.stor.clear();
        r = Act("upload", "{}");
        {
            Reply y(r);
            const std::vector<std::string> memo = y.L("state.memo");
            const bool memoLine = !memo.empty() && memo.back().find("ATC System Connect Error, PID FileTransfer Fail!!") != std::string::npos;
            CHECK(memoLine && bPIDTransferErr && FileMarkOf((atcDir + "old.pid").c_str()).exists == false &&
                  FileMarkOf(atcDir.c_str()).exists && g_srv7.stor.size() == 2,
                  "7. KYEC ATC arm, not connected: memo line, the transfer folder emptied and made again, bPIDTransferErr, then the upload (golden 913 :1212-1220)");
        }
        CosFunction.bUseATCFileTransfer = oATC; ATC_SYSTEM = oSys; asATCFileTransferPath = oATCDir; bPIDTransferErr = oPID;
        Act("close", "{}");

        W906_KyecFtpPrepareHook = 0; W906_KyecFtpSystemHook = 0; W906_KyecFtpCopyFileHook = 0;
        FTPClientTransfer_SetFastDelayForTest(false);
        IniConfig.FtpHost = oFH; IniConfig.FtpUserName = oFU; IniConfig.FtpPassword = oFP; IniConfig.N06_FtpPort = oPort;
        IniConfig.FtpUplaodPath = oUp; IniConfig.FtpTransMode = oMode; IniConfig.bN06_CopyTesterFile = oN06; DefaultPath = oDef;
        bHasEnteredPEModel = oPE; bSigurdUpload_Recipe = oSig;
        ::SetCurrentDirectoryA(cwd7);
    }

    // ---- 8. Server list (POOL-14 MR-A, AI(W906-P14) 20261010 (St02-E)) ----
    //   golden 913 ShowFTPModal case 0 (:1255-1460) against a FAKE FTP server (the F2-2 prepare hook; SIM, no socket), the NMFTP1*
    //   handlers writing into the dialog through the same sink wb_serve installs (WebLotInfoFtpInstall_St02.cpp
    //   W906_St02_InstallKyecFtpEngine, not linked here); the Server page ops; FormClose's connection close (:1708-1740).
    std::printf("[8] Server list (POOL-14 MR-A)\n");
    {
        const AnsiString oFH = IniConfig.FtpHost, oFU = IniConfig.FtpUserName, oFP = IniConfig.FtpPassword, oPort = IniConfig.N06_FtpPort;
        const AnsiString oDown = IniConfig.FtpDownloadPath, oRecipe8 = fMain->cbSetupFileName->Text;
        const bool oBR = CosFunction.bFTPUseBarcodeReader, oUB = IniConfig.bN06_UseBarcode, oSigD = bSigurdDownload_Recipe;
        IniConfig.FtpHost = "li9-fake-ftp"; IniConfig.FtpUserName = "li9"; IniConfig.FtpPassword = "li9"; IniConfig.N06_FtpPort = "21";
        IniConfig.FtpDownloadPath = "/dl/"; CosFunction.bFTPUseBarcodeReader = false; bSigurdDownload_Recipe = false;
        FTPClientEvt_LogSink sink;
        sink.AddMemoLine       = [](const AnsiString& s) { fFTPClient->memoFTP->Lines->Add(s); };
        sink.AddServerFileItem = [](const AnsiString& s) { fFTPClient->lstServerFile->Items->Add(s); };
        sink.AddListBox1Item   = [](const AnsiString& s) { fFTPClient->ListBox1->Items->Add(s); };
        SetFTPClientEvtLogSink(sink);
        W906_KyecFtpPrepareHook = &Prepare8;
        auto memoHas = [](const Reply& y, const char* s) {
            const std::vector<std::string> m = y.L("state.memo");
            for (std::size_t i = 0; i < m.size(); ++i) if (m[i].find(s) != std::string::npos) return true;
            return false;
        };

        g_srv8.listing = { "a.zip", "b.Offset", "c_ATC_Recipe.zip", "d.txt", "e.Offset.zip" };
        g_seq8.clear();
        r = Act("open", "{\"tag\":0}");
        {
            Reply y(r);
            std::printf("  %s\n", Join(g_seq8).c_str());
            CHECK(y.B("opened") && y.B("state.open") && y.S("state.page") == "server",
                  "8. tag 0 with a server: the dialog opens on the Server page (golden 913 :1539-1606 -> ShowModal)");
            CHECK(Join(y.L("state.server.lstServerFile.items")) == "a|c_ATC_Recipe|e.Offset",
                  "8. NMFTP1ListItem: *.zip only, '.zip' cut; golden quirk kept: c_ATC_Recipe and e.Offset are listed too (913 :1853-1855)");
            CHECK(Find8("ctrl:USER li9") >= 0 && Find8("ctrl:CWD /dl/") > Find8("ctrl:USER li9") && Find8("ctrl:NLST") > Find8("ctrl:CWD /dl/"),
                  "8. connect as the [FTP] user, CWD the [FTP] Download Path, NLST (golden 913 :1302-1400)");
            CHECK(NMFTP3 != NULL && W906_St02_FtpTmpList() != 0 && W906_St02_FtpTmpList()->Count == 3,
                  "8. NMFTP3 stays open for the dialog; tmpList (kyecftp's one copy) = the 3 names");
            CHECK(memoHas(y, "Process -- Connecting to server, OK--------") && memoHas(y, "Success -- NList successful"),
                  "8. memoFTP: golden's own lines (913 :1345) and the NMFTP1Success handler's line, through the sink");
        }
        r = Act("filter", "{\"edit\":\"server\",\"text\":\"A\"}");
        CHECK(Join(Reply(r).L("state.server.lstServerFile.items")) == "a", "8. FilterList on the Server edit: exact, case-insensitive (913 :2143-2160)");
        r = Act("filter", "{\"edit\":\"server\",\"text\":\"\"}");
        CHECK(Reply(r).L("state.server.lstServerFile.items").size() == 3, "8. empty text -> all three again");
        r = Act("pick", "{\"list\":\"server\",\"index\":1,\"name\":\"c_ATC_Recipe\"}");
        {
            Reply y(r);
            CHECK(y.S("state.server.edtServerWaferName.text") == "c_ATC_Recipe" && Join(y.L("state.server.lstServerFile.items")) == "c_ATC_Recipe",
                  "8. lstServerFile double click -> the name in edtServerWaferName (913 :2172-2182), OnChange filters to it");
        }
        r = Act("pick", "{\"list\":\"server\",\"index\":0,\"name\":\"WRONG\"}");
        CHECK(Has(r, "\"guard\":\"stale-list\""), "8. pick with a name that is not that item -> stale-list");
        r = Act("cleanName", "{}");
        CHECK(Has(r, "\"guard\":\"button-hidden\""), "8. Clean File Name is hidden when not CC_TSMC_TAINAN (913 FormShow :1803) -> button-hidden");
        fFTPClient->Panel16->Visible = true;
        r = Act("cleanName", "{}");
        CHECK(Reply(r).S("state.server.edtServerWaferName.text").empty() && Reply(r).L("state.server.lstServerFile.items").size() == 3,
              "8. Clean File Name (shown) -> edtServerWaferName cleared (913 :2317-2320)");
        fFTPClient->Panel16->Visible = false;
        fMain->cbSetupFileName->Text = "a";
        r = Act("copyName", "{}");
        CHECK(Reply(r).S("state.server.edtServerWaferName.text") == "a" && Join(Reply(r).L("state.server.lstServerFile.items")) == "a",
              "8. Copy File Name -> the main recipe name in edtServerWaferName (913 :2311-2314)");
        CosFunction.bFTPUseBarcodeReader = true; IniConfig.bN06_UseBarcode = true;
        r = Act("filter", "{\"edit\":\"server\",\"text\":\"x\"}");
        CHECK(Has(r, "\"guard\":\"barcode-keys\""), "8. barcode reader mode: typed keys are swallowed (913 :2238-2241) -> barcode-keys");
        CosFunction.bFTPUseBarcodeReader = false; IniConfig.bN06_UseBarcode = oUB;
        g_seq8.clear();
        r = Act("close", "{}");
        {
            Reply y(r);
            std::printf("  %s\n", Join(g_seq8).c_str());
            CHECK(y.B("executed") && !y.B("state.open") && Find8("ctrl:ABOR") >= 0,
                  "8. close: FormClose aborts the open Server connection and deletes it (golden 913 :1709-1725)");
        }

        g_srv8.listing.clear(); g_seq8.clear();
        r = Act("open", "{\"tag\":0}");
        {
            Reply y(r);
            CHECK(y.S("guard") == "server-list-failed" && y.Msg("FTP Error -- List fail! Empty directory!") && !y.B("state.open") &&
                  Find8("ctrl:NLST") >= 0 && Find8("ctrl:ABOR") > Find8("ctrl:NLST"),
                  "8. empty directory: golden's box, bError, Close() -> FormClose closes the connection (913 :1433-1438, :1608-1612, :1709-1725)");
        }
        g_serve8 = false;
        r = Act("open", "{\"tag\":0}");
        CHECK(Has(r, "\"guard\":\"server-list-failed\"") && Reply(r).Msg("FTP Server is not connected") && NMFTP3 == NULL,
              "8. no server: \"FTP Server is not connected\", NMFTP3 deleted and NULL (913 :1335-1343)");
        g_serve8 = true;
        r = Act("open", "{\"tag\":1}");
        r = Act("copyName", "{}");
        CHECK(Has(r, "\"guard\":\"wrong-page\""), "8. Copy File Name on the HD page -> wrong-page");
        Act("close", "{}");

        W906_KyecFtpPrepareHook = 0;
        ResetFTPClientEvtLogSinkToSimDefault();
        IniConfig.FtpHost = oFH; IniConfig.FtpUserName = oFU; IniConfig.FtpPassword = oFP; IniConfig.N06_FtpPort = oPort;
        IniConfig.FtpDownloadPath = oDown; CosFunction.bFTPUseBarcodeReader = oBR; IniConfig.bN06_UseBarcode = oUB; bSigurdDownload_Recipe = oSigD;
        fMain->cbSetupFileName->Text = oRecipe8;
    }

    // ---- 9. DownloadFromServer (POOL-14 MR-B1, AI(W906-P14) 20261010 (St02-E)) ----
    //   golden 913 TfLotInfo::DownloadFromServer (uLotInfo.cpp:4308-4830) as forms/fLotInfo_Download_St02.cpp, called directly (no live
    //   caller until MR-C) in the sandbox: the fake 7z above, sFTPSetupFileLogPath pointed into the sandbox, golden's
    //   SetCurrentDirectory("D://") undone.  Backup / OverWrite are MR-B2 (named stubs here).  Gate #2's seam with and without a hook.
    std::printf("[9] DownloadFromServer (POOL-14 MR-B1)\n");
    {
        char cwd9[MAX_PATH] = { 0 };
        ::GetCurrentDirectoryA(MAX_PATH, cwd9);
        const AnsiString oLogPath = sFTPSetupFileLogPath;
        const bool oFTPF = CosFunction.bFTPFunction, oRms = IniConfig.bEnableRms, oSysUnzip = IniConfig.FtpUseSystemCallToUnZip;
        const bool oCover = CosFunction.bFTPDownloadAlwaysCover, oE45 = IniConfig.bE45_AllSetupFileUseOneFile, oLocalOfs = CosFunction.bUseLocalRecipeOffset;
        const bool oMd5 = IniConfig.bN20_CheckMD5, oAtcDl = CosFunction.bUseFTPDownLoadATCRecipe, oAtcFt = CosFunction.bUseATCFileTransfer;
        const bool oCH = CosFunction.bContactHeightSaveToContactIni, oSmart = CosFunction.bSmartAutoClean;
        const int oCC9 = CUSTOMER_CODE, oDL = IniConfig.iN05_UpDLMethod;
        sFTPSetupFileLogPath = AnsiString((root + "FTPSetupFileLog").c_str());
        CosFunction.bFTPFunction = true; IniConfig.bEnableRms = false; IniConfig.FtpUseSystemCallToUnZip = true;
        CosFunction.bFTPDownloadAlwaysCover = false; IniConfig.bE45_AllSetupFileUseOneFile = false; CosFunction.bUseLocalRecipeOffset = false;
        IniConfig.bN20_CheckMD5 = false; CosFunction.bUseFTPDownLoadATCRecipe = false; CosFunction.bUseATCFileTransfer = false;
        CosFunction.bContactHeightSaveToContactIni = false; CosFunction.bSmartAutoClean = false; CUSTOMER_CODE = 0;
        W906_KyecFtpSystemHook = &FakeSystem9; W906_KyecFtpShellExecHook = &FakeShell9; W906_KyecFtpCopyFileHook = &FakeCopy9;
        fMain->cbSetupFileName->Items->Clear();
        const std::string data = root + "Data\\";
        W906_St02_FtpReport rep9;
        W906_St02_FtpCurrentReport = &rep9;
        auto gatedHas = [&rep9](const char* s) {
            for (std::size_t i = 0; i < rep9.gated.size(); ++i) if (rep9.gated[i].find(s) != std::string::npos) return true;
            return false;
        };

        // (a) a new recipe NEW1, downloaded as NEW1_NET, unzipped with the system call
        WriteAll(data + "NEW1_NET.zip", "fake zip");
        g_seq9.clear(); W906_ShowErrorMessage_Reset(); rep9.gated.clear();
        bool ok = W906_St02_LotInfo_DownloadFromServer("NEW1_NET", true);
        std::printf("  %s\n", Join(g_seq9).c_str());
        CHECK(ok && FileMarkOf((data + "NEW1_NET\\Tray.Data").c_str()).exists && !FileMarkOf((data + "NEW1_NET.zip").c_str()).exists,
              "9a. new recipe: 7z (system call) unzips NEW1_NET.zip into Data\\NEW1_NET\\, the zip is deleted, true (golden 913 :4565-4574, :4684-4686, :4819)");
        CHECK(FileMarkOf((data + "NEW1").c_str()).exists && fMain->cbSetupFileName->Items->IndexOf("NEW1_NET") >= 0,
              "9a. Data\\NEW1 (no _NET) made, NEW1_NET added to the setup file list (913 :4390-4407)");
        CHECK(Find9("sys:d:\\HT9045\\7z.exe e \"" + data + "NEW1_NET.zip\" -o\"" + data + "NEW1_NET\\\" -y") >= 0,
              "9a. golden's 7z command line, through the exec seam (nothing ran)");
        CHECK(W906_ShowErrorMessage_LastCode == "MES1687" && !gatedHas("DoBackupSetupFile"),
              "9a. MES1687 'UnZip %s.zip OK.' (913 :4801); a new recipe is not backed up");
        CHECK(FileMarkOf((root + "FTPSetupFileLog").c_str()).exists, "9a. 'No backup [NEW1]' went to the FTP setup-file change log (913 :4558-4559, :14024-14033), in the sandbox");

        // (b) a recipe that already exists: Backup / OverWrite are MR-B2's (named stubs)
        MyForceDirectories(AnsiString((data + "OLD1").c_str()));
        WriteAll(data + "OLD1_NET.zip", "fake zip");
        rep9.gated.clear();
        W906_St02_LotInfo_iAutoClean[68] = "stale";
        ok = W906_St02_LotInfo_DownloadFromServer("OLD1_NET", true);
        CHECK(ok && W906_St02_LotInfo_iAutoClean[68] == "1" && !gatedHas("DoBackupSetupFile") && !gatedHas("DoOverWriteSetupFile"),
              "9b. existing recipe OLD1: DoBackupSetupFile ran (iAutoClean[68] = golden default \"1\", 913 :3883) and DoOverWriteSetupFile "
              "(913 :4552-4554, :4701-4703; MR-B2 -- no stub left)");

        // (c) no zip -> WAR1684, false
        W906_ShowErrorMessage_Reset();
        ok = W906_St02_LotInfo_DownloadFromServer("NOZIP_NET", true);
        CHECK(!ok && W906_ShowErrorMessage_LastCode == "WAR1684", "9c. no NOZIP_NET.zip: WAR1684 '下載 %s.zip 失敗', false (913 :4635-4639)");

        // (d) the unzip leaves Tray.Data out (8 files, one extra) -> WAR1686, the zip stays
        g_unzip9.back() = "Extra.Data";
        WriteAll(data + "MISS_NET.zip", "fake zip");
        W906_ShowErrorMessage_Reset();
        ok = W906_St02_LotInfo_DownloadFromServer("MISS_NET", true);
        CHECK(!ok && W906_ShowErrorMessage_LastCode == "WAR1686" && FileMarkOf((data + "MISS_NET.zip").c_str()).exists,
              "9d. a required file missing after the unzip: WAR1686 'UnZip %s.zip Fail.', the zip is kept, false (913 :4667-4679, :4821-4828)");
        g_unzip9.back() = "Tray.Data";

        // (e) ShellExecute instead of the system call; a ShellExecute that fails (<= 32)
        IniConfig.FtpUseSystemCallToUnZip = false;
        WriteAll(data + "SHL_NET.zip", "fake zip");
        g_seq9.clear();
        ok = W906_St02_LotInfo_DownloadFromServer("SHL_NET", true);
        CHECK(ok && Find9("shell:d:\\HT9045\\7z.exe e \"" + data + "SHL_NET.zip\" -o\"" + data + "SHL_NET\\\" -y") >= 0,
              "9e. FtpUseSystemCallToUnZip off: ShellExecute 7z (913 :4631-4632), > 32 -> true");
        g_shellRet9 = 2;
        WriteAll(data + "SHF_NET.zip", "fake zip");
        W906_ShowErrorMessage_Reset();
        ok = W906_St02_LotInfo_DownloadFromServer("SHF_NET", true);
        CHECK(!ok && W906_ShowErrorMessage_LastCode == "WAR1686", "9e. ShellExecute returns 2 (<= 32): WAR1686, false (913 :4677-4679)");
        g_shellRet9 = 42;
        IniConfig.FtpUseSystemCallToUnZip = true;

        // (f) RMS is not in the port: refused, named
        IniConfig.bEnableRms = true; IniConfig.iN05_UpDLMethod = eByFTP;
        WriteAll(data + "RMS_NET.zip", "fake zip");
        rep9.gated.clear(); g_seq9.clear();
        ok = W906_St02_LotInfo_DownloadFromServer("RMS_NET", true);
        CHECK(!ok && gatedHas("RMS download") && FileMarkOf((data + "RMS_NET.zip").c_str()).exists,
              "9f. IniConfig.bEnableRms: the RMS download (913 :4480-4543) is refused and named; nothing unzipped");
        IniConfig.bEnableRms = false; IniConfig.iN05_UpDLMethod = oDL;

        // (g) a customer body (S25): CC_KYEC_LEE's forced Auto Clean is gated, the download itself still succeeds
        CUSTOMER_CODE = CC_KYEC_LEE;
        WriteAll(data + "KY_NET.zip", "fake zip");
        rep9.gated.clear();
        ok = W906_St02_LotInfo_DownloadFromServer("KY_NET", true);
        CHECK(ok && gatedHas("CC_KYEC_LEE"), "9g. CC_KYEC_LEE: the forced Auto Clean write (913 :4743-4758) is gated and named (S25), true");
        CUSTOMER_CODE = 0;

        // (h) Gate #2 (FTPClient_Transfer.cpp :264 / :266): no hook = false (the old stand-in); a hook is called
        W906_KyecFtpDownloadFromServerHook = 0;
        CHECK(W906_KyecFtpDownloadFromServer("X_NET", true) == false, "9h. no hook installed: Gate #2 still answers false (nothing runs)");
        W906_KyecFtpDownloadFromServerHook = &CountingDfsHook;
        WriteAll(data + "HK_NET.zip", "fake zip");
        g_dfsHookCalls = 0;
        ok = W906_KyecFtpDownloadFromServer("HK_NET", true);
        CHECK(ok && g_dfsHookCalls == 1, "9h. a hook installed: Gate #2 calls it (wb_serve installs DownloadFromServer, WebLotInfoFtpInstall_St02.cpp)");

        // (i) the real caller of Gate #2: LoadFileFormServer2 (KYECFTP/FTPClient_Transfer.cpp) downloads LF1.zip x3 + LF1.Offset from a
        //     fake server, then fLotInfo->DownloadFromServer("LF1_NET") = FTPClient_Transfer.cpp :264-:267 -> the seam -> the hook
        {
            const AnsiString oFH9 = IniConfig.FtpHost, oFU9 = IniConfig.FtpUserName, oFP9 = IniConfig.FtpPassword, oPort9 = IniConfig.N06_FtpPort;
            const int oMode9 = IniConfig.FtpTransMode;
            IniConfig.FtpHost = "li9-fake-ftp"; IniConfig.FtpUserName = "li9"; IniConfig.FtpPassword = "li9"; IniConfig.N06_FtpPort = "21";
            IniConfig.FtpTransMode = 1;
            g_srv8.listing = { "PK-fake-zip-bytes" };
            W906_KyecFtpPrepareHook = &Prepare8;
            FTPClientTransfer_SetFastDelayForTest(true);
            g_seq8.clear(); g_dfsHookCalls = 0;
            LoadFileFormServer2("/dl/", "LF1");
            std::printf("  %s\n", Join(g_seq8).c_str());
            CHECK(Find8("ctrl:RETR LF1.zip") >= 0 && g_dfsHookCalls == 1 && FileMarkOf((data + "LF1_NET\\Tray.Data").c_str()).exists,
                  "9i. LoadFileFormServer2 downloads LF1.zip, then Gate #2 (FTPClient_Transfer.cpp :264-:267) calls DownloadFromServer(\"LF1_NET\"), which unzips it");
            FTPClientTransfer_SetFastDelayForTest(false);
            W906_KyecFtpPrepareHook = 0;
            IniConfig.FtpHost = oFH9; IniConfig.FtpUserName = oFU9; IniConfig.FtpPassword = oFP9; IniConfig.N06_FtpPort = oPort9;
            IniConfig.FtpTransMode = oMode9;
        }
        W906_KyecFtpDownloadFromServerHook = 0;

        // ---- [10] DoBackupSetupFile / DoOverWriteSetupFile (POOL-14 MR-B2, golden 913 uLotInfo.cpp:3630-4306) ----
        //   recipe OLD2 exists with local settings; OLD2_NET.zip unzips to plain files; [Network] of AuthPath Security_new.def says
        //   which items are NOT covered by the download (0 = keep the local value) -- all in the sandbox (AuthPath = <root>config\).
        std::printf("[10] Backup / OverWrite (POOL-14 MR-B2)\n");
        {
            const std::string o2 = data + "OLD2\\", n2 = data + "OLD2_NET\\", sec = std::string(AuthPath.c_str()) + "Security_new.def";
            MyForceDirectories(AnsiString(o2.c_str()));
            WriteIniData(AnsiString((o2 + "Temperature.Data").c_str()), "User OffSet", "CH1", 1.5);
            WriteIniData(AnsiString((o2 + "Contact.Data").c_str()), "Test Arm1", "Contact", 12.25);
            WriteIniData(AnsiString((o2 + "HandlerCondition.Data").c_str()), "Configuration", "Shuttle Mode", 1);
            WriteIniData(AnsiString((o2 + "HandlerCondition.Data").c_str()), "Configuration", "iAutoClean_IntervalContact", 33);
            WriteAll(o2 + "HotPlate.Data", "local hotplate");
            WriteAll(o2 + "ArmCondition.Data", "local arm");   // the ArmCondition backup (golden 913 :3776-3778; bcc32 runs :3778, RULINGS_20261010 #9)
            WriteIniData(AnsiString(sec.c_str()), "Network", "Temp Offset", false);   // default true (cover) -> keep the local CH1
            WriteIniData(AnsiString(sec.c_str()), "Network", "Auto Clean", false);    // default true -> keep the local Auto Clean values
            WriteAll(data + "OLD2_NET.zip", "fake zip");
            g_seq9.clear();
            ok = W906_St02_LotInfo_DownloadFromServer("OLD2_NET", true);
            CHECK(ok && fLotInfo->fTempUserOffset[0] == 1.5 && fLotInfo->fContactHeight[1] == 12.25 && fLotInfo->iShuttleMode[0] == 1 &&
                      W906_St02_LotInfo_iAutoClean[48] == "33",
                  "10a. DoBackupSetupFile read OLD2's User OffSet CH1 / Test Arm1 Contact / Shuttle Mode / iAutoClean_IntervalContact (913 :3645-3863)");
            CHECK(ReadIniData(AnsiString((n2 + "Temperature.Data").c_str()), "User OffSet", "CH1", 0.0) == 1.5 &&
                      ReadIniData(AnsiString((n2 + "Contact.Data").c_str()), "Test Arm1", "Contact", 0.0) == 12.25 &&
                      ReadIniData(AnsiString((n2 + "HandlerCondition.Data").c_str()), "Configuration", "Shuttle Mode", 0) == 1 &&
                      ReadIniData(AnsiString((n2 + "HandlerCondition.Data").c_str()), "Configuration", "iAutoClean_IntervalContact", 0) == 33,
                  "10b. DoOverWriteSetupFile wrote them back into the downloaded OLD2_NET (Temp Offset / Contact High / Shuttle Mode / Auto Clean "
                  "not covered; 913 :3930-3945, :3959-3994, :4082-4089, :4131-4232)");
            std::ifstream hp((n2 + "HotPlate.Data").c_str());
            std::string hpText((std::istreambuf_iterator<char>(hp)), std::istreambuf_iterator<char>());
            hp.close();
            const bool noCH = Find9(std::string("copy:") + o2 + "HotPlate.Data -> CH") < 0;
            const bool armBak = Find9(std::string("copy:") + o2 + "ArmCondition.Data -> " + data + "ArmCondition.Data") >= 0;
            CHECK(noCH && armBak && hpText == "local hotplate" && !FileMarkOf((data + "HotPlate.Data").c_str()).exists &&
                      !FileMarkOf((data + "ArmCondition.Data").c_str()).exists,
                  "10c. GCC splices, bcc32 does not (RULINGS_20261010 #9): the HotPlate backup goes to DataPath (no CH<n> copy in the cwd) and "
                  "comes back into OLD2_NET; ArmCondition.Data is backed up too; both DataPath copies deleted (913 :3679-3681, :3776-3778, :4028-4076)");
            WriteIniData(AnsiString(sec.c_str()), "Network", "Temp Offset", true);
            WriteIniData(AnsiString((o2 + "Temperature.Data").c_str()), "User OffSet", "CH1", 2.5);
            WriteAll(data + "OLD2_NET.zip", "fake zip");
            ok = W906_St02_LotInfo_DownloadFromServer("OLD2_NET", true);
            CHECK(ok && fLotInfo->fTempUserOffset[0] == 2.5 &&
                      ReadIniData(AnsiString((n2 + "Temperature.Data").c_str()), "User OffSet", "CH1", -1.0) == -1.0,
                  "10d. Temp Offset covered (1): backed up (2.5) but not written back -- the downloaded file (no CH1 in the fake zip) stays "
                  "(913 :3930-3931)");
        }

        // ---- [11] btSaveSetupFileClick / ClearAllSetupFile (POOL-14 MR-B3, golden 913 uLotInfo.cpp:5254-5349, :12231-12288) ----
        //   LastDataPath is pointed into the sandbox first (WriteLastDataFN writes it); ChangeSetUpFile = the test's fake (records);
        //   LookForFile = a counting fake (Q4); ATC is not the HonPrec type (the ATC re-connect at :5326 is not reached).
        std::printf("[11] btSaveSetupFileClick / ClearAllSetupFile (POOL-14 MR-B3)\n");
        {
            const AnsiString oLast = LastDataPath, oData11 = DataPath, oOffset11 = OffsetPath;
            // SHFileOperation (btSaveSetupFileClick's copy) only takes '\' paths; ctest's scratch root comes with '/' (a machine's DataPath is
            //   D:\HT9045\IniData\Data\), so the same sandbox folders are spelled with '\' here.
            auto bs = [](std::string p) { for (std::size_t i = 0; i < p.size(); ++i) if (p[i] == '/') p[i] = '\\'; return p; };
            DataPath = AnsiString(bs(data).c_str());
            OffsetPath = AnsiString(bs(root + "Offset\\").c_str());
            const bool oKeep1 = CosFunction.bKeepOnly1SetupFile, oLastSet = CosFunction.bLastSetInSetUpFile, oNoSend = bNoSendSiteOnOff;
            const int oAtc = ATC_SYSTEM;
            void (*oLook)() = W906_St02_Ftp.LookForFile;
            LastDataPath = AnsiString((root + "lastdata_li9.dat").c_str());
            CosFunction.bKeepOnly1SetupFile = false; CosFunction.bLastSetInSetUpFile = false; bNoSendSiteOnOff = false;
            if (ATC_SYSTEM == eATCHonPrecType) ATC_SYSTEM = 0;
            static int s_look = 0;
            s_look = 0;
            W906_St02_Ftp.LookForFile = []() { ++s_look; };
            const std::string ofs = root + "Offset\\";
            MyForceDirectories(AnsiString((data + "SAV1_NET").c_str()));
            MyForceDirectories(AnsiString((ofs + "SAV1_NET").c_str()));
            WriteAll(data + "SAV1_NET\\Tray.Data", "downloaded tray");
            WriteAll(ofs + "SAV1_NET\\Position Offset.Data", "downloaded offset");
            fMain->cbSetupFileName->Text = "SAV1_NET";
            W906_St02_LotInfo_btSaveSetupFileVisible = true;
            g_changes.clear();
            W906_St02_LotInfo_btSaveSetupFileClick();
            CHECK(FileMarkOf((data + "SAV1\\Tray.Data").c_str()).exists && FileMarkOf((ofs + "SAV1\\Position Offset.Data").c_str()).exists,
                  "11a. btSaveSetupFileClick: SAV1_NET\\*.* copied into SAV1 under DataPath and OffsetPath (913 :5262-5296, SHFileOperation FO_COPY)");
            CHECK(!FileMarkOf((data + "SAV1_NET").c_str()).exists && !FileMarkOf((ofs + "SAV1_NET").c_str()).exists,
                  "11a. ClearAllSetupFile: the _NET recipe is deleted (913 :12237-12238 fBuilder->DeleteSetupFile)");
            std::ifstream ld(LastDataPath.c_str());
            std::string ldText((std::istreambuf_iterator<char>(ld)), std::istreambuf_iterator<char>());
            ld.close();
            CHECK(s_look == 1 && fMain->cbSetupFileName->Text == "SAV1" && ldText.find("SAV1") == 0 && g_changes.size() == 1 && g_changes[0] == "SAV1" &&
                      !W906_St02_LotInfo_btSaveSetupFileVisible && fSetup->bFirstTime == false,
                  "11a. then LookForFile (Q4 hook), the recipe name SAV1, WriteLastDataFN (sandbox), ChangeSetUpFile(SAV1), btSaveSetupFile hidden, "
                  "fSetup->bFirstTime false (913 :12275-12285, :5324)");

            // bKeepOnly1SetupFile: every other recipe folder goes, the kept one stays (sandbox only)
            MyForceDirectories(AnsiString((data + "KEEPX").c_str()));
            CosFunction.bKeepOnly1SetupFile = true;
            g_changes.clear(); s_look = 0;
            W906_St02_LotInfo_ClearAllSetupFile("SAV1", "");
            CHECK(!FileMarkOf((data + "KEEPX").c_str()).exists && FileMarkOf((data + "SAV1\\Tray.Data").c_str()).exists && g_changes.size() == 1,
                  "11b. bKeepOnly1SetupFile: KEEPX deleted, SAV1 kept, the recipe changed again (913 :12240-12272); the Q3 FindClose guard ran clean");

            // no ChangeSetUpFile installed: noted, nothing else
            int (*oChg)(AnsiString) = W906_St02_Ftp.ChangeSetUpFile;
            W906_St02_Ftp.ChangeSetUpFile = 0;
            CosFunction.bKeepOnly1SetupFile = false;
            rep9.notes.clear();
            W906_St02_LotInfo_ClearAllSetupFile("SAV1", "");
            bool noted = false;
            for (std::size_t i = 0; i < rep9.notes.size(); ++i) if (rep9.notes[i].find("ChangeSetUpFile is not installed") != std::string::npos) noted = true;
            CHECK(noted, "11c. ChangeSetUpFile not installed: noted, the recipe is not changed (port-only, as F1's plLoadClick)");
            W906_St02_Ftp.ChangeSetUpFile = oChg;

            W906_St02_Ftp.LookForFile = oLook;
            LastDataPath = oLast; DataPath = oData11; OffsetPath = oOffset11;
            CosFunction.bKeepOnly1SetupFile = oKeep1; CosFunction.bLastSetInSetUpFile = oLastSet; bNoSendSiteOnOff = oNoSend; ATC_SYSTEM = oAtc;
        }


        // ---- [12] plSLoadClick 'Download to Handler' / DownloadPasswordFormServer (POOL-14 MR-C, golden 913 FTPClient.cpp:862-1119,
        //      :4498-4586) -- the whole chain against the fake FTP server: list, download (RETR <name>.zip x3 + .Offset), Gate #2 ->
        //      DownloadFromServer (fake 7z), St01's seats (fakes here), the recipe change, btSaveSetupFileClick (copy _NET -> name),
        //      ClearAllSetupFile.  LastDataPath / pwPath / DataPath / OffsetPath are sandbox paths ('\' spelling for SHFileOperation).
        std::printf("[12] Download to Handler (POOL-14 MR-C)\n");
        {
            const AnsiString oLast12 = LastDataPath, oData12 = DataPath, oOffset12 = OffsetPath, oPwPath = pwPath, oPwName = pwName;
            const AnsiString oFH12 = IniConfig.FtpHost, oFU12 = IniConfig.FtpUserName, oFP12 = IniConfig.FtpPassword, oPort12 = IniConfig.N06_FtpPort;
            const AnsiString oDown12 = IniConfig.FtpDownloadPath, oPwDir = IniConfig.FtpPasswordDownloadPath;
            const bool oSecs = IniConfig.bEnable_SECS_GEM, oPwF = CosFunction.PassworDownloadByFTP, oPwI = IniConfig.bFtpPasswordDownload;
            const int oMode12 = IniConfig.FtpTransMode, oAtc12 = ATC_SYSTEM;
            if (ATC_SYSTEM == eATCHonPrecType) ATC_SYSTEM = 0;   // btSaveSetupFileClick :5326 would re-connect the HonPrec ATC (no panels here)
            void (*oLook12)() = W906_St02_Ftp.LookForFile;
            void (*oChg12)() = W906_St02_Ftp.cbSetupFileNameChange;
            auto bs = [](std::string p) { for (std::size_t i = 0; i < p.size(); ++i) if (p[i] == '/') p[i] = '\\'; return p; };
            DataPath = AnsiString(bs(data).c_str());
            OffsetPath = AnsiString(bs(root + "Offset\\").c_str());
            LastDataPath = AnsiString((root + "lastdata_li9_12.dat").c_str());
            IniConfig.FtpHost = "li9-fake-ftp"; IniConfig.FtpUserName = "li9"; IniConfig.FtpPassword = "li9"; IniConfig.N06_FtpPort = "21";
            IniConfig.FtpDownloadPath = "/dl/"; IniConfig.FtpTransMode = 1; IniConfig.bEnable_SECS_GEM = false;
            CosFunction.PassworDownloadByFTP = false; IniConfig.bFtpPasswordDownload = false;
            static std::vector<std::string> s_seq12;
            s_seq12.clear();
            W906_St02_Ftp.LookForFile = []() { s_seq12.push_back("look"); };
            W906_St02_Ftp.cbSetupFileNameChange = []() { s_seq12.push_back("cbChange:" + std::string(fMain->cbSetupFileName->Text.c_str())); };
            W906_St02_Ftp.DownloadResult = [](const AnsiString& n, bool f) { s_seq12.push_back("result:" + std::string(n.c_str()) + (f ? ":fail" : ":ok")); };
            W906_St02_Ftp.DownloadRecordNetData = []() { s_seq12.push_back("record"); };
            W906_KyecFtpPrepareHook = &Prepare8;
            W906_KyecFtpDownloadFromServerHook = &CountingDfsHook;
            FTPClientTransfer_SetFastDelayForTest(true);
            g_srv8.byCmd = true; g_srv8.vary = false; g_srv8.retrCount = 0;
            g_srv8.listing = { "DL1.zip", "OTHER.zip" };
            g_srv8.retr = "PK-fake-zip-bytes";
            CHECK(fFTPClient->bControlByBarcode == false,
                  "12. the LEADYO trigger starts off (Q2: golden 913 bControlByBarcode, default false; iErrorByBarcode is set by golden paths)");

            // (a) the operator: Server, type DL1, Download to Handler
            r = Act("open", "{\"tag\":0}");
            r = Act("filter", "{\"edit\":\"server\",\"text\":\"DL1\"}");
            g_seq8.clear(); g_changes.clear(); g_dfsHookCalls = 0;
            r = Act("download", "{}");
            {
                Reply y(r);
                std::string seq; for (std::size_t i = 0; i < s_seq12.size(); ++i) seq += s_seq12[i] + "|";
                std::printf("  %s\n", seq.c_str());
                CHECK(y.B("executed") && Find8("ctrl:RETR DL1.zip") >= 0 && g_dfsHookCalls == 1,
                      "12a. Download to Handler: LoadFileFormServer2 RETR DL1.zip, then Gate #2 -> DownloadFromServer (golden 913 :1002, :336)");
                CHECK(FileMarkOf((data + "DL1\\Tray.Data").c_str()).exists && !FileMarkOf((data + "DL1_NET").c_str()).exists &&
                          g_changes.size() == 1 && g_changes[0] == "DL1" && fMain->cbSetupFileName->Text == "DL1",
                      "12a. DL1_NET unzipped, saved as DL1 (btSaveSetupFileClick), DL1_NET deleted, the recipe changed to DL1 (913 :1037-1049)");
                CHECK(seq.find("result:DL1:ok|") != std::string::npos && seq.find("cbChange:DL1_NET|") > seq.find("result:DL1:ok|") &&
                          seq.find("record|") > seq.find("cbChange:DL1_NET|") && !y.B("state.open"),
                      "12a. order: St01's DownloadResult(DL1, ok) -> cbSetupFileNameChange(DL1_NET) -> ... -> DownloadRecordNetData; the dialog closes (913 :1118)");
            }

            // (b) SECS remote download (bControlBySECSGEM): no window, event 64 DownLoadRecipeByFTPOK
            IniConfig.bEnable_SECS_GEM = true;
            ResetSimEventReport();
            s_seq12.clear(); g_changes.clear();
            fFTPClient->bControlBySECSGEM = true; fFTPClient->aSetUpNameBySECSGEM = "DL1";
            fFTPClient->ShowFTPModal(0);
            CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DownLoadRecipeByFTPOK && g_changes.size() == 1 && !fFTPClient->bShow &&
                      !fFTPClient->W906_bModalOpen && fFTPClient->bControlBySECSGEM == false,
                  "12b. SECS remote: ShowFTPModal(0) lists, the SECS arm runs plSLoadClick without a window (913 :1543-1548), event 64 OK (:1062)");

            // (c) the three downloads differ in size -> FTP_DownloadFail: event 65 NG, iErrorBySECSGEM 3, DownloadResult(fail)
            g_srv8.vary = true; g_srv8.retrCount = 0;
            ResetSimEventReport();
            s_seq12.clear();
            fFTPClient->bControlBySECSGEM = true; fFTPClient->aSetUpNameBySECSGEM = "DL1";
            fFTPClient->ShowFTPModal(0);
            {
                std::string seq; for (std::size_t i = 0; i < s_seq12.size(); ++i) seq += s_seq12[i] + "|";
                CHECK(g_SimLastEventReportCeid == (unsigned)SECS_EVENT.DownLoadRecipeByFTPNG && fFTPClient->iErrorBySECSGEM == 3 &&
                          seq.find("result:DL1:fail|") != std::string::npos,
                      "12c. download size mismatch: FTP_DownloadFail -> iErrorBySECSGEM 3, DownloadResult(fail), event 65 NG (913 :1008-1014, :1060)");
            }
            g_srv8.vary = false;
            IniConfig.bEnable_SECS_GEM = false;

            // (d) the password book: CWD [FTP] password path, RETR pwName into pwPath (pointed into the sandbox)
            CosFunction.PassworDownloadByFTP = true; IniConfig.bFtpPasswordDownload = true;
            IniConfig.FtpPasswordDownloadPath = "/pw";
            pwName = "tech.com";
            pwPath = AnsiString((root + "tech_li9.com").c_str());
            g_seq8.clear();
            const bool pw = fFTPClient->DownloadPasswordFormServer();
            CHECK(pw && Find8("ctrl:CWD /pw/") >= 0 && Find8("ctrl:RETR tech.com") > Find8("ctrl:CWD /pw/") &&
                      FileMarkOf(pwPath.c_str()).exists && !FTP_DownloadFail,
                  "12d. DownloadPasswordFormServer: CWD /pw/, RETR tech.com into pwPath (sandbox), FTP_DownloadFail false (913 :4548-4574)");
            CosFunction.PassworDownloadByFTP = false;
            CHECK(fFTPClient->DownloadPasswordFormServer() == false, "12d. flag off: returns at once (913 :4500-4502)");
            CosFunction.PassworDownloadByFTP = oPwF; IniConfig.bFtpPasswordDownload = oPwI;

            // (e) LEADYO barcode arm (Q2): the name is not on the server -> iErrorByBarcode 2, no download
            const int oCC12 = CUSTOMER_CODE;
            CUSTOMER_CODE = CC_LEADYO;
            fFTPClient->bControlByBarcode = true; fFTPClient->asSetUpNameByBarcode = "NOPE";
            g_dfsHookCalls = 0;
            fFTPClient->ShowFTPModal(0);
            CHECK(fFTPClient->iErrorByBarcode == 2 && g_dfsHookCalls == 0 && fFTPClient->bControlByBarcode == false && !fFTPClient->bShow,
                  "12e. LEADYO barcode arm: NOPE is not on the server -> iErrorByBarcode 2, Close, nothing downloaded (913 :1580-1602, :1615)");
            CUSTOMER_CODE = oCC12;

            // (f) the WS op refuses while running
            r = Act("open", "{\"tag\":0}");
            SystemStart = true;
            r = Act("download", "{}");
            CHECK(Has(r, "\"guard\":\"running\""), "12f. download while SystemStart -> running (port-only: the web window is not modal)");
            SystemStart = false;
            Act("close", "{}");

            FTPClientTransfer_SetFastDelayForTest(false);
            g_srv8.byCmd = false;
            W906_KyecFtpPrepareHook = 0; W906_KyecFtpDownloadFromServerHook = 0;
            W906_St02_Ftp.LookForFile = oLook12; W906_St02_Ftp.cbSetupFileNameChange = oChg12;
            W906_St02_Ftp.DownloadResult = 0; W906_St02_Ftp.DownloadRecordNetData = 0;
            LastDataPath = oLast12; DataPath = oData12; OffsetPath = oOffset12; pwPath = oPwPath; pwName = oPwName;
            IniConfig.FtpHost = oFH12; IniConfig.FtpUserName = oFU12; IniConfig.FtpPassword = oFP12; IniConfig.N06_FtpPort = oPort12;
            IniConfig.FtpDownloadPath = oDown12; IniConfig.FtpPasswordDownloadPath = oPwDir; IniConfig.bEnable_SECS_GEM = oSecs;
            IniConfig.FtpTransMode = oMode12; ATC_SYSTEM = oAtc12;
        }

        W906_St02_FtpCurrentReport = 0;
        W906_KyecFtpSystemHook = 0; W906_KyecFtpShellExecHook = 0; W906_KyecFtpCopyFileHook = 0;
        W906_ShowErrorMessage_Reset();
        sFTPSetupFileLogPath = oLogPath;
        CosFunction.bFTPFunction = oFTPF; IniConfig.bEnableRms = oRms; IniConfig.FtpUseSystemCallToUnZip = oSysUnzip;
        CosFunction.bFTPDownloadAlwaysCover = oCover; IniConfig.bE45_AllSetupFileUseOneFile = oE45; CosFunction.bUseLocalRecipeOffset = oLocalOfs;
        IniConfig.bN20_CheckMD5 = oMd5; CosFunction.bUseFTPDownLoadATCRecipe = oAtcDl; CosFunction.bUseATCFileTransfer = oAtcFt;
        CosFunction.bContactHeightSaveToContactIni = oCH; CosFunction.bSmartAutoClean = oSmart; CUSTOMER_CODE = oCC9;
        fMain->cbSetupFileName->Items->Clear();
        ::SetCurrentDirectoryA(cwd9);
    }

    // ---- restore ----
    ht9045::sjson::SetLotInfoFtpBody(0);
    W906_St02_Ftp.ChangeSetUpFile = 0; W906_St02_Ftp.RecipeChainsReady = 0; W906_St02_Ftp.LocalIp = 0;
    DataPath = oldData; OffsetPath = oldOffset; AuthPath = oldAuth;
    IniConfig.N06_TasterListFile = oldListFile; IniConfig.N06_TasterListMap = oldMap;
    IniConfig.TasterType = oldTType; IniConfig.TasterNo = oldTNo; IniConfig.TasterName = oldTName;
    IniConfig.sMachineType = oldMType; IniConfig.SocketHandlerID = oldHID; IniConfig.TasterInputMethod = oldTIM;
    IniConfig.iServerEnable = oldSE; IniConfig.iHDEnable = oldHE; IniConfig.bSPILFunction = oldSPIL;
    CosFunction.bFTPFunction = oldFTPF;
    CUSTOMER_CODE = oldCC; AccessLevel = oldAL; SystemStart = oldSS;
    fMain->cbSetupFileName->Items->Clear();
    fMain->cbSetupFileName->Text = oldRecipe;

    CHECK(Same(realCfg0, FileMarkOf("D:\\HT9045\\config\\config.ini")) && Same(realInf0, FileMarkOf("D:\\HT9045\\SetUp.inf")),
          "the machine's D:\\HT9045\\config\\config.ini and D:\\HT9045\\SetUp.inf did not change");

    std::printf("LI9_FtpClient: %d passed, %d failed (sandbox kept: %s)\n", g_pass, g_fail, root.c_str());
    return g_fail == 0 ? 0 : 1;
}
