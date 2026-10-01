// =============================================================================
//  tests/test_g023_os_report.cpp -- todo G-023 / work card ST02-C1: golden TfTesterTCP's Open / Short report trio
//  (golden 906_0625_Steven Interface/TesterTCP.cpp:699-739 PlaceOSTestResultToTray, :741-962 ProcessOSPrint,
//  :964-1056 ProcessOSTrayData; V912 identical) as the free functions in Interface/TesterTCP_OSReport.cpp, and their two
//  callers (aoutarm9045.cpp:4757, Automation/SCK_ART_Remainder.cpp:243 gate #6).  AI(W906-G023) 20261001 (St02-E).
//  Suite: G023_OSReport.  Claim doc: ST02_G023_CLAIMS_20261001.md section 6.1.
//
//  0. CONTAINMENT FIRST (the b12ab375 rule): refuses (exit 2) before any Handler code unless W906_HT9045LOG_ROOT and the four
//     log roots (as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath) are ctest's
//     machine_log_scratch (tests/CMakeLists.txt _ht9045_env_extra).  Everything the bodies write goes into a private folder
//     <machine_log_scratch>\G023_OSReport_<tick>: as9045LogPath, sJamRatePath (common.cpp:307 is NOT on the seam),
//     IniConfig.sB05_OSReportPath (the network share -> a local folder), asSummaryPath and IniConfig.sN09_HandlerFolder.
//     The machine's own D:\HT9045_Log OS / JamRate files are stamped before and compared after (read only).
//  1. PlaceOSTestResultToTray, not TCP_IP_MODE: returns at once, no folder is made.
//  2. PlaceOSTestResultToTray, TCP_IP_MODE, source present: the Device file is read and DELETED; the tray's TestReport gets
//     golden's "Device / X / Y / Tested" line (Tested = iShtRow*iShtCol*contact + site) and the source lines, CRLF.
//  3. Source missing: "can not find file <path>" is appended; the report keeps growing (plus Auto2 / Fix1 reports).
//  4. ProcessOSPrint(false) through golden's real UpdataCount (ArmDataLot seeded as test_SCK_ART_Remainder PART 13):
//     OS_Summary is golden's RTF, byte for byte (header, \par lines, \ { } escaped, Big5 as \'hh, closing \par } + NUL);
//     the BY SITE columns are numbers (P4: StringsProxy wrapped); OS_Pin_All / OS_Pin_<tray> exact (Big5 Fail Pin header);
//     an unused tray gets nothing, a Fix tray is not saved locally, an empty Auto still gets its report, the TestReports
//     are emptied; no B05 copy, no JamRate (bLotStart false).
//  5. B05 on: only Auto2 / Auto3 are copied to the (local, sandboxed) share folder, same bytes as the local copy.
//  6. ProcessOSPrint(true): no report files, but the TestReports are still emptied (golden :1042-1046, outside bViewOnly).
//  7. SIM only: a UNC B05 path is skipped by W906_SimNetPathBlocked (W58 Q5) -- the string check, no network is touched;
//     the local reports are still written.  SHIP: skipped (golden would write the share).
//  8. RunInfo.SaveJamRateByLot (golden :961) with bLotStart: the JamRate file lands in the redirected sJamRatePath.
//  9. The dispatcher: SckArtRem_SaveTestSummary(st,0) writes no OS report; W906_SckArt_SaveTestSummary(1) (the Lot End path,
//     forms/fLotInfo.cpp SetLotEnd) writes one through gate #6 and no longer lists ProcessOSPrint as skipped.  The TSV branch
//     is steered as test_SCK_ART_Remainder PART 8 does (bUseTSVFunction + sN09_HandlerFolder / asSummaryPath redirected),
//     so its golden literal D:\HT9045_Log\TestSummary is never reached.
// 10. Ratchet (St01 28ad5b17): SCK_ART_SaveTestSummary.cpp's old `if (TCP_IP_MODE && iSaveData)` (:56) and its only body
//     (:57) are both comments now, so the next statement -- `if (CosFunction.bUseTSVFunction) W906_SckArtSkip(...)` (:58-59) --
//     must still run.  (a) in memory: a TTL / GPIB / RS232 tester with bUseTSVFunction gets exactly one skipped entry, the
//     ATK TSV one (iSaveData 0: no SaveSummaryTrayFeed; bUseTSVFunction keeps SaveTestSummaryTSV on the redirected
//     sN09_HandlerFolder -- with it off TSV would mkdir golden's literal D:\HT9045_Log\TestSummary, so the "off" case is
//     NOT driven); (b) in the source, comments stripped, anchored by text: the code right after `LotSummary.ClearAllData();`
//     is that `if` and that skip call, then the function's closing brace.
// 11. Source pins: the two callers are live (not under #if 0) and call the new functions.
// 12. AI(W906-U1) 20261002 (St02-E): SaveSummaryTrayFeed's BY SITE rows (G-023 group C, SCK_ART_Remainder.cpp:2657-2659;
//     golden SCK_ART.cpp:3253-3255) -- the summary folder (W906_SUMMARYLOT_ROOT) points into the private folder; per-site
//     counts seeded in ArmDataLot (the function's real TastCategory.UpdataCount(false) rebuilds them from there) come out as
//     the numbers in golden's " %-20s" columns, then (U4) a disabled site and a site with no count print golden's defaults
//     "0" / "0(0.00%)".  On a mismatch the BY SITE lines as written are printed.  N10 off, bLotStart false: nothing else
//     is written.
//  Every global it changes is saved and restored; the private folder is removed on a green run.
// =============================================================================
#include "Interface/TesterTCP_OSReport.h"
#include "Automation/SCK_ART_Remainder.h"        // SckArtRemainderState, SckArtRem_SaveTestSummary
#include "Automation/SCK_ART_SaveTestSummary.h"  // W906_SckArt_SaveTestSummary
#include "MachineDefine.h"
#include "MachineType.h"         // eTrayCount, eAuto1..eFix1, tNotUse / tTrayAuto / tTrayFix, SOFT_SIMULTE
#include "cprod.h"               // TestIF_File, TestIF, Prod, RunInfo, IniConfig
#include "cmydef.h"              // TCP_IP_MODE, s6TrayName, iOneTrayPickCount, iTestBinCount, SystemYear..SystemSec, bWaitTSV
#include "common.h"              // as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath, sJamRatePath,
                                 // asSummaryPath, W906_SimNetPathBlocked
#include "CosFunction.h"         // CosFunction
#include "canary_support.h"
#include "aHotPlateSubstrate.h"  // OutArmSuck, TestSocket
#include "cSocket.h"             // TastCategory, TArm, ArmDataLot
#include "FormsFacade.h"         // fLotInfo, fMain
#include "forms/fTesterTCP.h"    // fTesterTCP->SummaryHead
#include "Public/MyProductionRecord.h"   // eSiteNO, eOrderTest

#include <windows.h>
#include <cstdio>
#include <cstdlib>
// AI(W906-U1) 20261002: _putenv -- MinGW.org declares no POSIX / _ names in strict mode (CXX_EXTENSIONS OFF); the tree's
//   pattern, tests/test_agv_e84.cpp:158-163.  A CRT call on purpose: the code under test reads the CRT copy with getenv.
#if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
extern "C" int _putenv(const char*);
#define HT9045_TEST_PUTENV _putenv
#else
#define HT9045_TEST_PUTENV putenv
#endif
#include <cstdarg>
#include <cstring>
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
static long SizeOf(const std::string& p) { std::string s; return ReadAll(p, s) ? (long)s.size() : -1; }
static void WriteAll(const std::string& p, const std::string& bytes)
{
    std::ofstream f(p.c_str(), std::ios::binary);
    f << bytes;
}
static std::string Lines(const std::vector<std::string>& v)          // TStrings::SaveToFile: every line + CRLF
{
    std::string s;
    for (size_t i = 0; i < v.size(); ++i) s += v[i] + "\r\n";
    return s;
}
static std::string RtfEsc(const std::string& in)                       // P3: \ { } escaped, >= 0x80 as \'hh
{
    static const char hex[] = "0123456789abcdef";
    std::string out;
    for (size_t i = 0; i < in.size(); ++i) {
        const unsigned char c = (unsigned char)in[i];
        if (c == '\\' || c == '{' || c == '}') { out += '\\'; out += (char)c; }
        else if (c >= 0x80) { out += "\\'"; out += hex[c >> 4]; out += hex[c & 15]; }
        else out += (char)c;
    }
    return out;
}
static const char kRtfHead[]  = "{\\rtf1\\ansi\\ansicpg950\\deff0\\deflang1033\\deflangfe1028{\\fonttbl{\\f0\\fnil\\fcharset136 Courier New;}}\r\n";
static const char kRtfFirst[] = "\\viewkind4\\uc1\\pard\\lang1028\\f0\\fs16 ";
static std::string Rtf(const std::vector<std::string>& v)              // the BCB6 TRichEdit shape (claim doc section 1.2)
{
    std::string s = std::string(kRtfHead) + kRtfFirst;
    for (size_t i = 0; i < v.size(); ++i) { if (i > 0) s += "\\par "; s += RtfEsc(v[i]) + "\r\n"; }
    s += "\\par \r\n\\par }\r\n";
    s += '\0';
    return s;
}
static std::string Pct(long n, long d) { return d != 0 ? Fmt("%0.2f%%", ((double)n / (double)d) * 100.0) : std::string("0.00%"); }
static int CountFiles(const std::string& pattern)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return 0;
    int n = 0;
    do { if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) ++n; } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
    return n;
}
// read-only stamp of a machine path: exists / size / last write
static std::string Stamp(const std::string& p)
{
    WIN32_FILE_ATTRIBUTE_DATA a;
    if (!::GetFileAttributesExA(p.c_str(), GetFileExInfoStandard, &a)) return "absent";
    return Fmt("%lu/%lu/%lu/%lu", (unsigned long)a.nFileSizeLow, (unsigned long)a.ftLastWriteTime.dwHighDateTime,
               (unsigned long)a.ftLastWriteTime.dwLowDateTime, (unsigned long)a.dwFileAttributes);
}
static bool RemoveTree(const std::string& dir)                         // guarded: only our own scratch folder
{
    const std::string l = Lower(dir);
    if (l.find("machine_log_scratch") == std::string::npos || l.find("g023_osreport_") == std::string::npos) return false;
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((dir + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string n = fd.cFileName;
            if (n == "." || n == "..") continue;
            const std::string p = dir + "\\" + n;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(p); else ::DeleteFileA(p.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    return ::RemoveDirectoryA(dir.c_str()) != 0;
}
// line N of a source file is live: not inside any #if 0 (a linear scan, like tools/start_sites_census.py)
static bool LiveLineContaining(const std::string& rel, const std::string& needle)
{
    std::string text;
    if (!ReadAll(std::string(W906_SRC_ROOT) + "/" + rel, text)) return false;
    std::vector<int> stack;   // 1 = "#if 0" arm (dead), 2 = "#if 1" arm, 3 = the #else of "#if 1" (dead), 0 = anything else
    std::istringstream is(text);
    std::string line;
    while (std::getline(is, line)) {
        size_t b = line.find_first_not_of(" ");
        const std::string t = (b == std::string::npos) ? std::string() : line.substr(b);
        if (t.compare(0, 3, "#if") == 0) { stack.push_back(t.compare(0, 5, "#if 0") == 0 ? 1 : (t.compare(0, 5, "#if 1") == 0 ? 2 : 0)); continue; }
        if (t.compare(0, 5, "#else") == 0 || t.compare(0, 5, "#elif") == 0) {
            if (!stack.empty()) stack.back() = (stack.back() == 1) ? 0 : (stack.back() == 2 ? 3 : stack.back());
            continue;
        }
        if (t.compare(0, 6, "#endif") == 0) { if (!stack.empty()) stack.pop_back(); continue; }
        if (line.find(needle) == std::string::npos) continue;
        bool dead = false;
        for (size_t i = 0; i < stack.size(); ++i) if (stack[i] == 1 || stack[i] == 3) dead = true;
        const size_t c = line.find("//");
        if (!dead && (c == std::string::npos || c > line.find(needle))) return true;   // the code is before any comment
    }
    return false;
}
// the code lines of a source file: // and /* */ comments removed (string / char literals respected), each line trimmed,
// empty lines dropped
static std::vector<std::string> CodeLines(const std::string& rel)
{
    std::vector<std::string> out;
    std::string text;
    if (!ReadAll(std::string(W906_SRC_ROOT) + "/" + rel, text)) return out;
    std::string cur;
    bool block = false;
    char quote = 0;
    for (size_t i = 0; i < text.size(); ++i) {
        const char c = text[i], n = (i + 1 < text.size()) ? text[i + 1] : '\0';
        if (block) { if (c == '*' && n == '/') { block = false; ++i; } else if (c == '\n') { out.push_back(cur); cur.clear(); } continue; }
        if (quote) { cur += c; if (c == '\\' && n != '\0') { cur += n; ++i; } else if (c == quote) quote = 0; continue; }
        if (c == '/' && n == '/') { while (i < text.size() && text[i] != '\n') ++i; out.push_back(cur); cur.clear(); continue; }
        if (c == '/' && n == '*') { block = true; ++i; continue; }
        if (c == '"' || c == '\'') { quote = c; cur += c; continue; }
        if (c == '\n') { out.push_back(cur); cur.clear(); continue; }
        cur += c;
    }
    out.push_back(cur);
    std::vector<std::string> code;
    for (size_t i = 0; i < out.size(); ++i) {
        std::string s = out[i];
        while (!s.empty() && (s[s.size() - 1] == '\r' || s[s.size() - 1] == ' ')) s.erase(s.size() - 1);
        const size_t b = s.find_first_not_of(' ');
        if (b != std::string::npos) code.push_back(s.substr(b));
    }
    return code;
}
static int CountSkipped(TStringList& sk, const char* what)
{
    int n = 0;
    for (int i = 0; i < sk.Count; ++i) if (AnsiString(sk.Strings[i]).Pos(what) != 0) ++n;
    return n;
}
static void SeedRec(int iSiteNo, int iContact)
{
    OutArmSuck.PordRec[0][0].asBuffer->Strings[eSiteNO]    = AnsiString(iSiteNo);
    OutArmSuck.PordRec[0][0].asBuffer->Strings[eOrderTest] = AnsiString(iContact);
}
static void SetTime(int h, int m, int s) { SystemYear = 2026; SystemMonth = 10; SystemDate = 1; SystemHour = h; SystemMin = m; SystemSec = s; }

// ---- golden literals (Interface/TesterTCP.cpp; checked against the cp950 golden) ----------------------------------
static const char kDevFmt[]  = "===========================    Device:%d  X:%d  Y:%d  Tested:%d    ====================================";   // golden :730
static const char kTrayFmt[] = "===================================    %s    ==================================================";              // golden :1014
static const char kFailPin[] = "Fail Pin,,,,===============\xA4U\xAD\xAD,         \xA4W\xAD\xAD        ,\xB6q\xB4\xFA\xAD\xC8========";      // golden :1027, Big5
static const char kLot[]     = "W906G023LOT-DO-NOT-USE";

int main()
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("G023_OSReport\n");

    // ---- 0. containment first ------------------------------------------------------------------------------------
    {
        const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
        const char* const names[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
        bool contained = true;
        for (int i = 0; i < 4; ++i) {
            std::printf("  %s = %s\n", names[i], roots[i]->c_str());
            if (Lower(roots[i]->c_str()).find("machine_log_scratch") == std::string::npos) contained = false;
        }
        const char* v = std::getenv("W906_HT9045LOG_ROOT");
        std::printf("  W906_HT9045LOG_ROOT = %s\n", v ? v : "(unset)");
        if (Lower(v ? v : "").find("machine_log_scratch") == std::string::npos) contained = false;
        if (!contained) {
            std::printf("  ABORT: not inside ctest's redirect roots (run it with ctest -R G023_OSReport) -- nothing was called\n");
            return 2;
        }
    }
    const std::string root = Slashes(as9045LogPath.c_str()) + Fmt("\\G023_OSReport_%lu", (unsigned long)::GetTickCount());
    {
        const std::string l = Lower(root);
        if (l.find("machine_log_scratch") == std::string::npos || l.compare(0, 13, "d:\\ht9045_log") == 0) {
            std::printf("  ABORT: sandbox %s is not under the ctest scratch -- nothing was called\n", root.c_str());
            return 2;
        }
    }
    ForceDirectories(AnsiString(root.c_str()));
    if (!IsDir(root)) { std::printf("  ABORT: cannot create %s\n", root.c_str()); return 2; }
    std::printf("  sandbox = %s\n", root.c_str());

    // the machine's own files: stamped now, compared at the end (read only)
    const char* const machine[] = {
        "D:\\HT9045_Log\\OS_TestReport\\Auto1_TestReport.TXT", "D:\\HT9045_Log\\OS_TestReport\\Auto2_TestReport.TXT",
        "D:\\HT9045_Log\\OS_TestReport\\Auto3_TestReport.TXT", "D:\\HT9045_Log\\OS_TestReport\\Fix1_TestReport.TXT",
        "D:\\HT9045_Log\\OSTestResult\\Device000005_01.TXT", "D:\\HT9045_Log\\OSTestResult\\Device000006_01.TXT",
        "D:\\HT9045_Log\\OSTestResult\\Device000007_01.TXT", "D:\\HT9045_Log\\OSTestResult\\Device000008_01.TXT",
        "D:\\HT9045_Log\\OSTestResult\\Device000009_01.TXT", "D:\\HT9045_Log\\OS_Summary\\202610", "D:\\HT9045_Log\\JamRate",
        "D:\\HT9045_Log\\Summary_Lot\\202610" };   // AI(W906-U1) 20261002: + golden's SaveSummaryTrayFeed folder (section 12)
    const int nMachine = (int)(sizeof(machine) / sizeof(machine[0]));
    std::vector<std::string> before;
    for (int i = 0; i < nMachine; ++i) before.push_back(Stamp(machine[i]));

    // ---- save every global this test touches ----------------------------------------------------------------------
    const AnsiString sv9045Log = as9045LogPath, svJamRate = sJamRatePath, svSummary = asSummaryPath;
    const AnsiString svB05Path = IniConfig.sB05_OSReportPath, svN09 = IniConfig.sN09_HandlerFolder, svHandler = IniConfig.SocketHandlerID;
    const bool svB05 = IniConfig.bB05_OSReport, svN10 = IniConfig.bN10_UploadSummaryToFTP;
    const int  svN10Method = IniConfig.iN10UploadMethod;
    const int  svVTEST = IniConfig.bVTESTFunction;
    const bool svSPIL = IniConfig.bSPILFunction, svN17 = IniConfig.bN17UploadLotSummary, svA38 = IniConfig.bA38_SLT_Summary, svN23 = IniConfig.bN23UseLotInfoFile;
    const bool svSort2D = CosFunction.bSortingBy2DList, svSecs93K = CosFunction.bART_SECSGEM_93K, svTSV = CosFunction.bUseTSVFunction;
    const bool svWaitTSV = bWaitTSV;
    const int  svTestType = TestIF_File.iTestType;
    const int  svShtRow = TestSocket.iShtRow, svShtCol = TestSocket.iShtCol, svMaxRow = TestSocket.iMaxRow, svMaxCol = TestSocket.iMaxCol;
    const int  svMap00 = TestIF.iSiteMap[0][0], svMap01 = TestIF.iSiteMap[0][1];
    int svTrayType[eTrayCount];
    std::memcpy(svTrayType, Prod.iTrayType, sizeof(svTrayType));
    int svPick[ePosTrayCount];
    std::memcpy(svPick, iOneTrayPickCount, sizeof(svPick));
    const AnsiString svLotNo = RunInfo.LotNo, svLotStart = RunInfo.LotStartTime, svLotEnd = RunInfo.LotEndTime;
    const bool svLotStarted = RunInfo.bLotStart;
    const AnsiString svCust = fLotInfo->lbledtCustomer->Text, svOp = fLotInfo->edtSysOperatorID->Text, svSetup = fMain->cbSetupFileName->Text;
    const AnsiString svRunMode = fLotInfo->cbRunMode->Text, svProcess = fLotInfo->cbProcess->Text;
    const AnsiString svSite = OutArmSuck.PordRec[0][0].asBuffer->Strings[eSiteNO];
    const AnsiString svOrder = OutArmSuck.PordRec[0][0].asBuffer->Strings[eOrderTest];
    const int svY = SystemYear, svMo = SystemMonth, svD = SystemDate, svH = SystemHour, svMi = SystemMin, svS = SystemSec;
    TArm* const svArmLot0 = ArmDataLot[0];
    TArm* const svArmLot1 = ArmDataLot[1];

    // ---- redirect every write root into the sandbox ----------------------------------------------------------------
    as9045LogPath                = AnsiString(root.c_str());
    sJamRatePath                 = AnsiString((root + "\\JamRate").c_str());
    IniConfig.sB05_OSReportPath  = AnsiString((root + "\\B05_share").c_str());
    asSummaryPath                = AnsiString((root + "\\Summary").c_str());
    IniConfig.sN09_HandlerFolder = AnsiString((root + "\\N09").c_str());
    IniConfig.bB05_OSReport = false;
    IniConfig.bN10_UploadSummaryToFTP = false;
    IniConfig.iN10UploadMethod = 0;
    IniConfig.bVTESTFunction = 0;
    RunInfo.bLotStart = false;

    const std::string rpt = root + "\\OS_TestReport\\";
    const std::string src = root + "\\OSTestResult\\";

    // ---- 1. not TCP_IP_MODE -------------------------------------------------------------------------------------
    std::printf("\n-- 1. PlaceOSTestResultToTray, iTestType != TCP_IP_MODE --\n");
    TestIF_File.iTestType = 0;
    TesterTCP_PlaceOSTestResultToTray(0, 0, 0, 0, 0);
    CHECK(!Exists(root + "\\OS_TestReport"));   // golden :701-702 returns before MyForceDirectories

    // ---- 2. TCP_IP_MODE, the source file is there ---------------------------------------------------------------
    std::printf("\n-- 2. PlaceOSTestResultToTray, TCP_IP_MODE, source present --\n");
    TestIF_File.iTestType = TCP_IP_MODE;
    TestSocket.iShtRow = 1; TestSocket.iShtCol = 2;                 // iTotalCh = 2
    TestSocket.iMaxRow = 1; TestSocket.iMaxCol = 2;
    ForceDirectories(AnsiString((root + "\\OSTestResult").c_str()));
    WriteAll(src + "Device000005_01.TXT", "PIN1,OPEN\r\nPIN2,SHORT\r\n");
    SeedRec(2, 5);                                                  // site 2 -> Device..._01 (golden :712 iTesterCh-1)
    iOneTrayPickCount[1 + eAuto1] = 7;
    TesterTCP_PlaceOSTestResultToTray(0, 0, 2, 3, eAuto1);
    const std::string hdr2 = Fmt(kDevFmt, 7, 3, 4, 2 * 5 + 2);
    std::vector<std::string> auto1;
    auto1.push_back(hdr2); auto1.push_back("PIN1,OPEN"); auto1.push_back("PIN2,SHORT");
    std::string got;
    CHECK(!Exists(src + "Device000005_01.TXT"));                    // golden :718 DeleteFile
    CHECK(ReadAll(rpt + "Auto1_TestReport.TXT", got) && got == Lines(auto1));

    // ---- 3. the source file is missing; two more trays ----------------------------------------------------------
    std::printf("\n-- 3. source missing -> \"can not find file\"; Auto2 / Fix1 reports --\n");
    SeedRec(2, 6);
    iOneTrayPickCount[1 + eAuto1] = 8;
    TesterTCP_PlaceOSTestResultToTray(0, 0, 0, 0, eAuto1);
    auto1.push_back(Fmt(kDevFmt, 8, 1, 1, 2 * 6 + 2));
    auto1.push_back("can not find file " + src + "Device000006_01.TXT");
    CHECK(ReadAll(rpt + "Auto1_TestReport.TXT", got) && got == Lines(auto1));
    WriteAll(src + "Device000007_01.TXT", "PIN9,OPEN\r\n");
    SeedRec(2, 7);
    iOneTrayPickCount[1 + eAuto2] = 1;
    TesterTCP_PlaceOSTestResultToTray(0, 0, 0, 1, eAuto2);
    std::vector<std::string> auto2;
    auto2.push_back(Fmt(kDevFmt, 1, 1, 2, 2 * 7 + 2)); auto2.push_back("PIN9,OPEN");
    CHECK(ReadAll(rpt + "Auto2_TestReport.TXT", got) && got == Lines(auto2));
    WriteAll(src + "Device000008_01.TXT", "PIN7,SHORT\r\n");
    SeedRec(2, 8);
    iOneTrayPickCount[1 + eFix1] = 1;
    TesterTCP_PlaceOSTestResultToTray(0, 0, 1, 0, eFix1);
    std::vector<std::string> fix1;
    fix1.push_back(Fmt(kDevFmt, 1, 2, 1, 2 * 8 + 2)); fix1.push_back("PIN7,SHORT");
    CHECK(ReadAll(rpt + "Fix1_TestReport.TXT", got) && got == Lines(fix1));

    // ---- 4. ProcessOSPrint(false) -----------------------------------------------------------------------------------
    std::printf("\n-- 4. ProcessOSPrint(false): RTF summary, Pin reports, TestReports emptied --\n");
    ForceDirectories(AnsiString((root + "\\B05_share").c_str()));    // AI(W906-R126) 20261002: the (sandboxed) share exists from here on,
                                                                     //   so the B05-off check below can fail (vclcompat SaveToFile never creates folders)
    for (int i = 0; i < eTrayCount; ++i) Prod.iTrayType[i] = tNotUse;
    Prod.iTrayType[eAuto1] = tTrayAuto; Prod.iTrayType[eAuto2] = tTrayAuto; Prod.iTrayType[eAuto3] = tTrayAuto;
    Prod.iTrayType[eFix1] = tTrayFix;
    ArmDataLot[0] = new TArm("W906G023ARM0-DO-NOT-USE");            // allocation only (cSocket.cpp TArm ctor)
    ArmDataLot[1] = new TArm("W906G023ARM1-DO-NOT-USE");
    TestIF.iSiteMap[0][0] = 1; TestIF.iSiteMap[0][1] = 2;           // DUT1 at (0,0), DUT2 at (0,1)
    ArmDataLot[0]->ArmSKET[0][0]->Pass = 48; ArmDataLot[0]->ArmSKET[0][0]->Fail = 12; ArmDataLot[0]->ArmSKET[0][0]->Total = 60;
    ArmDataLot[1]->ArmSKET[0][0]->Pass = 32; ArmDataLot[1]->ArmSKET[0][0]->Fail =  8; ArmDataLot[1]->ArmSKET[0][0]->Total = 40;
    ArmDataLot[0]->ArmSKET[0][1]->Pass =  9; ArmDataLot[0]->ArmSKET[0][1]->Fail =  1; ArmDataLot[0]->ArmSKET[0][1]->Total = 10;
    // DUT1 100 / 80 / 20, DUT2 10 / 9 / 1, all 110 / 89 / 21 (GetTotal() = Pass + Fail)
    RunInfo.LotNo        = kLot;
    RunInfo.LotStartTime = "2026-10-01 08:00:00";
    RunInfo.LotEndTime   = "2026-10-01 09:00:00";
    const std::string cust = "A{B}C\\D\xA4\xA4";                    // RTF specials + one Big5 character
    fLotInfo->lbledtCustomer->Text   = AnsiString(cust.c_str());
    fLotInfo->edtSysOperatorID->Text = "OP23";
    fMain->cbSetupFileName->Text     = "W906G023SETUP";
    IniConfig.SocketHandlerID        = "W906G023HANDLER";
    SetTime(12, 34, 56);
    TesterTCP_ProcessOSPrint(false);

    CHECK(TastCategory.iTotalSocket == 110 && TastCategory.iPassSocket == 89 && TastCategory.iFailSocket == 21);
    CHECK(TastCategory.iBySiteTotal[0] == 100 && TastCategory.iBySiteTotal[1] == 10);
    CHECK(fTesterTCP->SummaryHead->Count == 14);

    std::vector<std::string> head;
    head.push_back("============================ SUMMARY REPORT ============================");
    head.push_back(Fmt("%-34s %s", "Lot#:", kLot));
    head.push_back(Fmt("%-34s %s", "Start:", "2026-10-01 08:00:00"));
    head.push_back(Fmt("%-34s %s", "End:", "2026-10-01 09:00:00"));
    head.push_back(Fmt("%-34s %s", "Customer:", cust.c_str()));
    head.push_back(Fmt("%-34s %s", "Program:", "W906G023SETUP"));
    head.push_back(Fmt("%-34s %s", "Operator ID:", "OP23"));
    head.push_back(Fmt("%-34s %s", "Machine ID:", "W906G023HANDLER"));
    head.push_back(Fmt("%-34s %d", "Input:", 110));
    head.push_back(Fmt("%-34s %d", "Pass:", 89));
    head.push_back(Fmt("%-34s %d", "Fail:", 21));
    head.push_back(Fmt("%-34s %d", "Open:", TastCategory.iUnloadCnt[1]));
    head.push_back(Fmt("%-34s %d", "Short:", TastCategory.iUnloadCnt[2]));
    head.push_back(Fmt("%-34s %s", "Yield:", Pct(89, 110).c_str()));

    std::vector<std::string> sum = head;
    sum.push_back("============================= BY TRAY COUNT ============================");
    sum.push_back(" ");
    const int used[] = { eAuto1, eAuto2, eAuto3, eFix1 };
    for (int k = 0; k < 4; ++k) sum.push_back(Fmt("%-34s %d", (std::string(s6TrayName[used[k]].c_str()) + ":").c_str(), TastCategory.iUnloadCnt[used[k]]));
    sum.push_back(" ");
    sum.push_back("============================= BY SITE COUNT ============================");
    sum.push_back(" ");
    sum.push_back(Fmt("%-34s", "Total Tested DUT Count:") + Fmt(" %-20s", "DUT1") + Fmt(" %-20s", "DUT2") + Fmt(" %-20s", "SUM"));
    const std::string rowTotal = Fmt("%-34s", "Total") + Fmt(" %-20d", 80 + 20) + Fmt(" %-20s", "10") + Fmt(" %-20d", 110);   // DUT1 total = PASS 80 + FAIL 20 (computed, not a literal)
    const std::string rowPass  = Fmt("%-34s", "PASS") + Fmt(" %-20s", ("80(" + Pct(80, 100) + ")").c_str()) + Fmt(" %-20s", ("9(" + Pct(9, 10) + ")").c_str()) + Fmt(" %-20s", ("89(" + Pct(89, 110) + ")").c_str());
    const std::string rowFail  = Fmt("%-34s", "FAIL") + Fmt(" %-20s", ("20(" + Pct(20, 100) + ")").c_str()) + Fmt(" %-20s", ("1(" + Pct(1, 10) + ")").c_str()) + Fmt(" %-20s", ("21(" + Pct(21, 110) + ")").c_str());
    sum.push_back(rowTotal); sum.push_back(rowPass); sum.push_back(rowFail);
    sum.push_back(" ");
    sum.push_back("============================= BY BIN COUNT =============================");
    sum.push_back(" ");
    sum.push_back(Fmt("%-34s", "HW BIN Count:") + Fmt(" %-20s", "DUT1") + Fmt(" %-20s", "DUT2") + Fmt(" %-20s", "SUM"));
    for (int iCat = 0; iCat <= iTestBinCount; ++iCat) {
        std::string r = Fmt("%-34s", iCat < iTestBinCount ? Fmt("BIN %d", iCat).c_str() : "REJECT");
        for (int iDut = 0; iDut < 2; ++iDut) {
            const int n = TastCategory.iBySiteCate[iDut][iCat];
            r += Fmt(" %-20s", (Fmt("%d(", n) + Pct(n, TastCategory.iTotalCategory[iCat]) + ")").c_str());
        }
        r += Fmt(" %-20s", (Fmt("%d(", TastCategory.iTotalCategory[iCat]) + Pct(TastCategory.iTotalCategory[iCat], TastCategory.iTotalSocket) + ")").c_str());
        sum.push_back(r);
    }

    const std::string ts  = "2026_10_01_12_34_56";
    const std::string dir = root + "\\OS_Summary\\202610\\";
    std::string fsum;
    CHECK(ReadAll(dir + "OS_Summary_" + kLot + "_" + ts + ".TXT", fsum));
    CHECK(fsum.compare(0, std::strlen(kRtfHead), kRtfHead) == 0);                                                // RTF, not plain text
    CHECK(fsum.find(std::string(kRtfFirst) + "============================ SUMMARY REPORT ============================\r\n") == std::strlen(kRtfHead));
    CHECK(fsum.find("\r\n\\par " + rowTotal + "\r\n") != std::string::npos);                                    // P4: numbers, not object bytes
    CHECK(fsum.find("\r\n\\par " + rowPass + "\r\n") != std::string::npos);
    CHECK(fsum.find("\r\n\\par " + rowFail + "\r\n") != std::string::npos);
    CHECK(fsum.find("A\\{B\\}C\\\\D\\'a4\\'a4") != std::string::npos);                                         // P3 escaping
    CHECK(fsum.size() > 16 && fsum.compare(fsum.size() - 16, 16, std::string("\\par \r\n\\par }\r\n") + '\0') == 0);
    CHECK(fsum == Rtf(sum));                                                                                     // the whole file

    std::vector<std::string> all = head;                            // OS_Pin_All = SummaryHead + every used tray that had a report
    const std::vector<std::string>* rep[] = { &auto1, &auto2, 0, &fix1 };
    for (int k = 0; k < 4; ++k) {
        if (!rep[k]) continue;
        all.push_back(Fmt(kTrayFmt, s6TrayName[used[k]].c_str())); all.push_back(" ");
        for (size_t i = 0; i < rep[k]->size(); ++i) all.push_back((*rep[k])[i]);
        all.push_back(" ");
    }
    CHECK(ReadAll(dir + "OS_Pin_All_" + kLot + "_" + ts + ".TXT", got) && got == Lines(all));
    std::vector<std::string> pin1 = head; pin1.push_back(" "); pin1.push_back(kFailPin); pin1.insert(pin1.end(), auto1.begin(), auto1.end());
    std::vector<std::string> pin2 = head; pin2.push_back(" "); pin2.push_back(kFailPin); pin2.insert(pin2.end(), auto2.begin(), auto2.end());
    std::vector<std::string> pin3 = head; pin3.push_back(" "); pin3.push_back(kFailPin);   // 無料Auto仍產生報表 (golden :1024)
    std::string pin2bytes;
    CHECK(ReadAll(dir + "OS_Pin_Auto1_" + kLot + "_" + ts + ".TXT", got) && got == Lines(pin1));
    CHECK(ReadAll(dir + "OS_Pin_Auto2_" + kLot + "_" + ts + ".TXT", pin2bytes) && pin2bytes == Lines(pin2));
    CHECK(ReadAll(dir + "OS_Pin_Auto3_" + kLot + "_" + ts + ".TXT", got) && got == Lines(pin3));
    CHECK(pin2bytes.find(std::string("\r\n") + kFailPin + "\r\n") != std::string::npos);                         // P5: Big5 bytes
    CHECK(!Exists(dir + "OS_Pin_Fix1_" + kLot + "_" + ts + ".TXT"));                                             // golden :1038 Fix不存本地
    CHECK(!Exists(dir + "OS_Pin_Auto4_" + kLot + "_" + ts + ".TXT"));                                            // tNotUse
    CHECK(SizeOf(rpt + "Auto1_TestReport.TXT") == 0 && SizeOf(rpt + "Auto2_TestReport.TXT") == 0 && SizeOf(rpt + "Fix1_TestReport.TXT") == 0);
    CHECK(!Exists(rpt + "Auto3_TestReport.TXT"));                                                                // no report -> not created
    CHECK(CountFiles(root + "\\B05_share\\*") == 0);                                                             // B05 off
    CHECK(!Exists(root + "\\JamRate"));                                                                          // bLotStart false

    // ---- 5. B05 on --------------------------------------------------------------------------------------------------
    std::printf("\n-- 5. B05 on: only Auto2 / Auto3 go to the (sandboxed) share --\n");
    ForceDirectories(AnsiString((root + "\\B05_share").c_str()));    // the share exists (golden does not create it; already made in section 4)
    IniConfig.bB05_OSReport = true;
    SetTime(12, 34, 57);
    TesterTCP_ProcessOSPrint(false);
    const std::string ts5 = "2026_10_01_12_34_57";
    std::string share2, local2;
    CHECK(CountFiles(root + "\\B05_share\\*") == 2);
    CHECK(ReadAll(root + "\\B05_share\\OS_Pin_Auto2_" + kLot + "_" + ts5 + ".TXT", share2)
          && ReadAll(dir + "OS_Pin_Auto2_" + kLot + "_" + ts5 + ".TXT", local2) && share2 == local2);
    CHECK(Exists(root + "\\B05_share\\OS_Pin_Auto3_" + kLot + "_" + ts5 + ".TXT"));
    CHECK(!Exists(root + "\\B05_share\\OS_Pin_Auto1_" + kLot + "_" + ts5 + ".TXT"));
    CHECK(!Exists(root + "\\B05_share\\OS_Summary_" + kLot + "_" + ts5 + ".TXT"));                               // golden :955-956 commented out
    IniConfig.bB05_OSReport = false;

    // ---- 6. bViewOnly ------------------------------------------------------------------------------------------------
    std::printf("\n-- 6. ProcessOSPrint(true): no reports, TestReports still emptied --\n");
    WriteAll(src + "Device000009_01.TXT", "PIN3,OPEN\r\n");
    SeedRec(2, 9);
    TesterTCP_PlaceOSTestResultToTray(0, 0, 0, 0, eAuto1);
    CHECK(SizeOf(rpt + "Auto1_TestReport.TXT") > 0);
    SetTime(12, 34, 58);
    TesterTCP_ProcessOSPrint(true);
    const std::string ts6 = "2026_10_01_12_34_58";
    CHECK(!Exists(dir + "OS_Summary_" + kLot + "_" + ts6 + ".TXT"));
    CHECK(!Exists(dir + "OS_Pin_All_" + kLot + "_" + ts6 + ".TXT"));
    CHECK(!Exists(dir + "OS_Pin_Auto1_" + kLot + "_" + ts6 + ".TXT"));
    CHECK(SizeOf(rpt + "Auto1_TestReport.TXT") == 0);                                                            // golden :1042-1046

    // ---- 7. SIM: a UNC share is skipped ---------------------------------------------------------------------------
    std::printf("\n-- 7. W58 Q5: a network-share B05 path in SIM --\n");
#ifdef SOFT_SIMULTE
    {
        const char* e = std::getenv("W906_SIM_NET_PATHS");
        if (e && std::strcmp(e, "1") == 0) {
            std::printf("  SKIP: W906_SIM_NET_PATHS=1 would write the share\n");
        } else {
            const AnsiString unc = "\\\\W906-G023-NO-SUCH-HOST\\share";
            CHECK(W906_SimNetPathBlocked(unc) == true);                                                          // a string test, no lookup
            IniConfig.sB05_OSReportPath = unc;
            IniConfig.bB05_OSReport = true;
            SetTime(12, 34, 59);
            TesterTCP_ProcessOSPrint(false);
            CHECK(Exists(dir + "OS_Pin_Auto2_" + kLot + "_2026_10_01_12_34_59.TXT"));                            // the local copy still written
            IniConfig.bB05_OSReport = false;
            IniConfig.sB05_OSReportPath = AnsiString((root + "\\B05_share").c_str());
        }
    }
#else
    std::printf("  SKIP: SHIP build -- golden writes every share path, so the test does not point it at one\n");
#endif

    // ---- 8. JamRate ----------------------------------------------------------------------------------------------
    std::printf("\n-- 8. RunInfo.SaveJamRateByLot (golden :961) into the redirected sJamRatePath --\n");
    RunInfo.bLotStart = true;
    fLotInfo->cbRunMode->Text = "RM23";
    fLotInfo->cbProcess->Text = "FT23";
    SetTime(12, 35, 0);
    TesterTCP_ProcessOSPrint(false);
    const std::string jam = root + "\\JamRate\\" + Fmt("%s_%s_%04d%02d%02d-%02d%02d%02d_%s_%s_%s_JamRateByLot.txt",
        IniConfig.sMachineType.c_str(), "W906G023HANDLER", 2026, 10, 1, 12, 35, 0, kLot, "RM23", "FT23");
    CHECK(ReadAll(jam, got) && got.compare(0, std::strlen("Lot No: ") + std::strlen(kLot), std::string("Lot No: ") + kLot) == 0);
    RunInfo.bLotStart = false;

    // ---- 9. the dispatcher (gate #6) --------------------------------------------------------------------------------
    std::printf("\n-- 9. SaveTestSummary -> ProcessOSPrint (SCK_ART_Remainder.cpp gate #6) --\n");
    IniConfig.bSPILFunction = false; IniConfig.bN17UploadLotSummary = false; IniConfig.bA38_SLT_Summary = false; IniConfig.bN23UseLotInfoFile = false;
    CosFunction.bSortingBy2DList = false;
    CosFunction.bART_SECSGEM_93K = false;
    CosFunction.bUseTSVFunction  = true;      // test_SCK_ART_Remainder PART 8: keeps SaveTestSummaryTSV on the redirected sN09_HandlerFolder
    TestIF_File.iTestType = TCP_IP_MODE;
    SetTime(12, 36, 0);
    const std::string ts9 = dir + "OS_Summary_" + kLot + "_2026_10_01_12_36_00.TXT";
    {
        SckArtRemainderState st;
        SckArtRem_SaveTestSummary(st, 0);
        CHECK(!Exists(ts9));                                                                                     // golden :1633 if(iSaveData)
    }
    TStringList skipped;
    W906_SckArt_SaveTestSummary(1, &skipped);
    CHECK(Exists(ts9));                                                                                          // golden :1634 through gate #6
    CHECK(CountSkipped(skipped, "ProcessOSPrint") == 0);                                                         // SCK_ART_SaveTestSummary.cpp:56-57
    CHECK(CountSkipped(skipped, "ATK TSV") == 1 && skipped.Count == 1);                                          // :58-59 still ran
    CHECK(IsDir(root + "\\Summary"));                                                                            // the TSV summary stayed in the sandbox
    bWaitTSV = svWaitTSV;

    // ---- 10. ratchet: the statement after the two commented lines still runs (St01 28ad5b17) -----------------------
    std::printf("\n-- 10. ratchet: SCK_ART_SaveTestSummary.cpp :58-59 still run for a non-TCP tester --\n");
    {
        const int types[] = { TTL_MODE, GPIB_MODE, RS232_MODE };
        const char* const names[] = { "TTL", "GPIB", "RS232" };
        for (int k = 0; k < 3; ++k) {
            TestIF_File.iTestType = types[k];
            TStringList sk;
            W906_SckArt_SaveTestSummary(0, &sk);          // iSaveData 0: no SaveSummaryTrayFeed; TSV stops after its (sandboxed) folders
            const std::string what = Fmt("%s tester + bUseTSVFunction: exactly one skipped entry, the ATK TSV one (:58-59)", names[k]);
            check(sk.Count == 1 && CountSkipped(sk, "ATK TSV") == 1 && CountSkipped(sk, "ProcessOSPrint") == 0, what.c_str(), __LINE__);
        }
        bWaitTSV = svWaitTSV;
        const std::vector<std::string> code = CodeLines("Automation/SCK_ART_SaveTestSummary.cpp");
        size_t at = code.size();
        for (size_t i = 0; i < code.size(); ++i) if (code[i] == "LotSummary.ClearAllData();") at = i;
        CHECK(at + 3 < code.size());
        CHECK(at + 3 < code.size() && code[at + 1] == "if (CosFunction.bUseTSVFunction)");                      // not the old TCP_IP_MODE if
        CHECK(at + 3 < code.size() && code[at + 2].compare(0, 25, "W906_SckArtSkip(skipped, ") == 0 && code[at + 2].find("ATK TSV") != std::string::npos);
        CHECK(at + 3 < code.size() && code[at + 3] == "}");                                                     // end of W906_SckArt_SaveTestSummary
        bool tcpIf = false;
        for (size_t i = 0; i < code.size(); ++i) if (code[i].find("TCP_IP_MODE") != std::string::npos && code[i].compare(0, 3, "if ") == 0) tcpIf = true;
        CHECK(!tcpIf);                                                                                           // the :56 if is gone as code
    }
    CosFunction.bUseTSVFunction = svTSV;

    // ---- 11. the callers are live ----------------------------------------------------------------------------------
    std::printf("\n-- 11. source pins: both callers live --\n");
    CHECK(LiveLineContaining("aoutarm9045.cpp", "TesterTCP_PlaceOSTestResultToTray(i, j, iTrayRow, iTrayCol, OutArmSuck.iWhichAuto[i][j]);"));
    CHECK(!LiveLineContaining("aoutarm9045.cpp", "fTesterTCP->PlaceOSTestResultToTray("));
    CHECK(LiveLineContaining("Automation/SCK_ART_Remainder.cpp", "#define W5SCKARTREM_FTESTERTCP_PROCESSOSPRINT()   TesterTCP_ProcessOSPrint()"));
    CHECK(LiveLineContaining("Automation/SCK_ART_Remainder.cpp", "W5SCKARTREM_FTESTERTCP_PROCESSOSPRINT();"));

    // ---- 12. U1: SaveSummaryTrayFeed's BY SITE rows (G-023 group C) ---------------------------------------------------
    // AI(W906-U1) 20261002 (St02-E): St02-M card (our G-023 pass, U1 + U4).  SaveSummaryTrayFeed runs at every Lot End on a
    //   non-TCP tester (SCK_ART_Remainder.cpp:936); its three BY SITE rows had no test.  Its first act is the REAL
    //   TastCategory.UpdataCount(false) (SCK_ART_Remainder.cpp:2545; cSocket.cpp:1258: ClearCount(), then per site from
    //   ArmDataLot[arm]->ArmSKET[row][col] Pass / Fail, only where TestIF.iSiteMap > 0), so the counts are seeded there
    //   (the first push of this section seeded TastCategory directly and went red: UpdataCount overwrote them).
    //   Mutation (by reasoning): without the AnsiString(...) wrap at :2657-2659 the StringsProxy goes through '...' and its
    //   own bytes are printed, so none of the exact rows below is in the file.
    std::printf("\n-- 12. U1: SaveSummaryTrayFeed BY SITE rows (numbers in golden's \" %%-20s\" columns), U4 defaults --\n");
    {
        const char* e0 = std::getenv("W906_SUMMARYLOT_ROOT");
        const bool hadRoot = (e0 != 0);
        const std::string oldRoot = e0 ? e0 : "";
        const std::string sumRoot = root + "\\SummaryLot";
        static std::string kvSet, kvOld;                                 // the CRT may keep the pointer: the text stays alive
        kvSet = "W906_SUMMARYLOT_ROOT=" + sumRoot;
        HT9045_TEST_PUTENV(kvSet.c_str());             // else golden's literal D:\HT9045_Log\Summary_Lot (SCK_ART_Remainder.cpp:2730-2732)
        IniConfig.bN10_UploadSummaryToFTP = false;
        RunInfo.bLotStart = false;
        const int jam0 = CountFiles(root + "\\JamRate\\*");          // section 8 left one there
        TestIF.iSiteMap[0][0] = 1; TestIF.iSiteMap[0][1] = 2;
        // DUT1 = (0,0): arm 0 30 / 5 + arm 1 15 / 0 -> 50 / 45 / 5; DUT2 = (0,1): arm 0 2 / 6 + arm 1 0 / 0 -> 8 / 2 / 6
        // (GetTotal() = Pass + Fail, cSocket.cpp:383-386); all 58 / 47 / 11
        ArmDataLot[0]->ArmSKET[0][0]->Pass = 30; ArmDataLot[0]->ArmSKET[0][0]->Fail = 5; ArmDataLot[0]->ArmSKET[0][0]->Total = 35;
        ArmDataLot[1]->ArmSKET[0][0]->Pass = 15; ArmDataLot[1]->ArmSKET[0][0]->Fail = 0; ArmDataLot[1]->ArmSKET[0][0]->Total = 15;
        ArmDataLot[0]->ArmSKET[0][1]->Pass =  2; ArmDataLot[0]->ArmSKET[0][1]->Fail = 6; ArmDataLot[0]->ArmSKET[0][1]->Total =  8;
        ArmDataLot[1]->ArmSKET[0][1]->Pass =  0; ArmDataLot[1]->ArmSKET[0][1]->Fail = 0; ArmDataLot[1]->ArmSKET[0][1]->Total =  0;
        std::string sum;
        // the BY SITE rows as they are in the file, every line in <>, on a mismatch only
        auto showRows = [](const std::string& text) {
            const size_t at = text.find("BY SITE COUNT");
            std::printf("  BY SITE section as written (%s):\n", at == std::string::npos ? "header NOT found" : "from the header");
            size_t p = (at == std::string::npos) ? 0 : at;
            for (int k = 0; k < 7 && p < text.size(); ++k) {
                const size_t e = text.find("\r\n", p);
                const std::string ln = text.substr(p, (e == std::string::npos ? text.size() : e) - p);
                std::printf("    <%s> (%u chars)\n", ln.c_str(), (unsigned)ln.size());
                if (e == std::string::npos) break;
                p = e + 2;
            }
        };
        auto row = [&](const std::string& text, const std::string& want, const char* what, int line) {
            const bool ok = text.find("\r\n" + want + "\r\n") != std::string::npos;
            check(ok, what, line);
            if (!ok) { std::printf("    expected <%s> (%u chars)\n", want.c_str(), (unsigned)want.size()); showRows(text); }
        };
        auto feed = [&](int s, std::string& out) {
            SetTime(12, 40, s);
            { SckArtRemainderState st; SckArtRem_SaveSummaryTrayFeed(st); }
            return ReadAll(sumRoot + "\\202610\\" + Fmt("W906G023HANDLER 20261001-1240%02d ", s) + kLot + " Summary.txt", out);
        };
        // golden SCK_ART.cpp:3247-3266: "%-34s" head, one " %-20s" per DUT (:3253-3255), then the SUM column (:3264-3266)
        CHECK(feed(0, sum));
        CHECK(TastCategory.iBySiteTotal[0] == 50 && TastCategory.iBySiteTotal[1] == 8 && TastCategory.iTotalSocket == 58);   // what UpdataCount made of the seeds
        row(sum, Fmt("%-34s", "Total") + Fmt(" %-20s", "50") + Fmt(" %-20s", "8") + Fmt(" %-20d", 58), "12. Total row: 50 / 8 / sum 58", __LINE__);
        row(sum, Fmt("%-34s", "PASS") + Fmt(" %-20s", "45(90.00%)") + Fmt(" %-20s", "2(25.00%)") + Fmt(" %-20s", "47(81.03%)"),
            "12. PASS row: 45(90.00%) / 2(25.00%) / sum 47(81.03%)", __LINE__);
        row(sum, Fmt("%-34s", "FAIL") + Fmt(" %-20s", "5(10.00%)") + Fmt(" %-20s", "6(75.00%)") + Fmt(" %-20s", "11(18.97%)"),
            "12. FAIL row: 5(10.00%) / 6(75.00%) / sum 11(18.97%)", __LINE__);
        // U4 (a): DUT2 disabled in the site map (golden :3228 iSiteMap > 0 false) -> its column keeps the defaults (:3218-3220);
        //   UpdataCount skips the same site (cSocket.cpp:1258 the iSiteMap test), so the sum is DUT1's alone
        TestIF.iSiteMap[0][1] = 0;
        CHECK(feed(1, sum));
        row(sum, Fmt("%-34s", "Total") + Fmt(" %-20s", "50") + Fmt(" %-20s", "0") + Fmt(" %-20d", 50), "12. U4 (a) Total: DUT2 disabled -> \"0\"", __LINE__);
        row(sum, Fmt("%-34s", "PASS") + Fmt(" %-20s", "45(90.00%)") + Fmt(" %-20s", "0(0.00%)") + Fmt(" %-20s", "45(90.00%)"),
            "12. U4 (a) PASS: DUT2 disabled -> \"0(0.00%)\"", __LINE__);
        row(sum, Fmt("%-34s", "FAIL") + Fmt(" %-20s", "5(10.00%)") + Fmt(" %-20s", "0(0.00%)") + Fmt(" %-20s", "5(10.00%)"),
            "12. U4 (a) FAIL: DUT2 disabled -> \"0(0.00%)\"", __LINE__);
        // U4 (b): DUT2 enabled but nothing counted (golden :3235 iSiteTotalCt > 0 false) -> the same defaults
        TestIF.iSiteMap[0][1] = 2;
        ArmDataLot[0]->ArmSKET[0][1]->Pass = 0; ArmDataLot[0]->ArmSKET[0][1]->Fail = 0; ArmDataLot[0]->ArmSKET[0][1]->Total = 0;
        CHECK(feed(2, sum));
        row(sum, Fmt("%-34s", "Total") + Fmt(" %-20s", "50") + Fmt(" %-20s", "0") + Fmt(" %-20d", 50), "12. U4 (b) Total: DUT2 no count -> \"0\"", __LINE__);
        row(sum, Fmt("%-34s", "FAIL") + Fmt(" %-20s", "5(10.00%)") + Fmt(" %-20s", "0(0.00%)") + Fmt(" %-20s", "5(10.00%)"),
            "12. U4 (b) FAIL: DUT2 no count -> \"0(0.00%)\"", __LINE__);
        CHECK(CountFiles(root + "\\JamRate\\*") == jam0);                                                          // bLotStart false: SaveJamRateByLot wrote nothing
        kvOld = "W906_SUMMARYLOT_ROOT=" + (hadRoot ? oldRoot : std::string());   // Windows: "K=" removes K
        HT9045_TEST_PUTENV(kvOld.c_str());
    }

    // ---- restore ---------------------------------------------------------------------------------------------------
    // NOT deleted: ~TArm calls WriteFile() (cSocket.cpp:709-713), a file write this test must not make -- the same
    // two-object leak test_SCK_ART_Remainder PART 13 accepts.
    ArmDataLot[0] = svArmLot0; ArmDataLot[1] = svArmLot1;
    as9045LogPath = sv9045Log; sJamRatePath = svJamRate; asSummaryPath = svSummary;
    IniConfig.sB05_OSReportPath = svB05Path; IniConfig.sN09_HandlerFolder = svN09; IniConfig.SocketHandlerID = svHandler;
    IniConfig.bB05_OSReport = svB05; IniConfig.bN10_UploadSummaryToFTP = svN10; IniConfig.iN10UploadMethod = svN10Method;
    IniConfig.bVTESTFunction = svVTEST;
    IniConfig.bSPILFunction = svSPIL; IniConfig.bN17UploadLotSummary = svN17; IniConfig.bA38_SLT_Summary = svA38; IniConfig.bN23UseLotInfoFile = svN23;
    CosFunction.bSortingBy2DList = svSort2D; CosFunction.bART_SECSGEM_93K = svSecs93K; CosFunction.bUseTSVFunction = svTSV;
    TestIF_File.iTestType = svTestType;
    TestSocket.iShtRow = svShtRow; TestSocket.iShtCol = svShtCol; TestSocket.iMaxRow = svMaxRow; TestSocket.iMaxCol = svMaxCol;
    TestIF.iSiteMap[0][0] = svMap00; TestIF.iSiteMap[0][1] = svMap01;
    std::memcpy(Prod.iTrayType, svTrayType, sizeof(svTrayType));
    std::memcpy(iOneTrayPickCount, svPick, sizeof(svPick));
    RunInfo.LotNo = svLotNo; RunInfo.LotStartTime = svLotStart; RunInfo.LotEndTime = svLotEnd; RunInfo.bLotStart = svLotStarted;
    fLotInfo->lbledtCustomer->Text = svCust; fLotInfo->edtSysOperatorID->Text = svOp; fMain->cbSetupFileName->Text = svSetup;
    fLotInfo->cbRunMode->Text = svRunMode; fLotInfo->cbProcess->Text = svProcess;
    OutArmSuck.PordRec[0][0].asBuffer->Strings[eSiteNO] = svSite;
    OutArmSuck.PordRec[0][0].asBuffer->Strings[eOrderTest] = svOrder;
    SystemYear = svY; SystemMonth = svMo; SystemDate = svD; SystemHour = svH; SystemMin = svMi; SystemSec = svS;

    // ---- containment proof: the machine's own files did not change -------------------------------------------------
    std::printf("\n-- containment: D:\\HT9045_Log untouched --\n");
    bool same = true;
    for (int i = 0; i < nMachine; ++i)
        if (Stamp(machine[i]) != before[i]) { same = false; std::printf("  changed: %s (%s -> %s)\n", machine[i], before[i].c_str(), Stamp(machine[i]).c_str()); }
    CHECK(same);
    CHECK(CountFiles(std::string("D:\\HT9045_Log\\OS_Summary\\202610\\*") + kLot + "*") == 0);
    CHECK(CountFiles(std::string("D:\\HT9045_Log\\JamRate\\*") + kLot + "*") == 0);

    std::printf("\n%d checks, %d failed\n", g_total, g_fail);
    if (g_fail == 0) {
        const bool gone = RemoveTree(root);
        std::printf("  sandbox %s: %s\n", root.c_str(), gone ? "removed" : "kept (could not remove)");
    } else {
        std::printf("  sandbox kept as evidence: %s\n", root.c_str());
    }
    return g_fail == 0 ? 0 : 1;
}
