// =============================================================================================
//  tests/test_nb2_w170_resetcleancount.cpp -- AI(W906-W170) 20261008 (NB2-1, README R267)
//
//  W-170 (CHAT_JIMMY 1008 08:3x; R265 = A, RULINGS_20261001 #0): golden TfCleaning::btnResetCleanCountClick (906
//  AutoClean/uCleaning.cpp:2095-2117, V912 :2116-2138) for its non-page callers.  The facade method (forms/fCleaning.cpp, ht9045_forms)
//  forwards through W906_ResetCleanCountHook, which FileRW_Cleaning_Boot sets to FileRW_Cleaning_ResetCleanCount (the generated golden
//  body CL_btnResetCleanCountClick under FormLock).
//    [1] no hook (before boot) = the old no-op
//    [2] after boot the facade call runs golden: iIndexArmAutoCleanCnt 0, bErrorAutoClean false, iAutoCleanAlarm 1 -> 0 (:2128-2130),
//        edCleaningCount "0", one EventReport(SECS_EVENT.AutoCleanClearCount), FormLock balanced
//    [3] csystem.cpp CheckSafeDoorForICFallDown: clean pad changed + the safe door open -> bChangeCleanPad cleared and the count reset
//        (golden csystem.cpp:3577 Fix3 / :3586 other route; SHIP = G01a / G01b, SIM = the SOFT_SIMULTE branch)
//    [4] source ratchet: Command.cpp HTSET 469 and cShowBinSelect.cpp ed_AutoCleanCountClick (B5) call it live (not in #if 0, not a comment)
//  Writes only the W906_INIDATA_ROOT sandbox (ReadWriteAutoCleanCount -> DataPath HandlerCondition.Data; E43-1 folder and
//  bUseDefineAutoCleanOffset off, as tests/test_b8_cl4_autoclean.cpp) and refuses outside ctest; compares Gerneral.ini / config.ini and
//  the D:\HT9045\IniData / D:\HT9045\data file lists.
//  REVERSE (done when this landed): drop the hook install in FileRW_Cleaning_Boot -> [2] [3] red; G01a / G01b back to #if 0 -> [3] red in
//  SHIP; Command.cpp HTSET 469 back to #if 0 -> [4] red.
// =============================================================================================
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "csystem.h"                     // CheckSafeDoorForICFallDown
#include "mysensor.h"
#include "forms/fCleaning.h"             // fCleaning (TfCleaning facade)
#include "SECSGEM/SecsEventReport.h"     // g_SimEventReportCount / g_SimLastEventReportCeid / ResetSimEventReport
#include "SECSGEM/SecsEventType.h"       // SECS_EVENT
#include "w906_ctest_guard.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "Motor/mymotor.h"
#include "w906_test_motors.h"
#pragma GCC diagnostic pop

#include <windows.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>
#include <string>

extern void FileRW_Cleaning_Boot();
extern void (*W906_ResetCleanCountHook)();   // forms/fCleaning.cpp (W-170)
extern AnsiString DataPath;
extern AnsiString DefaultPath;

// Link-only (not under test) -- same as tests/test_b8_cl4_autoclean.cpp.
void FileRW_IniConfig_ChangeCBListProperty() {}
void FileRW_LdUld_Boot() { std::printf("link-only stand-in reached: FileRW_LdUld_Boot\n"); std::abort(); }
void FileRW_LdUld_ReadFile() { std::printf("link-only stand-in reached: FileRW_LdUld_ReadFile\n"); std::abort(); }
namespace {
int g_lockDepth = 0;
int g_lockMax = 0;
}
namespace ht9045 {
namespace formbridge {
const BridgeDesc* FindBridge(const std::string&) { return nullptr; }
bool RunEvent(const BridgeDesc&, const formevent::Request&, formevent::Result* out)
{
    out->code = "handler-failed"; out->why = "test: no A-shape page";
    return false;
}
}  // namespace formbridge
namespace formjson {
void FormLock() { ++g_lockDepth; if (g_lockDepth > g_lockMax) g_lockMax = g_lockDepth; }
void FormUnlock() { --g_lockDepth; }
}  // namespace formjson
}  // namespace ht9045

namespace {

int g_pass = 0;
int g_fail = 0;

void Check(bool ok, const std::string& what)
{
    if (ok) { ++g_pass; std::printf("  PASS: %s\n", what.c_str()); return; }
    ++g_fail;
    std::printf("  FAIL: %s\n", what.c_str());
}

bool ReadAll(const std::string& p, std::string* out)
{
    std::ifstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    *out = ss.str();
    return true;
}

void ListTree(const std::string& root, const std::string& rel, std::map<std::string, std::string>* out)
{
    WIN32_FIND_DATAA fd;
    const std::string pat = root + (rel.empty() ? "" : "\\" + rel) + "\\*";
    HANDLE h = ::FindFirstFileA(pat.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        const std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        const std::string r = rel.empty() ? name : rel + "\\" + name;
        char buf[96];
        std::snprintf(buf, sizeof(buf), "%lu/%lu/%lu/%lu", (unsigned long)fd.nFileSizeHigh, (unsigned long)fd.nFileSizeLow,
                      (unsigned long)fd.ftLastWriteTime.dwHighDateTime, (unsigned long)fd.ftLastWriteTime.dwLowDateTime);
        (*out)[r] = buf;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ListTree(root, r, out);
    } while (::FindNextFileA(h, &fd));
    ::FindClose(h);
}

void Arm()   // the counters golden resets, set to non-zero
{
    TestIF_File.iIndexArmAutoCleanCnt = 7;
    bErrorAutoClean = true;
    iAutoCleanAlarm = 1;
    fCleaning->edCleaningCount->Text = "5";
    ResetSimEventReport();
}
bool Reset()  // what golden uCleaning.cpp:2095-2117 leaves
{
    return TestIF_File.iIndexArmAutoCleanCnt == 0 && !bErrorAutoClean && iAutoCleanAlarm == 0 &&
           g_SimEventReportCount == 1 && g_SimLastEventReportCeid == (unsigned)SECS_EVENT.AutoCleanClearCount;
}
std::string Facts()
{
    char b[200];
    std::snprintf(b, sizeof(b), "iIndexArmAutoCleanCnt=%d bErrorAutoClean=%d iAutoCleanAlarm=%d events=%lu ceid=%u edCleaningCount='%s'",
                  TestIF_File.iIndexArmAutoCleanCnt, (int)bErrorAutoClean, iAutoCleanAlarm, g_SimEventReportCount, g_SimLastEventReportCeid,
                  fCleaning->edCleaningCount->Text.c_str());
    return b;
}

// [4] helpers: the live lines of a file (drop // and /* */ comments, \r, and #if 0 ... #endif blocks)
std::string Slurp(const std::string& p) { std::string s; ReadAll(p, &s); return s; }
std::string LiveAfter(const std::string& text, const std::string& from, int nLines)
{
    std::size_t p = text.find(from);
    if (p == std::string::npos) return std::string();
    std::istringstream in(text.substr(p));
    std::string line, out;
    int depth0 = 0;   // nesting inside an #if 0
    for (int i = 0; i < nLines && std::getline(in, line); ++i) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (i > 0 && line == "}") break;                                        // the end of the function body
        std::string t = line;
        t.erase(0, t.find_first_not_of(" \t"));
        if (depth0 > 0) {
            if (t.compare(0, 3, "#if") == 0) ++depth0;
            else if (t.compare(0, 6, "#endif") == 0) --depth0;
            continue;
        }
        if (t.compare(0, 5, "#if 0") == 0) { depth0 = 1; continue; }
        std::size_t c = line.find("//");
        if (c != std::string::npos) line = line.substr(0, c);
        out += line + "\n";
    }
    return out;
}

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("test_nb2_w170_resetcleancount -- W-170 golden TfCleaning::btnResetCleanCountClick (906 uCleaning.cpp:2095-2117, V912 :2116-2138)\n");
    {
        const char* const rt[] = {"DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), 0};
        if (!W906TestRequireCtestRedirects("NB2_W170ResetCleanCount", rt))
            return 2;
    }
    const std::string root = argc > 1 ? argv[1] : "";
    const char* const kGuard[] = {"D:\\HT9045\\system\\Gerneral.ini", "D:\\HT9045\\config\\config.ini"};
    std::string before[2];
    bool had[2];
    for (int i = 0; i < 2; ++i) had[i] = ReadAll(kGuard[i], &before[i]);
    const char* const kTrees[] = {"D:\\HT9045\\IniData", "D:\\HT9045\\data"};
    std::map<std::string, std::string> treeBefore[2];
    for (int i = 0; i < 2; ++i) ListTree(kTrees[i], "", &treeBefore[i]);

    const char* env = std::getenv("W906_INIDATA_ROOT");
    const bool sandbox = env && *env && DataPath.Pos(env) == 1;
    Check(sandbox, std::string("[0] W906_INIDATA_ROOT sandbox is on and DataPath is under it (") + DataPath.c_str() + ")");
    if (!sandbox) { std::printf("\n%d / %d passed\n", g_pass, g_pass + g_fail); return 1; }

    SystemStart = false; SoftStart = false;
    CosFunction.bUseDefineAutoCleanOffset = false;          // else golden writes D:\HT9045\IniData\DefineAutoClean (not the sandbox)
    if (CUSTOMER_CODE == CC_TSMC_TAINAN) CUSTOMER_CODE = 0;
    CUSTOMER_CODE = 0;                                      // not KYEC: Barcode_Reader(bcAutoClean) answers 2 and the reset goes on
    IniConfig.bE43_1_AutoCleanCountSaveFolder = false;
    IniConfig.bEnableAutoCleanFunction = true;
    const int made = W906_TestEnsureSimMotors();
    Check(made > 0, "[0] sim motors attached (" + std::to_string(made) + " axes)");

    // ---------------------------------------------------------------- [1] before boot: no hook
    std::printf("[1] before FileRW_Cleaning_Boot: no hook\n");
    Check(W906_ResetCleanCountHook == nullptr, "[1] W906_ResetCleanCountHook is 0 before boot");
    Arm();
    fCleaning->btnResetCleanCountClick(fCleaning);
    Check(TestIF_File.iIndexArmAutoCleanCnt == 7 && bErrorAutoClean && g_SimEventReportCount == 0,
          "[1] no hook = the old no-op (" + Facts() + ")");

    // ---------------------------------------------------------------- [2] after boot: golden body
    std::printf("[2] after FileRW_Cleaning_Boot: the facade call runs golden\n");
    FileRW_Cleaning_Boot();
    Check(W906_ResetCleanCountHook != nullptr, "[2] boot installs W906_ResetCleanCountHook");
    Arm();
    g_lockMax = 0;
    fCleaning->btnResetCleanCountClick(fCleaning);
    Check(Reset(), "[2] golden reset: iIndexArmAutoCleanCnt 0, bErrorAutoClean false, iAutoCleanAlarm 0, one EventReport(AutoCleanClearCount) (" + Facts() + ")");
    Check(fCleaning->edCleaningCount->Text == "0", "[2] edCleaningCount shows 0 (golden :2104)");
    Check(g_lockDepth == 0 && g_lockMax >= 1, "[2] ran under FormLock, balanced");

    // ---------------------------------------------------------------- [3] csystem CheckSafeDoorForICFallDown
    std::printf("[3] csystem.cpp CheckSafeDoorForICFallDown: clean pad changed, safe door open\n");
    bHomeinitialCheckPushZ1 = false; bContractModeCheckPushZ1 = false;
    bIsTestSitICFallDown = false; bIsContactforce = false; bIsSocketSensor = false;
    for (int door : {SnSafeDoor6, SnSafeDoor3}) {           // Enable + a backend-free ISABase + Type 1 = IsOff() true (door open)
        Sen[door].Enable = true; Sen[door].ISABase = -1; Sen[door].Type = 1;
    }
    Arm();
    bChangeCleanPad = true;
    TestIF_File.iAutoClean_Tray = eCKPos_Fix3;
    CheckSafeDoorForICFallDown();
    Check(!bChangeCleanPad && Reset(), "[3] Fix3 route (golden csystem.cpp:3577): bChangeCleanPad cleared, count reset (" + Facts() + ")");
    Arm();
    bChangeCleanPad = true;
    bUse_NewAutoCleanForm = true;
    TestIF_File.iAutoClean_Tray = eCKPos_Fix3 + 1;
    CheckSafeDoorForICFallDown();
    Check(!bChangeCleanPad && Reset(), "[3] other route (golden csystem.cpp:3586): bChangeCleanPad cleared, count reset (" + Facts() + ")");
    Sen[SnSafeDoor6].Enable = false; Sen[SnSafeDoor3].Enable = false;

    // ---------------------------------------------------------------- [4] source ratchet: the remote and the Bin Select callers
    std::printf("[4] the other callers call it live (argv[1] = %s)\n", root.c_str());
    const std::string cmd = LiveAfter(Slurp(root + "/Command.cpp"), "sData[0]==\"HTSET\" && sData[1]==\"469\"", 16);
    Check(cmd.find("fCleaning->btnResetCleanCountClick(") != std::string::npos,
          "[4] Command.cpp HTSET 469 (golden :13661) calls fCleaning->btnResetCleanCountClick live");
    const std::string sbs = LiveAfter(Slurp(root + "/cShowBinSelect.cpp"), "void TfShowBinSelect::ed_AutoCleanCountClick(", 80);
    Check(sbs.find("fCleaning->btnResetCleanCountClick(") != std::string::npos,
          "[4] cShowBinSelect.cpp ed_AutoCleanCountClick (golden :2220, B5) calls fCleaning->btnResetCleanCountClick live");

    // ---------------------------------------------------------------- guard
    {
        bool same = true;
        for (int i = 0; i < 2; ++i) {
            std::string now;
            const bool has = ReadAll(kGuard[i], &now);
            if (has != had[i] || now != before[i]) { same = false; std::printf("    changed: %s\n", kGuard[i]); }
        }
        for (int i = 0; i < 2; ++i) {
            std::map<std::string, std::string> after;
            ListTree(kTrees[i], "", &after);
            if (after != treeBefore[i]) { same = false; std::printf("    changed: %s file list\n", kTrees[i]); }
        }
        Check(same, "[guard] real machine files untouched (Gerneral.ini, config.ini, D:\\HT9045\\IniData and D:\\HT9045\\data lists)");
    }
    std::printf("\n%d / %d passed\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
