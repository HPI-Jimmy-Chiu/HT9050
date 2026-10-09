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
//  NEVER THE NETWORK.  Server (tag 0) is card F2 and ends at its gate; the only network-shaped path, a Taster map under
//  \\host\share, is run only in the SIM build where W58 Q5 skips it by the path string alone (W906_SimNetPathBlocked), and
//  only when W906_SIM_NET_PATHS is not 1.
//  CONTAINMENT FIRST: exit 2 before any Handler code unless ctest's redirect roots are set (st02_test_containment.h) and
//  DataPath / OffsetPath / AuthPath are ctest's scratch.  Then DataPath, OffsetPath, AuthPath and the two Taster files are
//  pointed at <as9045LogPath>\LI9_FtpClient_<tick>\ (inside machine_log_scratch) and restored at the end.  Fails if the
//  machine's D:\HT9045\config\config.ini or D:\HT9045\SetUp.inf changed (size / write time).
//    1  dispatcher: not-installed, bad-op
//    2  dfm defaults of the Lot Info FTP buttons; the body installs
//    3  btnFtpServerClick guards: tab-hidden, button-disabled, access-level (button back on), system-start, bad-tag,
//       tag 3 (non-TSMC returns), tag 2 + SPIL (golden quirk: stays disabled), tag 0 = f2-not-yet (FormClose ran, no socket)
//    4  HD: the list (fallback = DataPath folders when fMain->cbSetupFileName is empty; else its Items), FilterList exact /
//       case-insensitive, pick, "FTP Form already Opened!!", loadHD -> ChangeSetUpFile + close + the btnFtpServerClick tail,
//       refused loadHD (未選擇這路徑), mode 1 (golden :1648 quirk), enterHD, boot-read-chain guard, hook missing, close, not-open
//    5  Tester: list file -> combos, cbTesterTypeChange / cbTesterIDChange, Save writes the map + config.ini only in the
//       sandbox, a bad name (Tester Name 錯誤, nothing closes), Manually reads config.ini, exit, missing list file = VCL
//       EFOpenError, W58 Q5 network map skipped (SIM build)
//    6  memoClear
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
    const int memo0 = fFTPClient->memoFTP->Lines->Count;
    r = Act("open", "{\"tag\":0}");
    {
        Reply y(r);
        CHECK(y.S("guard") == "f2-not-yet" && !y.B("state.open") && !fFTPClient->bShow && fLotInfo->btnFtpServer->Enabled,
              "3. tag 0 = Server: f2-not-yet, not open, the button enabled again");
        CHECK(fFTPClient->memoFTP->Lines->Count == memo0 + 2 && W906_St02_FtpTmpList() == 0,
              "3. tag 0: golden's own failure path ran (bShow=false; Close() -> FormClose: memo lines, tmpList freed)");
        CHECK(Has(r, "card F2") && !y.B("opened"), "3. tag 0: the gate is named in the reply");
    }

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
    CHECK(Has(r, "\"guard\":\"f2-not-yet\""), "4. the Server edit is F2");
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
