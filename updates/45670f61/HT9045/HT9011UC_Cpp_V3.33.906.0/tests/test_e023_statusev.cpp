// =============================================================================
//  test_e023_statusev.cpp -- todo E-023 SB-1 / SB-3 / SB-4 / TP-2: Status.ShowBinSelect's Index-tab Auto Clean, UPH grid double-click,
//    Copy Recipe, and Status.TemperFrom's hidden Handler System entry
//
//  //AI(W906-E023-SB1) 20261002 [W906] St01 new file.  todo D:\HT9045\.claude\skills\ht9050-construction\references\todo.md E-023;
//    St02 inventory D:\HT9045\HT9011UC_Cpp_V3.33.906.0\docs\E019_DATA_STATUS_EVENTS_20261001.md 3.10 / 3.14; Jimmy RULINGS_20261001 #0, #40.
//  golden 906 D:\HT9045\backup\HT9011UC_Code_V3.33.906.0_20260618 [AI(W906-E032) 20261003] (V912 D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy in brackets;
//    AI(W906-E030-CITE) 20261003): cShowBinSelect.cpp:2101-2166 [2248-2315] btnAutoCleanClick, :2054-2099 [2201-2246] UPH_StringGridDblClick, :2845-2850 [2994-2999] sbCopyRecipeClick -> main.cpp:34444-34451 [35596-35603] RunBatchCopyRecipe; cTemperFrom.cpp:1692-1769 [1702-1779] Panel73/72/71.
//  Under test, the objects wb_serve links: cShowBinSelect_E023.cpp / cTemperFrom_E023.cpp (ht9045_sm) through the REAL act.* dispatch
//    JsonBridge/ChanAction.cpp HandleActionWithTag (:346 same line) -- "ok" computed as wb_serve.cpp:4833 does (the reply has
//    "executed":true); SB-1 through the REAL seat FileRW/TestIF_File_Cleaning.cpp FileRW_Cleaning_E023Seat (installed by
//    FileRW_Cleaning_Boot -> EvBoot) and CL-4's W906_ShowBinSelect_btnAutoCleanClick; god-stack bodies (AutoClean.cpp, csystem.cpp,
//    cCleanOut.cpp) by RESCAN.
//    [0] containment: ctest redirect roots, W906_INIDATA_ROOT sandbox, W906_COPYRECIPE_ROOT set and inside this build tree (else FAIL, exit 2)
//    [1] SB-1 refusals before anything runs: not-open, hidden (golden FormShow :803（V912 :864）), not-installed (seat missing); nothing changes.
//        No running refusal (Steven Q67 = B, 1002 08:0x): SystemStart / SoftStart go on to the next check (not-installed before the seat boots)
//    [2] boot the Cleaning page file: the seat is installed (state.autoCleanInstalled)
//    [3] SB-1 stopped, empty machine, all golden checks pass: armed (bRunAutoClean, watchdog, 5 tasks, count 0), no message, FormLock taken
//        and released by the seat
//    [4] SB-1 golden's checks: the message comes back AND is shown through golden ShowMyMessage (W906_ShowMyMessage_Count / _LastS1)
//    [5] SB-1 part in the machine: InitialAutoCleanAllTask -> BtnOneCycleClick (oneCycle).  While running too (Steven Q67 = B, 1002 08:0x),
//        SystemStart and SoftStart: part in the machine -> oneCycle (One Cycle first, then the auto clean); empty machine -> armed; golden's
//        message box (fAllMotorHome==false) -> shown through golden ShowMyMessage, as when stopped
//    [6] SB-3 UPH double-click: guards (running, level, tab, row, stale view, not open), OK / Cancel two-step with golden's text, the shift
//        (row 11 -> row 10), the average (non-number counts as 0), VTEST columns 4..6
//    [7] SB-4 Copy Recipe: golden's two commands into the W906_COPYRECIPE_ROOT sandbox (files and a sub-folder copied, contents equal),
//        Steven Q66 = B (1002 08:0x) refusal for path characters (nothing created)
//    [8] TP-2: 73 left, 72 left, 71 non-left -> YES / NO box (+ needPassword per SOFT_SIMULTE / CC_HONPREC_QC); NO; YES (+ password) ->
//        open handlersys; wrong password; wrong order; level / running / ASE / KYEC; a new click drops the box; answer re-checks guards;
//        the password is never in a reply (the literal is read from the port source, never printed)
//    [9] wiring ratchet (comments and '\r' stripped, #if 0 skipped; argv[1] = port tree, argv[2] = web\page, read-only)
//  Files: refuses to run outside ctest's sandbox.  D:\HT9045\system (file list), D:\HT9045\config\config.ini, D:\Run (exists? file list)
//    and D:\HT9045\IniData / D:\HT9045\data (file lists) must be unchanged at the end.
//  NOT COVERED: the browser half (E023_StatusPages, node); wb_serve's web message box (the ShowMyMessage hook is not installed in ctest);
//    the cleaning itself after START (ctest AutoClean / DoIndexAutoClean); HW.HandlerSys.html's own open gate (ctest OpenEnterLog etc.).
// =============================================================================
#include "JsonBridge/ChanAction.h"
#include "FileRW/_EditList.h"
#include "FileRW/_EditPage.h"
#include "FileRW/_FormEvent.h"
#include "JsonBridge/FormBridge.h"
#include "Public/cJSON.h"
#include "forms/fShowBinSelect.h"
#include "forms/fMain.h"
#include "cmydef.h"
#include "cprod.h"
#include "Config.h"
#include "CosFunction.h"
#include "MachineType.h"
#include "csystem.h"            // hAutoCleanHangUp, HasICUnderMachine, CheckIndexIsNormal
#include "w906_ctest_guard.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"   // Motor/HTMotor.h's virtual defaults (not this file's code; mymotor.h pulls it in)
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
#include <vector>

extern void FileRW_Cleaning_Boot();
extern AnsiString DataPath;      // common.cpp:225 (W906_INIDATA_ROOT)
extern AnsiString DefaultPath;   // common.cpp:224
extern AnsiString W906_ShowMyMessage_LastS1;   // canary_support.h:167 (its header cannot share a TU with cMyDB.h)
extern int W906_ShowMyMessage_Count;           // canary_support.h:168
extern std::string (*g_W906_E023_BtnAutoCleanSeat)();   // cShowBinSelect_E023.cpp
extern bool bGreen, bYellow;                   // cTemperFrom_E023.cpp (golden 906 cTemperFrom.cpp:35（V912 :36）)
bool W906_E023_RecipeNameRefused(const std::string& name, std::string* why);   // cShowBinSelect_E023.cpp
std::string W906_E023_CopyRecipeRoot();

// link-only (not the code under test), the same as tests/test_b8_cl4_autoclean.cpp: FileRW_IniConfig_ChangeCBListProperty, JsonBridge's
// FindBridge / RunEvent; FormLock as a counter (the seat must take it and give it back).
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
bool Has(const std::string& h, const std::string& n) { return h.find(n) != std::string::npos; }
bool AckOk(const std::string& r) { return Has(r, "\"executed\":true"); }   // what wb_serve.cpp:4833 does

std::string Act(const char* cmd, const std::string& value, bool quiet = false)
{
    const std::string r = ht9045::sjson::HandleActionWithTag(cmd, value, std::string());
    if (!quiet) std::printf("    %s %s -> %s\n", cmd, value.c_str(), r.substr(0, 300).c_str());
    return r;
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
bool WriteAll(const std::string& p, const std::string& s)
{
    std::ofstream f(p.c_str(), std::ios::binary);
    if (!f) return false;
    f << s;
    return (bool)f;
}

// relative path -> "size/mtime"
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
bool DirExists(const std::string& p)
{
    const DWORD a = ::GetFileAttributesA(p.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}
std::string Back(std::string s) { for (std::size_t i = 0; i < s.size(); ++i) if (s[i] == '/') s[i] = '\\'; return s; }
void MkDirs(const std::string& p)
{
    for (std::size_t i = 3; i <= p.size(); ++i)
        if (i == p.size() || p[i] == '\\') ::CreateDirectoryA(p.substr(0, i).c_str(), nullptr);
}
std::string Lower(std::string s) { for (std::size_t i = 0; i < s.size(); ++i) if (s[i] >= 'A' && s[i] <= 'Z') s[i] = (char)(s[i] - 'A' + 'a'); return s; }
// delete a sandbox folder tree (only ever called on paths that passed the e023_scratch check)
void RemoveTree(const std::string& p)
{
    WIN32_FIND_DATAA fd;
    HANDLE h = ::FindFirstFileA((p + "\\*").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            const std::string name = fd.cFileName;
            if (name == "." || name == "..") continue;
            const std::string c = p + "\\" + name;
            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) RemoveTree(c); else ::DeleteFileA(c.c_str());
        } while (::FindNextFileA(h, &fd));
        ::FindClose(h);
    }
    ::RemoveDirectoryA(p.c_str());
}

// source without // and /* */ comments (strings kept), '\r' dropped, #if 0 ... #endif dropped
std::string StripCode(const std::string& line, bool* inBlock)
{
    std::string out;
    bool inStr = false, inChr = false;
    for (std::size_t i = 0; i < line.size(); ++i) {
        const char c = line[i];
        if (*inBlock) { if (c == '*' && i + 1 < line.size() && line[i + 1] == '/') { *inBlock = false; ++i; } continue; }
        if (inStr) { out += c; if (c == '\\' && i + 1 < line.size()) { out += line[++i]; } else if (c == '"') inStr = false; continue; }
        if (inChr) { out += c; if (c == '\\' && i + 1 < line.size()) { out += line[++i]; } else if (c == '\'') inChr = false; continue; }
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '/') break;
        if (c == '/' && i + 1 < line.size() && line[i + 1] == '*') { *inBlock = true; ++i; continue; }
        if (c == '"') inStr = true;
        else if (c == '\'') inChr = true;
        out += c;
    }
    return out;
}
std::string Live(const std::string& text)
{
    std::istringstream in(text);
    std::string l, o;
    bool gated = false, inBlock = false;
    while (std::getline(in, l)) {
        if (!l.empty() && l.back() == '\r') l.pop_back();
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; o += "\n"; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; o += "\n"; continue; }
        o += StripCode(l, &inBlock) + "\n";
    }
    return o;
}
std::string StripHtmlComments(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s.compare(i, 4, "<!--") == 0) { const std::size_t e = s.find("-->", i + 4); if (e == std::string::npos) break; i = e + 2; continue; }
        if (s[i] != '\r') o += s[i];
    }
    return o;
}

// ---- SB-1 machine state (the same as ctest B8_Cl4_StartAutoClean's Ready()) ----------------------------------------------------
void Ready()
{
    SystemStart = false; SoftStart = false;
    bRunAutoClean = false; bIsASMAutoOneCycle = false; iOneCycle = 0;
    bIsAutoOneCycle = false; bManualOneCycle = false; bBackupOneCycle_ByAutoClean = false;
    fMain->BtnOneCycle->Down = false;
    IniConfig.bEnableAutoCleanFunction = true;
    TestIF.iAutoClean_Function = 1;
    TestIF.iAutoClean_Mode = M_MANUAL;
    IniConfig.bD51UseOnecycleCleanOutFinishTestArmAtRear = false;
    fAllMotorHome = true;
    Prod.iXTrayEmpty = 1000;
    MOT[MTrayX].Motor->SetPosition(2000);
    MOT[MTestZ1].Motor->Enable = false; MOT[MTestZ2].Motor->Enable = false; MOT[MTestY1].Motor->Enable = false;
    MOT[MTestZ1].MovFlag = false; MOT[MTestZ2].MovFlag = false; MOT[MTestY1].MovFlag = false;
    IndexStatus = Z1_Z2_Normal;
    MOT[MMTrayZ].fHasTray = false;
    iDoAutoCleanTask = 0; iDoShuttle1AutoCleanTask = 0; iDoShuttle2AutoCleanTask = 0; iDoShuttleAutoCleanTask = 0; iDoIndexAutoCleanTask = 0;
    iAutoClean_IndexContactCount = 7;
    fMain->AutoCleanContactCountLabel->Caption = "7";
    Prod.iHangupMaxTime = 3600;
    hAutoCleanHangUp.SetSecAndOn(0);
    fShowBinSelect->bShow = true;
}
int Tasks() { return iDoAutoCleanTask + iDoShuttle1AutoCleanTask + iDoShuttle2AutoCleanTask + iDoShuttleAutoCleanTask + iDoIndexAutoCleanTask; }
bool Untouched()
{
    return Tasks() == 0 && iAutoClean_IndexContactCount == 7 && fMain->AutoCleanContactCountLabel->Caption == "7" &&
           hAutoCleanHangUp.Off() && !bRunAutoClean && !bIsAutoOneCycle && !fMain->BtnOneCycle->Down;
}

// ---- SB-3 grid: header row 0, records 1..10 (col 3 = UPH), row 12 = average (golden CalculateUPH layout) ----------------------
void SeedGrid()
{
    TfShowBinSelectGrid* g = fShowBinSelect->UPH_StringGrid;
    for (int r = 0; r < 14; ++r) for (int c = 0; c < 7; ++c) g->Cells[c][r] = "";
    g->Cells[0][0] = "Time"; g->Cells[1][0] = "Lot"; g->Cells[2][0] = "Count"; g->Cells[3][0] = "UPH";
    for (int r = 1; r <= 10; ++r) {
        char b[32];
        std::snprintf(b, sizeof(b), "T%02d", r);  g->Cells[0][r] = b;
        std::snprintf(b, sizeof(b), "L%02d", r);  g->Cells[1][r] = b;
        std::snprintf(b, sizeof(b), "%d", r * 10); g->Cells[2][r] = b;
        std::snprintf(b, sizeof(b), "%d", r * 100); g->Cells[3][r] = b;   // 100 .. 1000
        std::snprintf(b, sizeof(b), "V%d", r); g->Cells[4][r] = b; g->Cells[5][r] = b; g->Cells[6][r] = b;
    }
    g->Cells[3][12] = "550";
    RunInfo.iAvgUPH = "550";
}
std::string Row(int r, int n = 4)
{
    std::string s = "[";
    for (int c = 0; c < n; ++c) {
        s += (c ? ",\"" : "\"");
        s += fShowBinSelect->UPH_StringGrid->Cells[c][r].c_str();
        s += "\"";
    }
    return s + "]";
}
std::string Cell(int c, int r) { return fShowBinSelect->UPH_StringGrid->Cells[c][r].c_str(); }

// ---- TP-2 -------------------------------------------------------------------------------------------------------------------
std::string Mouse(int panel, const char* button, const std::string& extra = std::string(), bool quiet = false)
{
    char b[96];
    std::snprintf(b, sizeof(b), "{\"panel\":%d,\"button\":\"%s\"", panel, button);
    return Act("act.temperFrom.mouseDown", std::string(b) + extra + "}", quiet);
}
std::string JsonEsc(const std::string& s)
{
    std::string o;
    for (std::size_t i = 0; i < s.size(); ++i) { if (s[i] == '"' || s[i] == '\\') o += '\\'; o += s[i]; }
    return o;
}

}  // namespace

int main(int argc, char** argv)
{
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    std::printf("test_e023_statusev -- E-023 golden TfShowBinSelect btnAutoCleanClick / UPH_StringGridDblClick / sbCopyRecipeClick and "
                "TfTemperFrom Panel73/72/71MouseDown (golden 906; V912 numbers in brackets)\n");
    {
        extern AnsiString as9045LogPath, asSaveEventLogPath, asGeneralPath, AuthPath;
        const char* const rt[] = { "DataPath", DataPath.c_str(), "DefaultPath", DefaultPath.c_str(), "as9045LogPath", as9045LogPath.c_str(),
                                   "asSaveEventLogPath", asSaveEventLogPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), "AuthPath", AuthPath.c_str(), 0 };
        if (!W906TestRequireCtestRedirects("E023_StatusEvents", rt))
            return 2;
    }
    // [0] the Copy Recipe seam MUST be set by ctest, inside this build tree (tests/CMakeLists.txt end); unset -> golden D:\Run -> refuse
    const char* seam = std::getenv("W906_COPYRECIPE_ROOT");
    const std::string seamRoot = seam ? Back(seam) : std::string();
    if (!seam || !*seam || Lower(seamRoot).find("\\e023_scratch\\") == std::string::npos) {
        std::printf("  FAIL: [0] W906_COPYRECIPE_ROOT is unset or not the ctest sandbox (<build>/tests/e023_scratch/...): \"%s\" -- "
                    "refusing to run (Copy Recipe would write the golden D:\\Run)\n", seam ? seam : "");
        return 2;
    }
    Check(W906_E023_CopyRecipeRoot() == seamRoot, "[0] the Copy Recipe target resolves to the sandbox " + seamRoot + " (not D:\\Run)");

    const std::string root = argc > 1 ? argv[1] : "";
    const std::string web = argc > 2 ? argv[2] : "";
    std::map<std::string, std::string> sys0, sys1, run0, run1, ini0, ini1, dat0, dat1;
    std::string cfg0, cfg1;
    ListTree("D:\\HT9045\\system", "", &sys0);
    ReadAll("D:\\HT9045\\config\\config.ini", &cfg0);
    const bool runHad = DirExists("D:\\Run");
    ListTree("D:\\Run", "", &run0);
    ListTree("D:\\HT9045\\IniData", "", &ini0);
    ListTree("D:\\HT9045\\data", "", &dat0);

    const char* inidata = std::getenv("W906_INIDATA_ROOT");
    const bool sandbox = inidata && *inidata && DataPath.Pos(inidata) == 1;
    Check(sandbox, std::string("[0] W906_INIDATA_ROOT sandbox is on and DataPath is under it (") + DataPath.c_str() + ")");

    CUSTOMER_CODE = CC_HONPREC_QC;
    SystemStart = false; SoftStart = false;
    if (sandbox) {
        CosFunction.bUseDefineAutoCleanOffset = false;     // golden LoadAutoCleanData writes a hard-coded D:\HT9045\IniData\DefineAutoClean otherwise
        IniConfig.bE43_1_AutoCleanCountSaveFolder = false;
        const int made = W906_TestEnsureSimMotors();
        Check(made > 0 && MOT[MTrayX].Motor && MOT[MTestZ1].Motor, "[0] sim motors attached (" + std::to_string(made) + " axes)");

        std::printf("[1] SB-1 refusals before anything runs\n");
        Ready();
        fShowBinSelect->bShow = false;
        std::string r = Act("act.showBinSelect.autoClean", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-open\"") && Untouched(), "[1] window not open -> not-open, nothing done");
        Ready();
        IniConfig.bEnableAutoCleanFunction = false;
        r = Act("act.showBinSelect.autoClean", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"hidden\"") && Has(r, "cShowBinSelect.cpp:803 (V912 :864)") && Untouched(),
              "[1] [Enable Auto Clean] off -> hidden (golden FormShow :803 (V912 :864) btnAutoClean->Visible), nothing done");
        Ready();   // AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B (1002 08:0x) -- no running refusal any more
        SystemStart = true;
        r = Act("act.showBinSelect.autoClean", "{}");
        const bool sysGo = !AckOk(r) && !Has(r, "\"guard\":\"running\"") && Has(r, "\"guard\":\"not-installed\"");
        SystemStart = false;
        Check(sysGo && Untouched(), "[1] SystemStart -> no running refusal (Steven Q67 = B): on to the next check (not-installed: the seat is not booted yet), nothing done");
        Ready();
        SoftStart = true;
        r = Act("act.showBinSelect.autoClean", "{}");
        const bool softGo = !AckOk(r) && !Has(r, "\"guard\":\"running\"") && Has(r, "\"guard\":\"not-installed\"");
        SoftStart = false;
        Check(softGo && Untouched(), "[1] SoftStart -> no running refusal either (not-installed), nothing done");
        Ready();
        Check(g_W906_E023_BtnAutoCleanSeat == nullptr, "[1] the seat is not installed before the Cleaning page file boots");
        r = Act("act.showBinSelect.autoClean", "{}");
        Check(!AckOk(r) && Has(r, "\"guard\":\"not-installed\"") && Untouched(), "[1] no seat -> not-installed, nothing done");
        r = Act("act.showBinSelect.state", "{}");
        Check(AckOk(r) && Has(r, "\"shown\":true") && Has(r, "\"autoCleanVisible\":true") && Has(r, "\"autoCleanInstalled\":false") &&
              Has(r, "\"copyRecipeVisible\":true"), "[1] state: shown, Auto Clean visible, seat missing, Copy Recipe visible");

        std::printf("[2] boot the Cleaning page file (FileRW_Cleaning_Boot -> EvBoot installs the seat)\n");
        FileRW_Cleaning_Boot();
        Check(g_W906_E023_BtnAutoCleanSeat != nullptr, "[2] g_W906_E023_BtnAutoCleanSeat installed by FileRW_Cleaning_EvBoot");
        r = Act("act.showBinSelect.state", "{}");
        Check(Has(r, "\"autoCleanInstalled\":true"), "[2] state.autoCleanInstalled");
        Check(g_lockDepth == 0, "[2] FormLock balanced after boot");

        std::printf("[3] SB-1 stopped, empty machine: armed\n");
        Ready();
        g_lockMax = 0;
        const int sm0 = W906_ShowMyMessage_Count;
        Check(!HasICUnderMachine() && CheckIndexIsNormal(), "[3] precondition: HasICUnderMachine()==false, CheckIndexIsNormal()==true");
        r = Act("act.showBinSelect.autoClean", "{}");
        Check(AckOk(r) && Has(r, "\"result\":\"armed\"") && Has(r, "\"running\":false") && Has(r, "\"messages\":[]"),
              "[3] executed, result armed, running false (pressed while stopped), no message");
        Check(bRunAutoClean && !hAutoCleanHangUp.Off(), "[3] bRunAutoClean = true and the hang-up watchdog is on (golden :2159-2160 (V912 :2308-2309))");
        Check(Tasks() == 5 && iAutoClean_IndexContactCount == 0 && fMain->AutoCleanContactCountLabel->Caption == "0",
              "[3] five tasks registered, contact count 0 (golden :2112-2116 (V912 :2261-2265))");
        Check(!bIsAutoOneCycle && !fMain->BtnOneCycle->Down, "[3] no One Cycle (empty machine)");
        Check(W906_ShowMyMessage_Count == sm0, "[3] golden showed no message box");
        Check(g_lockMax >= 1 && g_lockDepth == 0, "[3] the seat took FormLock around the golden body and released it");
        r = Act("act.showBinSelect.autoClean", "{}");
        Check(AckOk(r) && Has(r, "\"result\":\"nothing\"") && bRunAutoClean, "[3] second press while bRunAutoClean: golden :2103 (V912 :2250) return");

        std::printf("[4] SB-1 golden's checks -> golden ShowMyMessage\n");
        {
            struct Case { const char* what; void (*set)(); const char* en; };
            const Case cases[] = {
                {"fAllMotorHome==false (golden :2124-2127 (V912 :2273-2276))", [] { fAllMotorHome = false; }, "Auto Clean Need Home"},
                {"MOT[MTrayX] < Prod.iXTrayEmpty (golden :2130-2134 (V912 :2279-2283))", [] { MOT[MTrayX].Motor->SetPosition(999); }, "Tray arm not Safe pos"},
                {"CheckIndexIsNormal()==false (golden :2136-2140 (V912 :2285-2289))", [] { MOT[MTestZ1].MovFlag = true; }, "Index arm Z axis is not in safe position."},
                {"IndexStatus!=Z1_Z2_Normal (golden :2150-2156 (V912 :2299-2305))", [] { IndexStatus = IndexIsBack; }, "Index arm axis is not in standy position."},
            };
            for (const Case& c : cases) {
                Ready();
                c.set();
                const int n0 = W906_ShowMyMessage_Count;
                r = Act("act.showBinSelect.autoClean", "{}");
                Check(AckOk(r) && Has(r, "\"result\":\"nothing\"") && Has(r, std::string("\"en\":\"") + c.en + "\"") && !bRunAutoClean,
                      std::string("[4] ") + c.what + ": message \"" + c.en + "\" in the reply, bRunAutoClean stays false");
                Check(W906_ShowMyMessage_Count == n0 + 1 && std::string(W906_ShowMyMessage_LastS1.c_str()) == c.en,
                      std::string("[4] ") + c.what + ": shown once through golden ShowMyMessage (LastS1 \"" + W906_ShowMyMessage_LastS1.c_str() + "\")");
                Check(g_lockDepth == 0, std::string("[4] ") + c.what + ": FormLock released before the message box");
            }
            Ready();
            fAllMotorHome = false;
            r = Act("act.showBinSelect.autoClean", "{}");
            Check(Has(r, "Auto Clean \xe9\x9c\x80\xe8\xa6\x81\xe6\xad\xb8\xe9\x9b\xb6"), "[4] the home message carries golden's Chinese S2");
        }

        std::printf("[5] SB-1 part in the machine: One Cycle first\n");
        Ready();
        MOT[MMTrayZ].fHasTray = true;
        r = Act("act.showBinSelect.autoClean", "{}");
        Check(AckOk(r) && Has(r, "\"result\":\"oneCycle\"") && !bRunAutoClean && bIsAutoOneCycle && fMain->BtnOneCycle->Down,
              "[5] InitialAutoCleanAllTask -> TfMain::BtnOneCycleClick: bIsAutoOneCycle, BtnOneCycle->Down, bRunAutoClean not set");
        MOT[MMTrayZ].fHasTray = false;
        // AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B (1002 08:0x) -- allowed while running, as golden: One Cycle first, then the auto clean
        for (int st = 0; st < 2; ++st) {
            const std::string sn = st == 0 ? "SystemStart" : "SoftStart";
            Ready();
            MOT[MMTrayZ].fHasTray = true;
            SystemStart = st == 0; SoftStart = st == 1;
            r = Act("act.showBinSelect.autoClean", "{}");
            const bool runOne = AckOk(r) && Has(r, "\"result\":\"oneCycle\"") && Has(r, "\"running\":true") && !bRunAutoClean && bIsAutoOneCycle &&
                                fMain->BtnOneCycle->Down;
            SystemStart = false; SoftStart = false;
            MOT[MMTrayZ].fHasTray = false;
            Check(runOne, "[5] " + sn + " + part in the machine -> allowed (Steven Q67 = B): oneCycle (bIsAutoOneCycle, BtnOneCycle->Down), "
                  "bRunAutoClean not set yet -- One Cycle first, then the auto clean");
            Ready();
            SystemStart = st == 0; SoftStart = st == 1;
            const int n0 = W906_ShowMyMessage_Count;
            r = Act("act.showBinSelect.autoClean", "{}");
            const bool runArm = AckOk(r) && Has(r, "\"result\":\"armed\"") && Has(r, "\"running\":true") && bRunAutoClean && !hAutoCleanHangUp.Off() &&
                                W906_ShowMyMessage_Count == n0;
            SystemStart = false; SoftStart = false;
            Check(runArm, "[5] " + sn + " + empty machine, golden's checks pass -> armed (bRunAutoClean, hang-up watchdog), no message");
            Ready();
            fAllMotorHome = false;
            SystemStart = st == 0; SoftStart = st == 1;
            const int n1 = W906_ShowMyMessage_Count;
            r = Act("act.showBinSelect.autoClean", "{}");
            const bool runMsg = AckOk(r) && Has(r, "\"result\":\"nothing\"") && Has(r, "\"en\":\"Auto Clean Need Home\"") && !bRunAutoClean &&
                                W906_ShowMyMessage_Count == n1 + 1;
            SystemStart = false; SoftStart = false;
            Check(runMsg, "[5] " + sn + " + empty machine, fAllMotorHome==false -> golden's box through golden ShowMyMessage, as when stopped "
                  "(wb_serve's box then stops the motors, golden 906 mymessbox.cpp:302-309 (V912 :303-310); not in ctest)");
        }
        Ready();
        bRunAutoClean = false;
    }

    std::printf("[6] SB-3 UPH grid double-click\n");
    {
        fShowBinSelect->bShow = true;
        SystemStart = false;
        AccessLevel = iDefHonPrecLevel;
        IniConfig.bVTESTFunction = false;
        SeedGrid();
        const std::string row3 = Row(3);
        std::string r = Act("act.showBinSelect.uphDblClick", "{\"row\":3,\"pageIndex\":2,\"cells\":" + row3 + "}");
        Check(!AckOk(r) && Has(r, "\"needConfirm\":true") && Has(r, "\"prompt\":[\"Do you want to delete this record?\",\"Confirm\"]") &&
              Has(r, "\"buttons\":\"okcancel\"") && Has(r, "cShowBinSelect.cpp:2063 (V912 :2210)"), "[6] no answer -> needConfirm with golden's text and caption (:2063, V912 :2210)");
        Check(Cell(3, 3) == "300" && Cell(3, 12) == "550", "[6] needConfirm: nothing changed");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":3,\"pageIndex\":2,\"cells\":" + row3 + ",\"answer\":\"cancel\"}");
        Check(AckOk(r) && Has(r, "\"deleted\":false") && Cell(3, 3) == "300" && Cell(0, 3) == "T03", "[6] Cancel -> executed, nothing deleted");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":3,\"pageIndex\":2,\"cells\":" + row3 + ",\"answer\":\"ok\"}");
        Check(AckOk(r) && Has(r, "\"deleted\":true"), "[6] OK -> executed, deleted");
        Check(Cell(0, 3) == "T04" && Cell(3, 3) == "400" && Cell(0, 9) == "T10" && Cell(3, 9) == "1000" && Cell(0, 10) == "" && Cell(3, 10) == "",
              "[6] rows 4..10 moved up one, row 10 takes row 11 (empty) (golden :2065-2077 (V912 :2212-2224))");
        // remaining 100,200,400,...,1000 = 5500-300 = 5200 over 9 -> 577
        Check(std::string(RunInfo.iAvgUPH.c_str()) == "577" && Cell(3, 12) == "577", "[6] average over the 9 left = 577 into RunInfo.iAvgUPH and Cells[3][12] (golden :2079-2095 (V912 :2226-2242))");
        Check(Cell(4, 3) == "V3", "[6] VTEST off: columns 4..6 do not move (golden :2071 (V912 :2218))");
        SeedGrid();
        IniConfig.bVTESTFunction = true;
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":1,\"pageIndex\":2,\"cells\":" + Row(1, 7) + ",\"answer\":\"ok\"}", true);
        IniConfig.bVTESTFunction = false;
        Check(AckOk(r) && Cell(4, 1) == "V2" && Cell(6, 9) == "V10" && Cell(6, 10) == "", "[6] VTEST on: columns 4..6 move too (golden :2071-2076 (V912 :2218-2223))");
        SeedGrid();
        fShowBinSelect->UPH_StringGrid->Cells[3][5] = "abc";
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":10,\"pageIndex\":2,\"cells\":" + Row(10) + ",\"answer\":\"ok\"}", true);
        // left: 100,200,300,400,abc(=0),600,700,800,900 -> 4000 / 9 = 444
        Check(AckOk(r) && Cell(3, 12) == "444", "[6] golden oddity: a non-number counts as 0 but still counts (atoi) -> 444, got " + Cell(3, 12));

        SeedGrid();
        const std::string row2 = Row(2);
        SystemStart = true;
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2,\"cells\":" + row2 + ",\"answer\":\"ok\"}");
        SystemStart = false;
        Check(Has(r, "\"guard\":\"running\"") && Has(r, ":2057 (V912 :2204)") && Cell(0, 2) == "T02", "[6] SystemStart -> running (golden :2057 (V912 :2204)), nothing deleted");
        AccessLevel = iDefHonPrecLevel - 1;
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2,\"cells\":" + row2 + ",\"answer\":\"ok\"}");
        AccessLevel = iDefHonPrecLevel;
        Check(Has(r, "\"guard\":\"not-authorized\"") && Cell(0, 2) == "T02", "[6] AccessLevel < iDefHonPrecLevel -> not-authorized (golden :2057 (V912 :2204))");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":1,\"cells\":" + row2 + ",\"answer\":\"ok\"}");
        Check(Has(r, "\"guard\":\"not-uph-tab\"") && Cell(0, 2) == "T02", "[6] another tab active -> not-uph-tab (golden :2058 (V912 :2205) ActivePageIndex==2)");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":0,\"pageIndex\":2,\"cells\":" + Row(0) + ",\"answer\":\"ok\"}");
        Check(Has(r, "\"guard\":\"bad-row\"") && Cell(0, 0) == "Time", "[6] header row 0 -> bad-row (golden :2061 (V912 :2208))");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":11,\"pageIndex\":2,\"cells\":" + Row(11) + ",\"answer\":\"ok\"}");
        Check(Has(r, "\"guard\":\"bad-row\""), "[6] row 11 -> bad-row (golden :2061 (V912 :2208) iRow<11)");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2,\"cells\":[\"T02\",\"L02\",\"20\",\"999\"],\"answer\":\"ok\"}");
        Check(Has(r, "\"guard\":\"stale-view\"") && Cell(3, 2) == "200", "[6] the page showed another record -> stale-view, nothing deleted");
        fShowBinSelect->bShow = false;
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2,\"cells\":" + row2 + ",\"answer\":\"ok\"}");
        fShowBinSelect->bShow = true;
        Check(Has(r, "\"guard\":\"not-open\"") && Cell(0, 2) == "T02", "[6] window not open -> not-open");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2,\"cells\":" + row2 + ",\"answer\":\"yes\"}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[6] answer \"yes\" -> bad-payload (golden MB_OKCANCEL: ok / cancel)");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2.5,\"pageIndex\":2,\"cells\":" + row2 + "}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[6] fractional row -> bad-payload");
        r = Act("act.showBinSelect.uphDblClick", "{\"row\":2,\"pageIndex\":2}");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[6] no cells -> bad-payload");
    }

    std::printf("[7] SB-4 Copy Recipe\n");
    if (sandbox) {
        fShowBinSelect->bShow = true;
        const AnsiString dp0 = DataPath;
        DataPath = AnsiString(Back(DataPath.c_str()).c_str());                 // cmd needs backslashes; the location is the same sandbox
        const std::string src = std::string(DataPath.c_str()) + "E023Recipe";
        MkDirs(src + "\\sub");                                                 // DataPath\E023Recipe\sub (every missing level, inside the sandbox)
        const bool seeded = WriteAll(src + "\\Recipe.ini", "[E023]\r\nKey=1\r\n") && WriteAll(src + "\\sub\\Part.txt", "e023 sub file\r\n");
        Check(seeded, "[7] sandbox recipe folder " + src + " seeded (Recipe.ini + sub\\Part.txt)");
        RemoveTree(seamRoot);                                                  // fresh target (it is the e023_scratch sandbox, checked at [0])
        Check(!DirExists(seamRoot), "[7] target " + seamRoot + " does not exist before the press (golden mkdir must create it)");
        const AnsiString fn0 = fMain->cbSetupFileName->Text;
        fMain->cbSetupFileName->Text = "E023Recipe";
        std::string r = Act("act.showBinSelect.copyRecipe", "{}");
        Check(AckOk(r) && Has(r, "\"rc\":[0,0]"), "[7] executed, mkdir and xcopy both returned 0");
        Check(Has(r, "\"" + JsonEsc("if not exist " + seamRoot + " mkdir " + seamRoot) + "\"") &&
              Has(r, "\"" + JsonEsc("xcopy /y \"" + src + "\\*\" \"" + seamRoot + "\\\" /s /e") + "\""),
              "[7] golden's two command lines (906 main.cpp:34447 (V912 :35599) / :34449 (V912 :35601)) with the seam root");
        std::string a, b;
        Check(ReadAll(seamRoot + "\\Recipe.ini", &a) && a == "[E023]\r\nKey=1\r\n" && ReadAll(seamRoot + "\\sub\\Part.txt", &b) && b == "e023 sub file\r\n",
              "[7] the recipe folder was copied into the sandbox target, sub-folder included (xcopy /s /e), contents equal");
        r = Act("act.showBinSelect.copyRecipe", "{}", true);
        Check(AckOk(r) && Has(r, "\"rc\":[0,0]"), "[7] a second press overwrites (xcopy /y), still 0 / 0");
        const char* const bad[] = { "..\\E023Recipe", "a|b", "x\"y", "c:z", "p/q", 0 };
        for (int i = 0; bad[i]; ++i) {
            RemoveTree(seamRoot);
            fMain->cbSetupFileName->Text = bad[i];
            r = Act("act.showBinSelect.copyRecipe", "{}", true);
            Check(!AckOk(r) && Has(r, "\"guard\":\"recipe-name\"") && !DirExists(seamRoot),
                  std::string("[7] recipe name \"") + bad[i] + "\" -> recipe-name (Steven Q66 = B), nothing created");
        }
        std::string why;
        Check(!W906_E023_RecipeNameRefused("Normal_Recipe-01 A", &why) && !W906_E023_RecipeNameRefused("", &why),
              "[7] ordinary names and the empty name (golden) pass the Q66 = B rule");
        fShowBinSelect->bShow = false;
        r = Act("act.showBinSelect.copyRecipe", "{}", true);
        fShowBinSelect->bShow = true;
        Check(Has(r, "\"guard\":\"not-open\""), "[7] window not open -> not-open");
        SystemStart = true;
        fMain->cbSetupFileName->Text = "E023Recipe";
        r = Act("act.showBinSelect.copyRecipe", "{}", true);
        SystemStart = false;
        Check(AckOk(r), "[7] allowed while running (golden has no running check; flagged B)");
        fMain->cbSetupFileName->Text = fn0;
        RemoveTree(seamRoot);
        RemoveTree(src);
        DataPath = dp0;
    }

    std::printf("[8] TP-2 hidden Handler System entry\n");
    std::string lit;   // golden's password literal, read from the port source; never printed
    {
        {
            std::string src;
            if (!root.empty() && ReadAll(root + "/cTemperFrom_E023.cpp", &src)) {
                const std::string key = "edPasswordText!=\"";
                const std::string live = Live(src);
                const std::size_t p = live.find(key);
                if (p != std::string::npos) {
                    const std::size_t e = live.find('"', p + key.size());
                    if (e != std::string::npos) lit = live.substr(p + key.size(), e - p - key.size());
                }
            }
            Check(!lit.empty() && lit.find("@@") == std::string::npos, "[8] golden's password literal found in cTemperFrom_E023.cpp (" +
                  std::to_string(lit.size()) + " characters; the value is not printed)");
            std::string g;
            if (ReadAll("D:\\HT9045\\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\\cTemperFrom.cpp", &g)) {
                Check(!lit.empty() && Has(g, "edPassword->Text!=\"" + lit + "\""), "[8] it equals golden V912 cTemperFrom.cpp:1771 (= 906 :1761; value not printed)");
            } else {
                std::printf("    (golden V912 tree not readable here -- the equality check is skipped)\n");
            }
        }
#ifdef SOFT_SIMULTE
        const bool simCfg = true;
#else
        const bool simCfg = false;
#endif
        AccessLevel = iDefHonPrecLevel;
        SystemStart = false;
        std::string r;
        for (int qc = 0; qc < 2; ++qc) {
            CUSTOMER_CODE = qc ? CC_HONPREC_QC : CC_Greatek;   // CC_HONPREC_QC is 0 (MachineType.h:224); Greatek = an ordinary customer (no ASE / KYEC rule here)
            const bool needPw = !simCfg && !qc;
            const std::string tag = std::string("[8] ") + (simCfg ? "SIM" : "SHIP") + (qc ? " CC_HONPREC_QC" : " CC_Greatek") + ": ";
            bGreen = bYellow = false;
            Mouse(73, "left");
            Check(bYellow && !bGreen, tag + "73 left -> bYellow (golden :1697-1700 (V912 :1707-1710))");
            Mouse(72, "left");
            Check(bYellow && bGreen, tag + "72 left after yellow -> bGreen (golden :1717-1719 (V912 :1727-1729))");
            r = Mouse(71, "right");
            Check(!AckOk(r) && Has(r, "\"needConfirm\":true") && Has(r, "Reset the hardware apparatus information?") &&
                  Has(r, std::string("\"needPassword\":") + (needPw ? "true" : "false")),
                  tag + "71 right -> YES/NO box (:1752, V912 :1762), needPassword=" + (needPw ? "true" : "false") + " (golden #ifndef SOFT_SIMULTE / HONPREC_QC :1755-1756 (V912 :1765-1766))");
            Check(!bGreen && !bYellow, tag + "both flags reset before the box (golden :1750-1751 (V912 :1760-1761))");
            r = Mouse(71, "right", ",\"answer\":\"no\"");
            Check(AckOk(r) && Has(r, "\"opened\":false"), tag + "NO -> nothing opened (golden :1752-1753 (V912 :1762-1763))");
            r = Mouse(71, "right", ",\"answer\":\"yes\"");
            Check(!AckOk(r) && Has(r, "\"guard\":\"not-asked\""), tag + "an answer without an open box -> not-asked");
            Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(71, "middle", "", true);
            r = Mouse(71, "right", ",\"answer\":\"yes\"" + (needPw ? ",\"password\":\"" + JsonEsc(lit) + "\"" : std::string()), true);
            Check(AckOk(r) && Has(r, "\"opened\":true") && Has(r, "\"open\":\"handlersys\""),
                  tag + "73 left, 72 left, 71 middle (any non-left button), YES" + (needPw ? " + golden's password" : "") + " -> open handlersys (:1768, V912 :1778)");
            Check(lit.empty() || !Has(r, lit), tag + "the password is not in the reply");
            if (needPw) {
                Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(71, "right", "", true);
                r = Mouse(71, "right", ",\"answer\":\"yes\",\"password\":\"wrong-1\"", true);
                Check(AckOk(r) && Has(r, "\"opened\":false") && Has(r, ":1761-1764 (V912 :1771-1774)"), tag + "wrong password -> nothing opened (golden :1761-1764 (V912 :1771-1774))");
                Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(71, "right", "", true);
                r = Mouse(71, "right", ",\"answer\":\"yes\"", true);
                Check(Has(r, "\"opened\":false"), tag + "YES without a password -> nothing opened (an empty edPassword)");
            }
        }
        CUSTOMER_CODE = CC_HONPREC_QC;
        // order / button / guards
        Mouse(72, "left", "", true); Mouse(73, "left", "", true);
        r = Mouse(71, "right");
        Check(AckOk(r) && Has(r, "\"opened\":false") && Has(r, ":1744-1749 (V912 :1754-1759)"), "[8] 72 before 73 -> 71 returns at :1744 (V912 :1754) (no bGreen)");
        Mouse(73, "left", "", true); Mouse(72, "left", "", true);
        r = Mouse(71, "left");
        Check(Has(r, ":1744-1749 (V912 :1754-1759)") && !bGreen && !bYellow, "[8] 71 LEFT -> returns at :1744 (V912 :1754) and resets both flags");
        Mouse(73, "left", "", true); Mouse(73, "right", "", true);
        Check(!bGreen && !bYellow, "[8] 73 right -> both flags reset (golden :1702-1705 (V912 :1712-1715))");
        Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(72, "left", "", true);
        Check(!bGreen && !bYellow, "[8] 72 left again with bGreen set -> both reset (golden :1721-1724 (V912 :1731-1734))");
        Mouse(73, "left", "", true); Mouse(72, "left", "", true);
        SystemStart = true;
        r = Mouse(71, "right");
        SystemStart = false;
        Check(Has(r, ":1741-1742 (V912 :1751-1752)") && bGreen && bYellow, "[8] SystemStart -> returns at :1741 (V912 :1751) before touching the flags");
        AccessLevel = iDefHonPrecLevel - 1;
        r = Mouse(71, "right");
        AccessLevel = iDefHonPrecLevel;
        Check(Has(r, ":1741-1742 (V912 :1751-1752)"), "[8] AccessLevel < iDefHonPrecLevel -> returns at :1741 (V912 :1751)");
        CUSTOMER_CODE = CC_ASE_KaohSiung;
        r = Mouse(71, "right");
        Check(Has(r, ":1737-1739 (V912 :1747-1749)"), "[8] CC_ASE_KaohSiung -> returns at :1737 (V912 :1747)");
        CUSTOMER_CODE = CC_KYEC_LEE;
        bGreen = bYellow = false;
        Mouse(73, "left", "", true);
        Check(!bYellow, "[8] CC_KYEC_LEE -> 73 returns at :1695 (V912 :1705) (no bYellow)");
        CUSTOMER_CODE = CC_HONPREC_QC;
        // a new click drops the box; the answer re-checks the guards
        Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(71, "right", "", true);
        Mouse(73, "left", "", true);
        r = Mouse(71, "right", ",\"answer\":\"yes\"");
        Check(Has(r, "\"guard\":\"not-asked\""), "[8] a new mouse-down drops the unanswered box -> the answer is not-asked");
        Mouse(73, "left", "", true); Mouse(72, "left", "", true); Mouse(71, "right", "", true);
        SystemStart = true;
        r = Mouse(71, "right", ",\"answer\":\"yes\"");
        SystemStart = false;
        Check(AckOk(r) && Has(r, "\"opened\":false") && Has(r, ":1741-1742 (V912 :1751-1752)"), "[8] machine started while the box was open -> the answer re-checks :1741 (V912 :1751), nothing opened");
        r = Mouse(70, "left");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[8] panel 70 -> bad-payload");
        r = Mouse(73, "up");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[8] button \"up\" -> bad-payload");
        r = Mouse(73, "left", ",\"answer\":\"yes\"");
        Check(Has(r, "\"guard\":\"bad-payload\""), "[8] an answer on Panel73 -> bad-payload");
        r = Act("act.temperFrom.nope", "{}");
        Check(Has(r, "\"guard\":\"unknown-action\""), "[8] unknown act.temperFrom.* -> unknown-action");
        r = Act("act.showBinSelect.nope", "{}");
        Check(Has(r, "\"guard\":\"unknown-action\""), "[8] unknown act.showBinSelect.* -> unknown-action");
        r = Act("act.showBinSelect.clearCount", "{}");
        Check(Has(r, "\"guard\":\"wrong-route\""), "[8] act.showBinSelect.clearCount here -> wrong-route (it has its own wb_serve arm)");
        r = Act("act.main.noSuchThing", "{}");
        Check(Has(r, "unknown-action") && !Has(r, "E023"), "[8] other act.* still reach ChanAction's own refusal");
    }

    std::printf("[9] wiring ratchet\n");
    if (root.empty() || web.empty()) {
        Check(false, "[9] argv[1] (port tree) and argv[2] (web\\page) are required");
    } else {
        std::string s, sb, tp, cl, cm, html, js, thtml, tjs, wire;
        Check(ReadAll(root + "/JsonBridge/ChanAction.cpp", &s), "[9] read JsonBridge/ChanAction.cpp");
        s = Live(s);
        Check(Has(s, "if (cmd.compare(0, 18, \"act.showBinSelect.\") == 0) { std::string W906_ShowBinSelectAct(const std::string&, const std::string&); return W906_ShowBinSelectAct(cmd, payloadJson); }") &&
              Has(s, "if (cmd.compare(0, 15, \"act.temperFrom.\") == 0) { std::string W906_TemperFromAct(const std::string&, const std::string&); return W906_TemperFromAct(cmd, payloadJson); }"),
              "[9] ChanAction.cpp: the two dispatches are live code");
        Check(ReadAll(root + "/CMakeLists.txt", &cm) && Has(cm, "    SmartDiagnostic.cpp  cShowBinSelect_E023.cpp  cTemperFrom_E023.cpp  #"),
              "[9] CMakeLists.txt: the two new files sit on the ht9045_sm source line");
        Check(ReadAll(root + "/cShowBinSelect_E023.cpp", &sb), "[9] read cShowBinSelect_E023.cpp");
        sb = Live(sb);
        Check(Has(sb, "const std::string session = g_W906_E023_BtnAutoCleanSeat();") && !Has(sb, "bRunAutoClean=true") &&
              !Has(sb, "hAutoCleanHangUp.SetSecAndOn") && !Has(sb, "InitialAutoCleanTask(") && !Has(sb, "InitialAutoCleanAllTask("),
              "[9] SB-1 calls CL-4's body through the seat -- no second copy of golden btnAutoCleanClick");
        {   // AI(W906-E023-Q67B) 20261002 [W906] (St01): Steven Q67 = B -- the running refusal is gone from E023_AutoClean
            const std::size_t a0 = sb.find("std::string E023_AutoClean()");
            const std::size_t a1 = a0 == std::string::npos ? a0 : sb.find("\n}\n", a0);
            const std::string body = a1 == std::string::npos ? std::string() : sb.substr(a0, a1 - a0);
            Check(!body.empty() && !Has(body, "E023_Refuse(ck, \"running\"") && Has(body, "const bool running0 = SystemStart || SoftStart;") &&
                  !Has(body, "if (SystemStart") && !Has(body, "if (SoftStart"),
                  "[9] SB-1 E023_AutoClean has no running refusal (Steven Q67 = B): no \"running\" guard; SystemStart / SoftStart are only reported");
        }
        Check(Has(sb, "W906_FormShowing(\"fShowBinSelect\", f->bShow)"), "[9] window state read through W906_FormShowing (FShow_Audit rule)");
        Check(Has(sb, "out->rc1 = system(command.c_str());") && Has(sb, "out->rc2 = system(command.c_str());") &&
              Has(sb, "std::getenv(\"W906_COPYRECIPE_ROOT\")") && Has(sb, "std::string(\"D:\\\\Run\")"),
              "[9] SB-4: golden's two system() calls, the seam with the golden literal as the default");
        Check(ReadAll(root + "/cTemperFrom_E023.cpp", &tp), "[9] read cTemperFrom_E023.cpp");
        const std::string tpl = Live(tp);
        Check(!Has(tpl, "fAutoTeach->DoAutoTeachProcess();") && Has(tp, "fAutoTeach->DoAutoTeachProcess();"),
              "[9] TP-2: Panel73's DoAutoTeachProcess stays in its SAFETY-GATE #if 0 (TfAutoTeach not ported)");
        Check(!Has(tp, std::string("@@E023_TP2_") + "GOLDEN_LITERAL@@"), "[9] TP-2: the placeholder was replaced by golden's literal");
        const std::string* const newFiles[] = { &sb, &tpl };
        for (const std::string* f : newFiles)
            Check(!Has(*f, "->Start(") && !Has(*f, "W906_RemoteRunStart(") && !Has(*f, "StartFromWeb("), "[9] no START call in the new file");
        Check(ReadAll(root + "/FileRW/TestIF_File_Cleaning.cpp", &cl), "[9] read FileRW/TestIF_File_Cleaning.cpp");
        cl = Live(cl);
        Check(Has(cl, "{ void FileRW_Cleaning_E023InstallSeat(); FileRW_Cleaning_E023InstallSeat(); }") &&
              Has(cl, "g_W906_E023_BtnAutoCleanSeat = &FileRW_Cleaning_E023Seat;") && Has(cl, "W906_ShowBinSelect_btnAutoCleanClick();"),
              "[9] TestIF_File_Cleaning.cpp: EvBoot installs the seat, the seat calls CL-4's body");
        Check(ReadAll(web + "/Status.ShowBinSelect.html", &html) && ReadAll(web + "/ht9045_showbinselect_ev.js", &js), "[9] read the BinSelect page files");
        html = StripHtmlComments(html);
        Check(Has(html, "<script src=\"ht9045_showbinselect_ev.js\"></script>") && Has(html, "id=\"btnAutoClean\"") && Has(html, "id=\"sbCopyRecipe\""),
              "[9] Status.ShowBinSelect.html loads ht9045_showbinselect_ev.js and has btnAutoClean / sbCopyRecipe");
        js = Live(js);
        Check(Has(js, "'act.showBinSelect.autoClean'") && Has(js, "'act.showBinSelect.uphDblClick'") && Has(js, "'act.showBinSelect.copyRecipe'") &&
              Has(js, "'act.showBinSelect.state'"), "[9] ht9045_showbinselect_ev.js sends the four act names");
        Check(ReadAll(web + "/Status.TemperFrom.html", &thtml) && ReadAll(web + "/ht9045_temperfrom_ev.js", &tjs), "[9] read the TemperFrom page files");
        Check(!lit.empty() && !Has(thtml, lit) && !Has(tjs, lit), "[9] the password literal is nowhere in Status.TemperFrom.html / ht9045_temperfrom_ev.js (comments included)");
        const std::string th = StripHtmlComments(thtml);
        Check(Has(th, "<script src=\"ht9045_temperfrom_ev.js\"></script>") && Has(th, "id=\"Panel71\"") && Has(th, "id=\"Panel72\"") && Has(th, "id=\"Panel73\"") &&
              !Has(th, "prompt("), "[9] Status.TemperFrom.html: three panels, loads ht9045_temperfrom_ev.js, no prompt() compare left");
        Check(Has(Live(tjs), "'act.temperFrom.mouseDown'"), "[9] ht9045_temperfrom_ev.js sends act.temperFrom.mouseDown");
        Check(ReadAll(web + "/ht9045_showbinselect_wire.js", &wire) && Has(Live(wire), "h += '<tr data-r=\"' + r + '\">';"),
              "[9] ht9045_showbinselect_wire.js marks each UPH row with data-r (same line)");
    }

    ListTree("D:\\HT9045\\system", "", &sys1);
    ReadAll("D:\\HT9045\\config\\config.ini", &cfg1);
    ListTree("D:\\Run", "", &run1);
    ListTree("D:\\HT9045\\IniData", "", &ini1);
    ListTree("D:\\HT9045\\data", "", &dat1);
    Check(sys0 == sys1, "[guard] D:\\HT9045\\system: no file written (names, sizes, mtimes)");
    Check(cfg0 == cfg1, "[guard] D:\\HT9045\\config\\config.ini unchanged");
    Check(DirExists("D:\\Run") == runHad && run0 == run1, "[guard] D:\\Run untouched (golden Copy Recipe target)");
    Check(ini0 == ini1, "[guard] D:\\HT9045\\IniData file list unchanged");
    Check(dat0 == dat1, "[guard] D:\\HT9045\\data file list unchanged");
    std::printf("%d/%d checks passed (E023_StatusEvents)\n", g_pass, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
