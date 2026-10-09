// =============================================================================
//  tests/test_c10_hana_rms.cpp -- card ST02-C10: golden 912 HANA RMS Interlock (Automation/automation.cpp:2514-3158)
//  as THanaRmsMembers (Automation/HanaRms_St02.h), the base of TfAutomationShim, and the lifted gate G9
//  (TesterComm/Handler/HandlerBridgeCtl.cpp, golden 912 main.cpp:18535-18539).  AI(W906-ST02-C10) 20261002 (St02-E helper).
//  Suite: C10_HanaRms.  Steven W64 = A (1002 08:0x): 912.0, RogerYang's comments win.
//
//  NEVER THE NETWORK: the socket is vclcompat's TClientSocket in Sim mode (no OS socket at all); the only switch to a
//  real one is W906_HanaRmsSetRealSocket(true), which only TesterCommWiring.cpp calls -- never this test.  Part 3 asserts
//  IsSimMode() before anything is sent.  Server bytes come in through Socket->SimPushReceive, sent bytes are read from
//  Socket->SimTxBuffer, the server's 30 s idle close is HANARMSClient->Close() (fires the golden OnDisconnect).
//
//  0. CONTAINMENT FIRST (the b12ab375 rule): refuses (exit 2) before any HANA RMS code unless as9045LogPath /
//     W906_HT9045LOG_ROOT are ctest's machine_log_scratch and AuthPath / W906_AUTH_PATH its machine_config_scratch
//     (tests/CMakeLists.txt: _ht9045_env_extra and the merged ENVIRONMENT string, given to every test by the deferred
//     _w906_env_all_tests).  Then as9045LogPath / AuthPath / LastDataPath point into a private folder
//     <machine_log_scratch>\C10_HanaRms_<tick> (HANARMS\ = golden D:\HT9045_Log\HANARMS, config\HANARMS.ini = golden
//     D:\HT9045\config\HANARMS.ini).  The machine's own D:\HT9045\config\HANARMS.ini and D:\HT9045_Log\HANARMS are
//     stamped before and compared after (read only).  SystemYear/Month/Date = 2099-12-31 so the day file can never be a
//     real one.  0b: with the golden roots the two paths are golden's literals (string check, no disk).
//  1. Today unchanged: fresh object = dfm defaults, no socket; non-HANA machine: RunCheckOK passes with the latch set,
//     writes nothing; HANA with A77 off: PrepareHANARMSConnect reads no ini, makes no socket, writes no log;
//     HANARMSQueryJobInfo does nothing; the pump does nothing; rsmAutoSiteMap passes.
//  2. HANA + A77, A10-6 off: golden's [WARN] (K7 "A76" text); 127.0.0.1 = placeholder -> no connect (K3).
//  3. Connect: ini IP / Port -> "connecting to" + "connected", status label, Sim + polled socket; a second call does
//     nothing (golden :3062 "已連/連線中 -> 不重連"); the pump on a Sim socket adds nothing.
//  4. Send: trimmed + '\n'; empty -> nothing; the ManCmd button.
//  5. Receive: split by '\n', CR stripped, empty lines skipped, CHECKSTR warning (K6), 8192 garbage drop.
//  6. Auto chain: QueryJobInfo -> GET_LOT_INFO (HDNAME HD_01 = K2) -> REP -> GET_JOB_INFO with LOCATION / CUSTCD /
//     PARTNO -> REP -> indented fields + "[CMP] PASS", baseline stored; HDName from the ini when set.
//  7. Interlock: RunCheckOK latch / PASS clears it / FAIL keeps it + ShowMyMessage (bShowAlarm) / silent (false);
//     no baseline; GB_CT gated => FAIL ([W906] 4); CON_MTD index 9 = RELEASE (K4); TEMP room / Abit.
//  8. 912 defects as 912: server drop -> disconnected, [TX] then "not connected, not sent" (K1 / K5), the pump does not
//     reconnect; templates (K2 / K8); RogerYang :3109 applied: same lot keeps the baseline, a new lot voids it.
//  9. Error event: logged, ErrorCode cleared, status Disconnected.
// 10. RogerYang :3062 "connecting" half ([W906] 5): a pending connect is not restarted.
// 11. Button bodies that write files: Setting -> sandbox HANARMS.ini; Save -> sandbox .log; Clear; memo auto-trim at 1000.
// 12. Source pins: G9 is live in HandlerBridgeCtl.cpp (comment-stripped), the pump / policy calls in TesterCommWiring.cpp
//     are code (before the //), T08 in atester.cpp is still gated (plan Q5: 3b waits for HANA ART), atester_shims.h has
//     the THanaRmsMembers base (claim).
//  Every global it changes is saved and restored; the private folder is removed on a green run.
// =============================================================================
#include "MachineDefine.h"                    // the tree's include hub
#include "atester_shims.h"                    // fAutomation (TfAutomationShim : THanaRmsMembers)
#include "Automation/HanaRms_St02.h"
#include "Automation/HanaRms_Internal_St02.h" // w906hanarms::LogDir / IniPath / HANARMSAddLog (part 0b / 11)
#include "MachineType.h"                      // CC_HANA_MICRON, eBinFT / eBinRT, TEST_MAX_BIN, contact modes, rsmAutoSiteMap
#include "cprod.h"                            // TestIF, DeviceForm_File, Temperature, BinSelect, IniConfig
#include "cmydef.h"                           // CUSTOMER_CODE, Tempture_*, SystemYear .. SystemSec
#include "LastSet.h"
#include "Config.h"
#include "common.h"                           // as9045LogPath, AuthPath, LastDataPath, CloseIniFile
#include "canary_support.h"                   // W906_ShowMyMessage_Count / _LastS1

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdarg>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#ifndef W906_SRC_ROOT
#define W906_SRC_ROOT "."
#endif

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

// ---- small helpers ------------------------------------------------------------------------------------------------
static std::string Fmt(const char* fmt, ...)
{
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    return std::string(buf);
}
static std::string Lower(std::string s) { for (size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); return s; }
static std::string Slashes(std::string s) { for (size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\'; return s; }
static bool Exists(const std::string& p) { return ::GetFileAttributesA(p.c_str()) != INVALID_FILE_ATTRIBUTES; }
static bool IsDir(const std::string& p) { DWORD a = ::GetFileAttributesA(p.c_str()); return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY); }
static bool ReadAll(const std::string& p, std::string& out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss; ss << f.rdbuf(); out = ss.str();
    return true;
}
static void WriteAll(const std::string& p, const std::string& bytes)
{
    std::ofstream f(p.c_str(), std::ios::binary);
    f << bytes;
}
static std::string S(const AnsiString& a) { return std::string(a.c_str()); }
static bool Has(const std::string& hay, const std::string& needle) { return hay.find(needle) != std::string::npos; }
static bool EndsWith(const std::string& s, const std::string& t) { return s.size() >= t.size() && s.compare(s.size() - t.size(), t.size(), t) == 0; }

// a file's identity for the read-only "machine untouched" proof
struct Stamp { bool exists; unsigned long long size; unsigned long long mtime; };
static Stamp StampOf(const std::string& p)
{
    Stamp s = { false, 0, 0 };
    WIN32_FILE_ATTRIBUTE_DATA d;
    if (::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &d)) {
        s.exists = true;
        s.size = ((unsigned long long)d.nFileSizeHigh << 32) | d.nFileSizeLow;
        s.mtime = ((unsigned long long)d.ftLastWriteTime.dwHighDateTime << 32) | d.ftLastWriteTime.dwLowDateTime;
    }
    return s;
}
static bool SameStamp(const Stamp& a, const Stamp& b) { return a.exists == b.exists && a.size == b.size && a.mtime == b.mtime; }

// remove the private folder (only one under machine_log_scratch whose name is C10_HanaRms_*)
static bool RemoveTree(const std::string& dir)
{
    const std::string l = Lower(dir);
    if (l.find("machine_log_scratch") == std::string::npos || l.find("c10_hanarms_") == std::string::npos) return false;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p);
            else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    return ::RemoveDirectoryA(dir.c_str()) != 0;
}

// ---- the object under test ------------------------------------------------------------------------------------------
static THanaRmsMembers* H() { return fAutomation; }
static int MemoCount() { return H()->mmoHANARMS->Lines->GetCount(); }
static std::string MemoLine(int i) { return S(H()->mmoHANARMS->Lines->GetString(i)); }
static bool MemoHasSince(int from, const std::string& sub)
{
    for (int i = from; i < MemoCount(); ++i) if (Has(MemoLine(i), sub)) return true;
    return false;
}
static void DumpMemoSince(int from)
{
    for (int i = from; i < MemoCount(); ++i) std::printf("    memo[%d] = %s\n", i, MemoLine(i).c_str());
}
static std::string Tx()
{
    if (H()->HANARMSClient == NULL) return std::string();
    const std::vector<char>& v = H()->HANARMSClient->Socket->SimTxBuffer();
    return v.empty() ? std::string() : std::string(&v[0], v.size());
}
static void ClearTx() { if (H()->HANARMSClient != NULL) H()->HANARMSClient->Socket->SimClearTx(); }
static void Push(const std::string& bytes) { H()->HANARMSClient->Socket->SimPushReceive(bytes.data(), (int)bytes.size()); }

// golden-literal comment-stripping for the source pins (// and /* */ outside string literals)
static std::string StripComments(const std::string& line)
{
    std::string o;
    bool inStr = false, inChr = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (inStr) { o += c; if (c == '\\' && i + 1 < line.size()) { o += line[++i]; } else if (c == '"') inStr = false; continue; }
        if (inChr) { o += c; if (c == '\\' && i + 1 < line.size()) { o += line[++i]; } else if (c == '\'') inChr = false; continue; }
        if (c == '"') { inStr = true; o += c; continue; }
        if (c == '\'') { inChr = true; o += c; continue; }
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '/') break;
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '*') {
            const size_t e = line.find("*/", i + 2);
            if (e == std::string::npos) break;
            i = e + 1;
            continue;
        }
        o += c;
    }
    return o;
}
static bool ReadLines(const char* rel, std::vector<std::string>& out)
{
    std::string text;
    if (!ReadAll(std::string(W906_SRC_ROOT) + "/" + rel, text)) return false;
    out.clear();
    std::string cur;
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') { if (!cur.empty() && cur[cur.size() - 1] == '\r') cur.erase(cur.size() - 1); out.push_back(cur); cur.clear(); }
        else cur += text[i];
    }
    if (!cur.empty()) out.push_back(cur);
    return true;
}

// the job-info reply that matches the Handler recipe set in part 6 (FT_S_T_S with golden's real-machine spacing quirk)
static std::string JobLine(const std::string& gbct = "-", const std::string& conmtd = "RELEASE", const std::string& pinfc = "25.5")
{
    return "CMD=\"GET_JOB_INFO_REP\" LOT=\"LOT123\" RCP_NAME=\"C10_RECIPE\" GB_CT=\"" + gbct + "\" TEMP=\"85\" PIN_CNT=\"48\""
           " PIN_FC=\"" + pinfc + "\" CON_MTD=\"" + conmtd + "\" WIN_CNT=\"100\" LOW_SPD=\"E\""
           " FT_C_FAIL_SCK=\"3\" FT_S_T_S =\" 80\" FT_LOW_YD=\"70\" FT_AT_OFF=\"E\" FT_SB_ARM=\"3, 7\" FT_SB_ARM_CNT=\"2,5\""
           " FT_SB_SCK=\"-\" FT_SB_SCK_CNT=\"-\" RT_C_FAIL_SCK=\"4\" RT_S_T_S=\"75\" RT_LOW_YD=\"65\" RT_AT_OFF=\"E\""
           " RT_SB_ARM=\"-\" RT_SB_ARM_CNT=\"-\" RT_SB_SCK=\"-\" RT_SB_SCK_CNT=\"-\" GET_RDY=\"YES\" CHECKSTR=\"END\"";
}
static bool CheckLine(const std::string& line, std::string& fail)
{
    H()->sHANARMSJobInfoLine = AnsiString(line.c_str());
    AnsiString asFail;
    const bool r = H()->HANARMSCheckRecipe(asFail);
    fail = S(asFail);
    return r;
}

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("C10_HanaRms\n");

    // ---- 0. containment first ------------------------------------------------------------------------------------
    {
        bool contained = true;
        std::printf("  as9045LogPath = %s\n  AuthPath = %s\n", as9045LogPath.c_str(), AuthPath.c_str());
        if (Lower(S(as9045LogPath)).find("machine_log_scratch") == std::string::npos) contained = false;
        if (Lower(S(AuthPath)).find("machine_config_scratch") == std::string::npos) contained = false;
        const char* v1 = std::getenv("W906_HT9045LOG_ROOT");
        const char* v2 = std::getenv("W906_AUTH_PATH");
        std::printf("  W906_HT9045LOG_ROOT = %s\n  W906_AUTH_PATH = %s\n", v1 ? v1 : "(unset)", v2 ? v2 : "(unset)");
        if (Lower(v1 ? v1 : "").find("machine_log_scratch") == std::string::npos) contained = false;
        if (Lower(v2 ? v2 : "").find("machine_config_scratch") == std::string::npos) contained = false;
        if (!contained) {
            std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R C10_HanaRms) -- nothing was called\n");
            return 2;
        }
    }
    if (fAutomation == NULL) { std::printf("  ABORT: fAutomation is NULL\n"); return 2; }
    const std::string root = Slashes(S(as9045LogPath)) + Fmt("\\C10_HanaRms_%lu", (unsigned long)::GetTickCount());
    {
        const std::string l = Lower(root);
        if (l.find("machine_log_scratch") == std::string::npos || l.compare(0, 13, "d:\\ht9045_log") == 0) {
            std::printf("  ABORT: sandbox %s is not under the ctest scratch -- nothing was called\n", root.c_str());
            return 2;
        }
    }
    ForceDirectories(AnsiString((root + "\\config").c_str()));
    if (!IsDir(root + "\\config")) { std::printf("  ABORT: cannot create %s\n", root.c_str()); return 2; }
    std::printf("  sandbox = %s\n", root.c_str());

    // the machine's own files: stamped now, compared at the end (read only)
    const std::string kRealIni = "D:\\HT9045\\config\\HANARMS.ini";
    const std::string kRealLogDir = "D:\\HT9045_Log\\HANARMS";
    const std::string kRealDayFile = kRealLogDir + "\\HANARMS_20991231.txt";
    const Stamp realIni0 = StampOf(kRealIni);
    const Stamp realDir0 = StampOf(kRealLogDir);
    const bool realDayFile0 = Exists(kRealDayFile);

    // ---- save every global this test touches -----------------------------------------------------------------------
    const AnsiString sav_LogPath = as9045LogPath, sav_AuthPath = AuthPath, sav_LastDataPath = LastDataPath;
    const Word sav_Y = SystemYear, sav_M = SystemMonth, sav_D = SystemDate, sav_h = SystemHour, sav_m = SystemMin, sav_s = SystemSec;
    const int sav_CC = CUSTOMER_CODE;
    const bool sav_A77 = IniConfig.bA77_EnableHanaRMSInterlock, sav_A106 = IniConfig.bA10_6_HANA_ART_TestMode_Enable;
    const int sav_RunStart = LastSet.iRunStartMode, sav_Temp = LastSet.iTemperature;
    const double sav_fWork = Temperature.fWorkTemperBase, sav_fAbit = Temperature.fAbitTemp;
    const bool sav_bAbit = Temperature.bUseAbitCHK;
    const int sav_PinCT = DeviceForm_File.iPinCT, sav_Contact = DeviceForm_File.ContactMode;
    const double sav_PinFC = DeviceForm_File.ForcePerPinG;
    const int sav_Win = TestIF.iSlidingWindowSize;
    const unsigned int sav_CF = TestIF.iContsFailSocketAlarmCT, sav_CFRT = TestIF.iContsFailSocketAlarmCT_RT;
    const double sav_STS = TestIF.dFailAlarmSiteYield, sav_STSRT = TestIF.dFailAlarmSiteYield_RT, sav_LY = TestIF.dLowYieldLimit, sav_LYRT = TestIF.dLowYieldLimit_RT;
    const bool sav_AtOff = TestIF.bLowYieldAutoSiteOff;
    std::vector<bool> sav_bArm[2], sav_bSck[2];
    std::vector<unsigned int> sav_cArm[2], sav_cSck[2];
    const int binIdx[2] = { eBinFT, eBinRT };
    for (int k = 0; k < 2; ++k)
        for (int i = 0; i < TEST_MAX_BIN; ++i) {
            sav_bArm[k].push_back(BinSelect[binIdx[k]].bSpecialBinByArm[i]);    sav_bSck[k].push_back(BinSelect[binIdx[k]].bSpecialBinBySocket[i]);
            sav_cArm[k].push_back(BinSelect[binIdx[k]].iSpecialBinCountByArm[i]); sav_cSck[k].push_back(BinSelect[binIdx[k]].iSpecialBinCountBySocket[i]);
        }

    // ---- 0b. with golden's roots the two paths are golden's literals (string only, nothing touches the disk) --------
    std::printf("\n-- 0b. production paths = golden literals --\n");
    as9045LogPath = "D:\\HT9045_Log";  AuthPath = "D:\\HT9045\\config\\";
    const std::string prodLog = S(w906hanarms::LogDir()), prodIni = S(w906hanarms::IniPath());
    as9045LogPath = AnsiString(root.c_str());  AuthPath = AnsiString((root + "\\config\\").c_str());
    CHECK(prodLog == "D:\\HT9045_Log\\HANARMS");             // golden automation.cpp:2527 / :2996
    CHECK(prodIni == "D:\\HT9045\\config\\HANARMS.ini");     // golden :3044 / :2890 / :3112 (and :3015 "Config")
    const std::string logDir = root + "\\HANARMS", iniFile = root + "\\config\\HANARMS.ini", dayFile = logDir + "\\HANARMS_20991231.txt";
    CHECK(Lower(S(w906hanarms::LogDir())) == Lower(logDir));
    CHECK(Lower(S(w906hanarms::IniPath())) == Lower(iniFile));
    LastDataPath = AnsiString((root + "\\SetUp.inf").c_str());
    WriteAll(root + "\\SetUp.inf", "C10_RECIPE\r\n");                       // GetLastOpenFN -> RCP_NAME
    SystemYear = 2099; SystemMonth = 12; SystemDate = 31; SystemHour = 23; SystemMin = 59; SystemSec = 58;
    ::DeleteFileA(iniFile.c_str());
    CloseIniFile();

    // ---- 1. today unchanged ----------------------------------------------------------------------------------------
    std::printf("\n-- 1. non-HANA / A77 off: nothing happens --\n");
    CHECK(H()->HANARMSClient == NULL);
    CHECK(S(H()->edtHANARMSIP->Text) == "127.0.0.1");          // automation.dfm:555
    CHECK(S(H()->edtHANARMSPort->Text) == "6670");             // :563
    CHECK(S(H()->edtHANARMSManCmd->Text) == "");
    CHECK(S(H()->lblRMS_Status->Caption) == "Disconnected");   // :535
    CHECK(H()->bHANARMSNeedRunCheck == false);                 // golden ctor :148
    CHECK(MemoCount() == 0);
    CHECK(!IsDir(logDir));
    WriteAll(iniFile, "[Setting]\r\nIP=10.20.30.40\r\nPort=14140\r\n");
    {
        CUSTOMER_CODE = 0;                                     // not CC_HANA_MICRON
        IniConfig.bA77_EnableHanaRMSInterlock = true;           // bReadFromFile on every machine (IniConfig.gen.inc:3888)
        LastSet.iRunStartMode = 0;
        H()->bHANARMSNeedRunCheck = true;
        const int smm0 = W906_ShowMyMessage_Count;
        CHECK(H()->HANARMSRunCheckOK(true) == true);           // golden :3139-3142
        CHECK(H()->bHANARMSNeedRunCheck == true);              // untouched
        CHECK(W906_ShowMyMessage_Count == smm0);
        CHECK(MemoCount() == 0);
        H()->bHANARMSNeedRunCheck = false;
    }
    {
        CUSTOMER_CODE = CC_HANA_MICRON;
        IniConfig.bA77_EnableHanaRMSInterlock = false;
        IniConfig.bA10_6_HANA_ART_TestMode_Enable = false;
        H()->PrepareHANARMSConnect();                          // golden :3053-3054
        CHECK(S(H()->edtHANARMSIP->Text) == "127.0.0.1");      // the ini (IP=10.20.30.40) was not read
        CHECK(H()->HANARMSClient == NULL);
        CHECK(MemoCount() == 0);
        CHECK(!IsDir(logDir));
        H()->HANARMSQueryJobInfo("LOTX");                      // golden :3106-3107
        CHECK(S(H()->sHANARMSLot) == "");
        H()->bHANARMSNeedRunCheck = true;
        CHECK(H()->HANARMSRunCheckOK(true) == true);
        H()->bHANARMSNeedRunCheck = false;
        W906_HanaRmsPumpTick();
        CHECK(H()->HANARMSClient == NULL);
        CHECK(MemoCount() == 0);
        IniConfig.bA77_EnableHanaRMSInterlock = true;
        LastSet.iRunStartMode = rsmAutoSiteMap;
        H()->bHANARMSNeedRunCheck = true;
        CHECK(H()->HANARMSRunCheckOK(true) == true);           // ASM always passes (golden :3141)
        CHECK(MemoCount() == 0);
        H()->bHANARMSNeedRunCheck = false;
        LastSet.iRunStartMode = 0;
    }

    // ---- 2. HANA + A77: placeholder -------------------------------------------------------------------------------
    std::printf("\n-- 2. A77 on, A10-6 off, 127.0.0.1 --\n");
    ::DeleteFileA(iniFile.c_str());
    H()->PrepareHANARMSConnect();
    CHECK(MemoCount() == 1 && EndsWith(MemoLine(0), " [WARN] A76 on but A10-6(HANA ART) off : first-shot interlock inactive"));   // golden :3056 (K7)
    CHECK(S(H()->edtHANARMSIP->Text) == "127.0.0.1" && S(H()->edtHANARMSPort->Text) == "6670");   // golden :3045-3046 defaults (K3)
    CHECK(H()->HANARMSClient == NULL);                          // golden :3059-3060
    {
        std::string day;
        CHECK(ReadAll(dayFile, day) && Has(day, "[WARN] A76 on but A10-6(HANA ART) off"));   // golden :2527-2531 (day file in the sandbox)
    }
    IniConfig.bA10_6_HANA_ART_TestMode_Enable = true;
    WriteAll(iniFile, "[Setting]\r\nIP=127.0.0.1\r\nPort=14140\r\n");
    int m0 = MemoCount();
    H()->PrepareHANARMSConnect();
    CHECK(MemoCount() == m0);                                   // no WARN, still placeholder
    CHECK(H()->HANARMSClient == NULL);
    CHECK(S(H()->edtHANARMSPort->Text) == "14140");

    // ---- 3. connect ------------------------------------------------------------------------------------------------
    std::printf("\n-- 3. connect (Sim socket) --\n");
    WriteAll(iniFile, "[Setting]\r\nIP=10.20.30.40\r\nPort=14140\r\n");
    m0 = MemoCount();
    H()->PrepareHANARMSConnect();
    CHECK(H()->HANARMSClient != NULL);
    if (H()->HANARMSClient == NULL) { std::printf("  ABORT: no socket\n"); return 1; }
    CHECK(H()->HANARMSClient->IsSimMode());                     // never the network
    if (!H()->HANARMSClient->IsSimMode()) { std::printf("  ABORT: the socket is not Sim -- stop before anything is sent\n"); return 1; }
    CHECK(H()->HANARMSClient->IsPolled());                      // [W906] 1
    CHECK(S(H()->HANARMSClient->Address) == "10.20.30.40" && H()->HANARMSClient->Port == 14140);   // golden :3065-3066
    CHECK((bool)H()->HANARMSClient->Active);
    CHECK(MemoHasSince(m0, " [INFO] connecting to 10.20.30.40:14140"));   // golden :3067
    CHECK(MemoHasSince(m0, " [INFO] connected"));                        // golden :2844
    CHECK(S(H()->lblRMS_Status->Caption) == "Connected");
    CHECK(H()->bW906HANARMSConnecting == false);
    m0 = MemoCount();
    H()->PrepareHANARMSConnect();                               // golden :3062
    CHECK(MemoCount() == m0);
    W906_HanaRmsPumpTick();                                      // Sim Poll: nothing
    CHECK(MemoCount() == m0);

    // ---- 4. send ---------------------------------------------------------------------------------------------------
    std::printf("\n-- 4. send --\n");
    ClearTx();
    H()->HANARMSSendCmd("  CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\"  ");
    CHECK(Tx() == "CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\"\n");   // golden :3080 / :3090
    CHECK(EndsWith(MemoLine(MemoCount() - 1), " [TX] CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\""));
    m0 = MemoCount(); ClearTx();
    H()->HANARMSSendCmd("   ");
    CHECK(MemoCount() == m0 && Tx().empty());                   // golden :3081-3082
    H()->btnRMS_AliveClick(NULL);
    CHECK(S(H()->edtHANARMSManCmd->Text) == "CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\"");   // golden :2991
    H()->btnHANARMSManCmdClick(NULL);
    CHECK(Tx() == "CMD=\"ALIVECHK\" LOOPBACK=\"TEST\" CHECKSTR=\"END\"\n");   // golden :2954

    // ---- 5. receive framing ----------------------------------------------------------------------------------------
    std::printf("\n-- 5. receive --\n");
    m0 = MemoCount();
    Push("CMD=\"ALIVECHK_REP\" LOOPBACK=\"TE");
    CHECK(MemoCount() == m0);                                   // no '\n' yet
    Push("ST\" CHECKSTR=\"END\"\r\n");
    CHECK(MemoCount() == m0 + 1 && EndsWith(MemoLine(m0), " [RX] CMD=\"ALIVECHK_REP\" LOOPBACK=\"TEST\" CHECKSTR=\"END\""));   // CR stripped (golden :2876-2877)
    if (MemoCount() != m0 + 1) DumpMemoSince(m0);
    m0 = MemoCount();
    Push("\r\n  \r\n");
    CHECK(MemoCount() == m0);                                   // golden :2879-2880
    Push("CMD=\"EVENT_REPORT_REP\" CMD_OK=\"YES\"\n");
    CHECK(MemoCount() == m0 + 2 && Has(MemoLine(m0), " [RX] CMD=\"EVENT_REPORT_REP\"") && EndsWith(MemoLine(m0 + 1), " [WARN] reply without CHECKSTR=\"END\""));   // golden :2901-2902 (K6)
    m0 = MemoCount();
    Push(std::string(9000, 'A'));
    CHECK(MemoCount() == m0);
    Push("CMD=\"ALIVECHK_REP\" CHECKSTR=\"END\"\n");
    CHECK(MemoCount() == m0 + 1 && EndsWith(MemoLine(m0), " [RX] CMD=\"ALIVECHK_REP\" CHECKSTR=\"END\""));   // garbage dropped (golden :2905-2906)
    if (MemoCount() != m0 + 1) DumpMemoSince(m0);

    // ---- 6. auto chain + compare ------------------------------------------------------------------------------------
    std::printf("\n-- 6. LOT_INFO -> JOB_INFO -> [CMP] PASS --\n");
    LastSet.iTemperature = Tempture_Hot;   Temperature.fWorkTemperBase = 85.0;   Temperature.bUseAbitCHK = false;   Temperature.fAbitTemp = 0;
    DeviceForm_File.iPinCT = 48;  DeviceForm_File.ForcePerPinG = 25.5;  DeviceForm_File.ContactMode = TMoveSlowContact;   // index 9
    TestIF.iSlidingWindowSize = 100;  TestIF.iContsFailSocketAlarmCT = 3;  TestIF.dFailAlarmSiteYield = 80;  TestIF.dLowYieldLimit = 70;
    TestIF.bLowYieldAutoSiteOff = true;  TestIF.iContsFailSocketAlarmCT_RT = 4;  TestIF.dFailAlarmSiteYield_RT = 75;  TestIF.dLowYieldLimit_RT = 65;
    for (int k = 0; k < 2; ++k)
        for (int i = 0; i < TEST_MAX_BIN; ++i) {
            BinSelect[binIdx[k]].bSpecialBinByArm[i] = false;   BinSelect[binIdx[k]].bSpecialBinBySocket[i] = false;
            BinSelect[binIdx[k]].iSpecialBinCountByArm[i] = 0;  BinSelect[binIdx[k]].iSpecialBinCountBySocket[i] = 0;
        }
    BinSelect[eBinFT].bSpecialBinByArm[3] = true;  BinSelect[eBinFT].iSpecialBinCountByArm[3] = 2;
    BinSelect[eBinFT].bSpecialBinByArm[7] = true;  BinSelect[eBinFT].iSpecialBinCountByArm[7] = 5;
    ClearTx();
    H()->HANARMSQueryJobInfo("  LOT123 ");
    CHECK(S(H()->sHANARMSLot) == "LOT123");
    CHECK(Tx() == "CMD=\"GET_LOT_INFO\" LOT=\"LOT123\" HDNAME=\"HD_01\" CHECKSTR=\"END\"\n");   // golden :3112-3113 (K2)
    std::printf("    tx = %s", Tx().c_str());
    ClearTx();
    Push("CMD=\"GET_LOT_INFO_REP\" LOT=\"LOT123\" CMD_OK=\"YES\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"PN_1\" CHECKSTR=\"END\"\n");
    CHECK(Tx() == "CMD=\"GET_JOB_INFO\" LOT=\"LOT123\" HDNAME=\"HD_01\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"PN_1\" CHECKSTR=\"END\"\n");   // golden :2887-2897
    std::printf("    tx = %s", Tx().c_str());
    m0 = MemoCount();
    Push(JobLine() + "\n");
    CHECK(MemoHasSince(m0, "   RCP_Name      : C10_RECIPE"));          // golden :2799 / :2783
    CHECK(MemoHasSince(m0, "   GB_CT         : (not used)"));          // golden :2780
    CHECK(MemoHasSince(m0, "   PIN_FC        : 25.5 (gf)"));
    CHECK(MemoHasSince(m0, "   FT.S_T_S      : 80"));                  // dot key -> underscore + spacing (golden :2546-2592)
    CHECK(MemoHasSince(m0, "   GET_RDY       : YES"));
    CHECK(EndsWith(MemoLine(MemoCount() - 1), "   [CMP] PASS"));       // golden :2833
    if (!EndsWith(MemoLine(MemoCount() - 1), "   [CMP] PASS")) DumpMemoSince(m0);
    CHECK(S(H()->sHANARMSJobInfoLine) == JobLine());                     // golden :2830
    // HDName from the ini when set
    WriteAll(iniFile, "[Setting]\r\nIP=10.20.30.40\r\nPort=14140\r\nHDName=HT9046-15\r\n");
    ClearTx();
    Push("CMD=\"GET_LOT_INFO_REP\" LOCATION=\"L2\" CUSTCD=\"C2\" PARTNO=\"P2\" CHECKSTR=\"END\"\n");
    CHECK(Tx() == "CMD=\"GET_JOB_INFO\" LOT=\"LOT123\" HDNAME=\"HT9046-15\" LOCATION=\"L2\" CUSTCD=\"C2\" PARTNO=\"P2\" CHECKSTR=\"END\"\n");
    WriteAll(iniFile, "[Setting]\r\nIP=10.20.30.40\r\nPort=14140\r\n");

    // ---- 7. interlock ---------------------------------------------------------------------------------------------
    std::printf("\n-- 7. HANARMSRunCheckOK / HANARMSCheckRecipe --\n");
    {
        std::string f;
        H()->sHANARMSJobInfoLine = AnsiString(JobLine().c_str());
        H()->bHANARMSNeedRunCheck = true;
        int smm0 = W906_ShowMyMessage_Count;
        m0 = MemoCount();
        CHECK(H()->HANARMSRunCheckOK(true) == true);
        CHECK(H()->bHANARMSNeedRunCheck == false);                          // golden :3150
        CHECK(MemoHasSince(m0, " [CHK] recipe check PASS"));                // golden :3130
        CHECK(W906_ShowMyMessage_Count == smm0);
        m0 = MemoCount();
        CHECK(H()->HANARMSRunCheckOK(true) == true);                        // latch false (golden :3144-3145)
        CHECK(MemoCount() == m0);

        DeviceForm_File.ForcePerPinG = 17;
        H()->bHANARMSNeedRunCheck = true;
        m0 = MemoCount();
        CHECK(H()->HANARMSRunCheckOK(true) == false);
        CHECK(H()->bHANARMSNeedRunCheck == true);                           // FAIL keeps the latch
        CHECK(MemoHasSince(m0, " [CHK] recipe check FAIL : PIN_FC(S=25.5/H=17)"));
        CHECK(MemoHasSince(m0, " [CMP] RunCheck FAIL : PIN_FC(S=25.5/H=17)"));   // golden :3154
        CHECK(W906_ShowMyMessage_Count == smm0 + 1);
        CHECK(S(W906_ShowMyMessage_LastS1) == "HANA RMS recipe check FAIL!!\r\nPIN_FC(S=25.5/H=17)");   // golden :3156
        if (S(W906_ShowMyMessage_LastS1) != "HANA RMS recipe check FAIL!!\r\nPIN_FC(S=25.5/H=17)") std::printf("    LastS1 = %s\n", W906_ShowMyMessage_LastS1.c_str());
        CHECK(H()->HANARMSRunCheckOK(false) == false);                      // silent (golden :3155)
        CHECK(W906_ShowMyMessage_Count == smm0 + 1);
        DeviceForm_File.ForcePerPinG = 25.5;
        H()->bHANARMSNeedRunCheck = false;

        H()->sHANARMSJobInfoLine = "";
        AnsiString asFail;
        CHECK(H()->HANARMSCheckRecipe(asFail) == false && S(asFail) == "RMS JOB_INFO not received");   // golden :3120-3124

        CHECK(CheckLine(JobLine("CT05"), f) == false && f == "GB_CT(S=CT05/H=(N/A))");   // [W906] 4: GB_CT gated => FAIL
        if (f != "GB_CT(S=CT05/H=(N/A))") std::printf("    fail = %s\n", f.c_str());
        CHECK(CheckLine(JobLine(""), f) == false && f == "GB_CT(S=/H=(N/A))");           // even an empty value
        CHECK(CheckLine(JobLine("-"), f) == true && f == "");                             // "-" = not used (golden :2638)
        CHECK(CheckLine(JobLine("-", "DIRECT"), f) == false && f == "CON_MTD(S=DIRECT/H=RELEASE)");   // index 9 = RELEASE (K4)
        DeviceForm_File.ContactMode = DirectContactMode;                                   // 0: DIRECT, LOW_SPD "-"
        CHECK(CheckLine(JobLine("-", "DIRECT"), f) == false && f == "LOW_SPD(S=E/H=-)");
        DeviceForm_File.ContactMode = TMoveSlowContact;
        const int room = (Tempture_Ambient != Tempture_Hot && Tempture_Ambient != Tempture_AmbientHot) ? Tempture_Ambient : 2;
        LastSet.iTemperature = room;
        CHECK(CheckLine(JobLine(), f) == false && f == "TEMP(S=85/H=RoomTemp)");          // golden :2703-2708
        Temperature.bUseAbitCHK = true;  Temperature.fAbitTemp = 85;
        CHECK(CheckLine(JobLine(), f) == true);                                            // golden :2701-2702
        Temperature.bUseAbitCHK = false; Temperature.fAbitTemp = 0;
        LastSet.iTemperature = Tempture_Hot;
        CHECK(CheckLine("CMD=\"GET_JOB_INFO_REP\" RCP_NAME=\"C10_RECIPE\" CHECKSTR=\"END\"", f) == true);   // fields not sent = skipped
        CHECK(CheckLine("RCP_NAME=\"OTHER\"", f) == false && f == "RCP_NAME(S=OTHER/H=C10_RECIPE)");
    }

    // ---- 8. 912 known defects, as 912 ------------------------------------------------------------------------------
    std::printf("\n-- 8. 912 defects --\n");
    {
        // golden :3108-3109 verbatim (RogerYang "換批,舊基準作廢"): every lot-open query voids the baseline, same lot too
        H()->sHANARMSJobInfoLine = AnsiString(JobLine().c_str());
        ClearTx();
        H()->HANARMSQueryJobInfo("LOT123");                                    // same lot
        CHECK(S(H()->sHANARMSJobInfoLine) == "");                               // golden :3109 on every call (no [W906] 6)
        CHECK(Has(Tx(), "CMD=\"GET_LOT_INFO\" LOT=\"LOT123\""));              // asked again
        H()->sHANARMSJobInfoLine = AnsiString(JobLine().c_str());
        H()->HANARMSQueryJobInfo("LOT999");                                    // new lot
        CHECK(S(H()->sHANARMSJobInfoLine) == "" && S(H()->sHANARMSLot) == "LOT999");
        ClearTx();
        H()->HANARMSQueryJobInfo("");
        CHECK(S(H()->sHANARMSLot) == "" && Tx().empty());                     // golden :3110-3111

        // K1 / K5: the server's 30 s idle close; nothing reconnects; [TX] is logged anyway
        m0 = MemoCount();
        H()->HANARMSClient->Close();
        CHECK(MemoHasSince(m0, " [INFO] disconnected"));                        // golden :2853
        CHECK(S(H()->lblRMS_Status->Caption) == "Disconnected");
        CHECK(!(bool)H()->HANARMSClient->Active);
        m0 = MemoCount(); ClearTx();
        H()->HANARMSSendCmd("CMD=\"GET_LOT_INFO\" LOT=\"LOT7\" HDNAME=\"HD_01\" CHECKSTR=\"END\"");
        CHECK(MemoCount() == m0 + 2 && EndsWith(MemoLine(m0), " [TX] CMD=\"GET_LOT_INFO\" LOT=\"LOT7\" HDNAME=\"HD_01\" CHECKSTR=\"END\"")
              && EndsWith(MemoLine(m0 + 1), " [WARN] not connected, not sent"));   // golden :3084 / :3099
        CHECK(Tx().empty());
        m0 = MemoCount();
        W906_HanaRmsPumpTick(); W906_HanaRmsPumpTick(); W906_HanaRmsPumpTick();
        CHECK(!(bool)H()->HANARMSClient->Active && MemoCount() == m0);

        // K2 / K8: the button templates, verbatim
        H()->btnRMS_GetLotClick(NULL);
        CHECK(S(H()->edtHANARMSManCmd->Text) == "CMD=\"GET_LOT_INFO\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" CHECKSTR=\"END\"");   // golden :2967
        H()->btnRMS_GetJobClick(NULL);
        CHECK(S(H()->edtHANARMSManCmd->Text) == "CMD=\"GET_JOB_INFO\" LOT=\"TLOTNO\" HDNAME=\"HD_01\" LOCATION=\"T9999\" CUSTCD=\"CS\" PARTNO=\"XXX_XXXX_XXX_XXX\" CHECKSTR=\"END\"");
        H()->btnRMS_EventClick(NULL);
        CHECK(Has(S(H()->edtHANARMSManCmd->Text), "PJ_FG=\"YES\" PJ_CD=\"10\" COMMENT=\"-\" CHECKSTR=\"END\""));
        H()->btnRMS_GetTimeClick(NULL);
        CHECK(S(H()->edtHANARMSManCmd->Text) == "CMD=\"GET_TIME_INFO\" HDNAME=\"HD_01\" CHECKSTR=\"END\"");
    }

    // ---- 9. error event ---------------------------------------------------------------------------------------------
    std::printf("\n-- 9. OnError --\n");
    {
        int code = 10061;
        m0 = MemoCount();
        H()->bW906HANARMSConnecting = true;
        H()->HANARMSClient->OnError(H()->HANARMSClient, H()->HANARMSClient->Socket, Scktcomp::eeConnect, code);
        CHECK(code == 0);                                                       // golden :2914
        CHECK(MemoHasSince(m0, " [ERR] socket error 10061"));
        CHECK(S(H()->lblRMS_Status->Caption) == "Disconnected");
        CHECK(H()->bW906HANARMSConnecting == false);
    }

    // ---- 10. RogerYang :3062 "連線中" ([W906] 5) ------------------------------------------------------------------------
    std::printf("\n-- 10. a pending connect is not restarted --\n");
    {
        H()->bW906HANARMSConnecting = true;                                     // as a REAL polled Open() leaves it until Poll()
        m0 = MemoCount();
        H()->PrepareHANARMSConnect();
        CHECK(!(bool)H()->HANARMSClient->Active && !MemoHasSince(m0, "connecting to"));
        H()->bW906HANARMSConnecting = false;
        H()->PrepareHANARMSConnect();                                            // only an explicit second Prepare reconnects
        CHECK((bool)H()->HANARMSClient->Active && MemoHasSince(m0, " [INFO] connecting to 10.20.30.40:14140") && MemoHasSince(m0, " [INFO] connected"));
    }

    // ---- 11. button bodies that write files; the memo trim ------------------------------------------------------------
    std::printf("\n-- 11. Setting / Save / Clear / trim --\n");
    {
        H()->edtHANARMSIP->Text = "10.9.8.7";
        H()->edtHANARMSPort->Text = "14141";
        H()->btnHANARMSSettingClick(NULL);                                       // golden :3016-3017
        CloseIniFile();
        std::string ini;
        CHECK(ReadAll(iniFile, ini) && Has(ini, "IP=10.9.8.7") && Has(ini, "Port=14141"));
        if (!Has(ini, "IP=10.9.8.7")) std::printf("    ini = %s\n", ini.c_str());
        H()->edtHANARMSIP->Text = "x";
        H()->LoadHANARMSSetting();
        CHECK(S(H()->edtHANARMSIP->Text) == "10.9.8.7" && S(H()->edtHANARMSPort->Text) == "14141");
        const std::string saved = logDir + "\\HANARMS_20991231_235958.log";     // golden :2999-3001
        H()->btnSaveHANARMSLogClick(NULL);
        CHECK(Exists(saved));
        CHECK(EndsWith(MemoLine(MemoCount() - 1), " [INFO] log saved: " + saved));
        H()->btnRMS_ClearLogClick(NULL);
        CHECK(MemoCount() == 0);                                                 // golden :2961
        for (int i = 0; i < 1001; ++i) w906hanarms::HANARMSAddLog(H()->mmoHANARMS, "T ", Fmt("%d", i).c_str());
        CHECK(MemoCount() == 1001);
        w906hanarms::HANARMSAddLog(H()->mmoHANARMS, "T ", "last");
        CHECK(MemoCount() == 1 && MemoLine(0) == "T last");                     // golden :2537-2539
        std::string day;
        CHECK(ReadAll(dayFile, day) && Has(day, "[INFO] connected") && Has(day, "T last") && Has(day, "[CHK] recipe check FAIL : PIN_FC(S=25.5/H=17)"));
    }

    // ---- 12. source pins ---------------------------------------------------------------------------------------------
    std::printf("\n-- 12. source pins --\n");
    {
        std::vector<std::string> hbc, tcw, ate, shim;
        CHECK(ReadLines("TesterComm/Handler/HandlerBridgeCtl.cpp", hbc));
        int g9 = -1;
        for (size_t i = 0; i < hbc.size(); ++i) if (Has(hbc[i], "G9 -- PrepareHANARMSConnect is not a member")) { g9 = (int)i; break; }
        CHECK(g9 > 0 && hbc[g9].compare(0, 7, "//#if 0") == 0);                 // the gate line is a comment now
        if (g9 > 0 && g9 + 6 < (int)hbc.size()) {
            std::string body;
            for (int i = g9 + 1; i <= g9 + 5; ++i) body += StripComments(hbc[i]) + "\n";
            CHECK(Has(body, "if(fAutomation &&") && Has(body, "CUSTOMER_CODE==CC_HANA_MICRON)") && Has(body, "fAutomation->PrepareHANARMSConnect();"));
            CHECK(hbc[g9 + 6].compare(0, 8, "//#endif") == 0);
            CHECK(Has(StripComments(hbc[g9 - 1]), "{") && Has(StripComments(hbc[g9 - 2]), "if(iProgramReady!=1)"));   // golden main.cpp:18533-18534
        }
        bool inc = false;
        for (size_t i = 0; i < hbc.size() && !inc; ++i) if (StripComments(hbc[i]).compare(0, 26, "#include \"atester_shims.h\"") == 0) inc = true;
        CHECK(inc);                                                              // fAutomation for G9
        CHECK(ReadLines("TesterComm/Handler/TesterCommWiring.cpp", tcw));
        int pump = 0, policy = 0;
        for (size_t i = 0; i < tcw.size(); ++i) {
            const std::string c = StripComments(tcw[i]);
            if (Has(c, "W906_HanaRmsPumpTick();")) ++pump;
            if (Has(c, "W906_HanaRmsSetRealSocket(W906_TcpTesterRealSocket());")) ++policy;
        }
        CHECK(pump == 2 && policy == 1);                                         // Tick + Poll; Init
        CHECK(ReadLines("atester.cpp", ate));
        bool t08 = false;
        for (size_t i = 0; i < ate.size(); ++i) if (ate[i].compare(0, 31, "#if 0 // TODO(W906-GB-P2b): T08") == 0) t08 = true;
        CHECK(t08);                                                              // plan Q5: the 3b hooks wait for HANA ART
        CHECK(ReadLines("atester_shims.h", shim));
        bool base = false;
        for (size_t i = 0; i < shim.size(); ++i) if (StripComments(shim[i]).find("class TfAutomationShim : public THanaRmsMembers") == 0) base = true;
        CHECK(base);
    }

    // ---- restore, containment proof, cleanup ---------------------------------------------------------------------
    if (H()->HANARMSClient != NULL) H()->HANARMSClient->Close();
    CloseIniFile();
    as9045LogPath = sav_LogPath;  AuthPath = sav_AuthPath;  LastDataPath = sav_LastDataPath;
    SystemYear = sav_Y; SystemMonth = sav_M; SystemDate = sav_D; SystemHour = sav_h; SystemMin = sav_m; SystemSec = sav_s;
    CUSTOMER_CODE = sav_CC;
    IniConfig.bA77_EnableHanaRMSInterlock = sav_A77;  IniConfig.bA10_6_HANA_ART_TestMode_Enable = sav_A106;
    LastSet.iRunStartMode = sav_RunStart;  LastSet.iTemperature = sav_Temp;
    Temperature.fWorkTemperBase = sav_fWork;  Temperature.fAbitTemp = sav_fAbit;  Temperature.bUseAbitCHK = sav_bAbit;
    DeviceForm_File.iPinCT = sav_PinCT;  DeviceForm_File.ContactMode = sav_Contact;  DeviceForm_File.ForcePerPinG = sav_PinFC;
    TestIF.iSlidingWindowSize = sav_Win;  TestIF.iContsFailSocketAlarmCT = sav_CF;  TestIF.iContsFailSocketAlarmCT_RT = sav_CFRT;
    TestIF.dFailAlarmSiteYield = sav_STS;  TestIF.dFailAlarmSiteYield_RT = sav_STSRT;  TestIF.dLowYieldLimit = sav_LY;  TestIF.dLowYieldLimit_RT = sav_LYRT;
    TestIF.bLowYieldAutoSiteOff = sav_AtOff;
    for (int k = 0; k < 2; ++k)
        for (int i = 0; i < TEST_MAX_BIN; ++i) {
            BinSelect[binIdx[k]].bSpecialBinByArm[i] = sav_bArm[k][i];   BinSelect[binIdx[k]].bSpecialBinBySocket[i] = sav_bSck[k][i];
            BinSelect[binIdx[k]].iSpecialBinCountByArm[i] = sav_cArm[k][i];  BinSelect[binIdx[k]].iSpecialBinCountBySocket[i] = sav_cSck[k][i];
        }
    H()->sHANARMSLot = "";  H()->sHANARMSJobInfoLine = "";  H()->bHANARMSNeedRunCheck = false;

    std::printf("\n-- containment: the machine's HANA RMS files untouched --\n");
    CHECK(SameStamp(StampOf(kRealIni), realIni0));
    CHECK(SameStamp(StampOf(kRealLogDir), realDir0));
    CHECK(Exists(kRealDayFile) == realDayFile0);
    CHECK(Exists(dayFile));                                                      // everything went to the sandbox

    std::printf("\n%d / %d checks passed\n", g_total - g_fail, g_total);
    if (g_fail == 0) {
        if (!RemoveTree(root)) std::printf("  note: could not remove %s\n", root.c_str());
    } else {
        std::printf("  sandbox kept for inspection: %s\n", root.c_str());
    }
    return g_fail == 0 ? 0 : 1;
}
