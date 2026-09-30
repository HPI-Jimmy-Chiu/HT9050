// =============================================================================
//  tests/test_sysinit_boot.cpp  --  ctest SysinitBoot
//
//  AI(W906-SYSINIT) 20260930: pins the translation of golden TfMain::FormShow main.cpp:9659 `SystemInitialOK=true;`
//    (golden 906 numbering; the V912 main repo has it at :10092).  It is golden's ONLY writer of the flag; before
//    INBOX 127 the port had none outside tests (cmydef.cpp:332 `bool SystemInitialOK=false;` was all there was), so
//    ScanPannelKey answered -1 on every call (ckernel.cpp:3152-3153, golden ckernel.cpp:1924-1925), DoPanelLamp never
//    lit a lamp (ckernel.cpp:2817-2818, golden :1730-1731) and ShowMyMessage never stopped the motors.
//    The translation is W906_FRWBoot_SystemInitialOK() (FileRW/MainBoot.cpp, end of file), called once from the
//    wb_serve boot sequence (tools/wb_serve.cpp, on the W906_FRWBoot_ShowLotInfoDownloadFlag line).
//
//  [1] BEFORE BOOT: SystemInitialOK==false (the cmydef.cpp:332 initialiser).  With the front Pause key held and every
//      other ScanPannelKey guard open (safe lock released, no employee-ID lockout, front pad selected), ScanPannelKey
//      answers -1 AND bFrontPadActive is not assigned -- so the -1 is golden :1924-1925's return, which precedes the
//      pad-select at golden :1927 (same two-sided proof as W7_L2_CKernel PART A1).  DoPanelLamp writes nothing.
//  [2] MODEL-READ ERROR: with bHandlerModel==false the boot step leaves the flag false -- golden FormShow returns at
//      :9162-9167 (MessageDlg + Application->Terminate()) long before :9659.
//  [3] AFTER BOOT: SystemInitialOK==true; the SAME held key is now answered (SnFKPause), the pad selection has run,
//      and DoPanelLamp drives SW[SwRKManualStep] from bLampManualSetp (its unconditional tail, golden :1895-1896).
//      A second boot-step call keeps it true (golden: set once, never cleared).
//  [4] SOURCE PINS (tools/wb_serve.cpp, FileRW/MainBoot.cpp, read-only): the call is there exactly once, on the
//      ShowLotInfoDownloadFlag line, right after that step and before W906_BootTestCategory; the order
//      W906_DoReadLastData(true,..) < ShowLotInfoDownloadFlag < SYSINIT < BootTestCategory < MyDBUpdateDB < PumpInit
//      is golden's (906 :9562 < :9610 < :9659 < V912 :10594 < 906 :10222 < :10464); MainBoot.cpp's code has exactly
//      one `SystemInitialOK=true` and neither file's code ever assigns false (golden never clears it).
//
//  IO: the sim sensor seam of tests/test_w7_l2_ckernel.cpp (Sen[].Enable/Type/ISABase), no IO table is read.  The
//    switches it looks at are Enable=false, so TMySwitch::On/Off only change OutValue (myswitch.cpp:74-88/:124-138).
//  Writes no file.  Guarded by w906_ctest_guard.h like every test that links the machine libraries.
//  Mutations (20260930, recorded in the commit): delete `SystemInitialOK=true;` in W906_FRWBoot_SystemInitialOK ->
//    [3] and [4] fail; delete the call from tools/wb_serve.cpp -> [4] fails.
// =============================================================================
#include "ckernel.h"                 // ScanPannelKey
#include "MachineDefine.h"
#include "MachineType.h"             // MAX_SENSOR_ITEM, eISABase
#include "cmydef.h"                  // SystemInitialOK, bHandlerModel, Sn*/Sw* ids, TYPE_A/TYPE_B, bAse*, bLamp*
#include "mysensor.h"                // Sen[]
#include "myswitch.h"                // SW[]
#include "csystem.h"                 // IsSafeLockCheck
#include "Config.h"                  // IniConfig.bDisibleResetButton
#include "w906_ctest_guard.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>

void W906_FRWBoot_SystemInitialOK();   // FileRW/MainBoot.cpp (this target compiles it as a source)
void DoPanelLamp();                    // ckernel.cpp:2815 (golden ckernel.cpp:1728; not in ckernel.h, same as golden)

#ifndef W906_SRC_ROOT
#error "W906_SRC_ROOT must be defined by tests/CMakeLists.txt"
#endif

static int g_pass = 0;
static int g_fail = 0;
#define CHECK(cond, msg)                                                                    \
    do {                                                                                    \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                             \
        else      { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; }        \
    } while (0)

// ---- sim sensor seam: verbatim from tests/test_w7_l2_ckernel.cpp (TYPE_B reads ON, TYPE_A reads OFF offline) ----
static void simSensorOff(int idx)       { Sen[idx].Enable = true;  Sen[idx].Type = TYPE_A; Sen[idx].ISABase = eISABase; }
static void simSensorOn(int idx)        { Sen[idx].Enable = true;  Sen[idx].Type = TYPE_B; Sen[idx].ISABase = eISABase; }
static void simSensorUnknown(int idx)   { Sen[idx].Enable = false; }

static void allKeysUnknown()
{
    for (int i = 0; i < 32; ++i) simSensorUnknown(i);   // every front/rear key id + SnRKCoverOpen (27)
    simSensorUnknown(SnRearPadActive);
}
static void clearAseFlags()
{
    bAseReset = false; bAsePause = false; bAseHome = false; bAseStart = false; bAseOneCycle = false;
    bAseRetry = false; bAseSKIP = false; bAseCleanOut = false; bAseTrayFeed = false; bAseTrayEnd = false;
    bAseAlarmReset = false;
}

// ---- source helpers ------------------------------------------------------------------------------------------
static std::string ReadSource(const char* rel)
{
    std::ifstream f((std::string(W906_SRC_ROOT) + "/" + rel).c_str(), std::ios::binary);
    std::stringstream ss; ss << f.rdbuf();
    return ss.str();
}
// Blank out comments and string/char literals (newlines kept), so a count sees code only.
static std::string CodeOnly(const std::string& s)
{
    std::string o(s);
    enum { CODE, LINE, BLOCK, STR, CHR } st = CODE;
    for (size_t i = 0; i < o.size(); ++i) {
        const char c = s[i];
        const char n = (i + 1 < s.size()) ? s[i + 1] : '\0';
        switch (st) {
        case CODE:
            if (c == '/' && n == '/') { st = LINE; o[i] = ' '; }
            else if (c == '/' && n == '*') { st = BLOCK; o[i] = ' '; o[i + 1] = ' '; ++i; }
            else if (c == '"') { st = STR; }
            else if (c == '\'') { st = CHR; }
            break;
        case LINE:
            if (c == '\n') st = CODE; else if (c != '\r') o[i] = ' ';
            break;
        case BLOCK:
            if (c == '*' && n == '/') { st = CODE; o[i] = ' '; o[i + 1] = ' '; ++i; }
            else if (c != '\n' && c != '\r') o[i] = ' ';
            break;
        case STR:
        case CHR:
            if (c == '\\' && i + 1 < s.size()) { o[i] = ' '; if (s[i + 1] != '\n') o[i + 1] = ' '; ++i; }
            else if ((st == STR && c == '"') || (st == CHR && c == '\'')) st = CODE;
            else if (c != '\n' && c != '\r') o[i] = ' ';
            break;
        }
    }
    return o;
}
static int CountOf(const std::string& hay, const std::string& needle)
{
    int n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) ++n;
    return n;
}
// Count `SystemInitialOK <spaces> = <spaces> value` assignments (not `==`) in code-only text.
static int CountAssign(const std::string& code, const char* value)
{
    const std::string id = "SystemInitialOK";
    int n = 0;
    for (size_t p = code.find(id); p != std::string::npos; p = code.find(id, p + id.size())) {
        if (p > 0) {
            const char b = code[p - 1];
            if (b == '_' || (b >= '0' && b <= '9') || (b >= 'A' && b <= 'Z') || (b >= 'a' && b <= 'z')) continue;
        }
        size_t q = p + id.size();
        while (q < code.size() && (code[q] == ' ' || code[q] == '\t')) ++q;
        if (q >= code.size() || code[q] != '=' || (q + 1 < code.size() && code[q + 1] == '=')) continue;
        ++q;
        while (q < code.size() && (code[q] == ' ' || code[q] == '\t')) ++q;
        if (code.compare(q, std::string(value).size(), value) == 0) ++n;
    }
    return n;
}

int main()
{
    if (!W906TestRequireCtestRedirects("SysinitBoot"))
        return 2;
    std::printf("=== test_sysinit_boot  (golden TfMain::FormShow main.cpp:9659 SystemInitialOK=true) ===\n");

    // Fixture: every ScanPannelKey guard except SystemInitialOK open; front pad selected; front Pause held.
    const bool svHandlerModel = bHandlerModel;
    bEnableEmployeeIDCheck        = false;          // golden ckernel.cpp:1932-1933 open
    iControlPanelMode             = 0;              // IsSafeLockCheck's IO arm (golden csystem.cpp:16466)
    IniConfig.bDisibleResetButton = false;
    allKeysUnknown();                               // SnRKCoverOpen (27) unwired -> safe lock released
    clearAseFlags();
    simSensorOff(SnRearPadActive);                  // IsOff()==true -> bFrontPadActive becomes true at golden :1927
    simSensorOn(SnFKPause);                         // the held key
    SW[SwRKManualStep].Enable = false;              // lamp probe: OutValue only, no IO write
    SW[SwRKManualStep].OutValue = false;
    bLampManualSetp = true;

    // =============================================================================================================
    std::printf("\n-- [1] before boot --\n");
    CHECK(SystemInitialOK == false, "[1a] SystemInitialOK==false before the boot step (cmydef.cpp:332 initialiser)");
    CHECK(IsSafeLockCheck() == false && bEnableEmployeeIDCheck == false,
          "[1b] precondition: the other two ScanPannelKey guards are open (safe lock released, no employee-ID lockout)");
    bFrontPadActive = false;                        // the sensor says FRONT; the guard must return before :1927 writes it
    {
        const int r = ScanPannelKey();
        CHECK(r == -1 && bFrontPadActive == false,
              "[1c] front Pause held -> -1 and bFrontPadActive NOT assigned: the -1 is golden ckernel.cpp:1924-1925's SystemInitialOK return (before the pad select at :1927)");
    }
    DoPanelLamp();
    CHECK(SW[SwRKManualStep].OutValue == false,
          "[1d] DoPanelLamp writes nothing before boot (golden ckernel.cpp:1730-1731 guard): SwRKManualStep stays off although bLampManualSetp==true");

    // =============================================================================================================
    std::printf("\n-- [2] model-read error --\n");
    bHandlerModel = false;
    W906_FRWBoot_SystemInitialOK();
    CHECK(SystemInitialOK == false,
          "[2a] bHandlerModel==false -> the boot step leaves SystemInitialOK false (golden FormShow :9162-9167 terminates before :9659)");
    CHECK(ScanPannelKey() == -1 && bFrontPadActive == false, "[2b] ...and the panel key is still refused at the first guard");

    // =============================================================================================================
    std::printf("\n-- [3] after boot --\n");
    bHandlerModel = true;
    W906_FRWBoot_SystemInitialOK();
    CHECK(SystemInitialOK == true, "[3a] the boot step sets SystemInitialOK=true (golden main.cpp:9659)");
    {
        const int r = ScanPannelKey();
        CHECK(r == SnFKPause && SnFKPause == 3 && bFrontPadActive == true,
              "[3b] the SAME held front Pause is now answered: ScanPannelKey()==SnFKPause (3) and the pad select ran (golden :1927, :1958-1962)");
    }
    DoPanelLamp();
    CHECK(SW[SwRKManualStep].OutValue == true,
          "[3c] DoPanelLamp now runs: SwRKManualStep follows bLampManualSetp (golden ckernel.cpp:1895)");
    W906_FRWBoot_SystemInitialOK();
    CHECK(SystemInitialOK == true, "[3d] a second call keeps it true (golden: one write, never cleared)");

    // =============================================================================================================
    std::printf("\n-- [4] source pins --\n");
    {
        const std::string ws = ReadSource("tools/wb_serve.cpp");
        const std::string mb = ReadSource("FileRW/MainBoot.cpp");
        CHECK(!ws.empty() && !mb.empty(), "[4a] read tools/wb_serve.cpp and FileRW/MainBoot.cpp");

        const std::string kCall = "{ extern void W906_FRWBoot_SystemInitialOK(); W906_FRWBoot_SystemInitialOK(); }";
        const std::string wsCode = CodeOnly(ws);
        CHECK(CountOf(ws, kCall) == 1 && CountOf(wsCode, "W906_FRWBoot_SystemInitialOK") == 2,
              "[4b] wb_serve.cpp calls W906_FRWBoot_SystemInitialOK exactly once (one extern + one call in code)");

        const size_t pRead = ws.find("W906_DoReadLastData(true, binSelLoaded, tempLoaded);");
        const size_t pLot  = ws.find("{ extern void W906_FRWBoot_ShowLotInfoDownloadFlag(); W906_FRWBoot_ShowLotInfoDownloadFlag(); }");
        const size_t pSys  = ws.find(kCall);
        const size_t pCat  = ws.find("{ extern void W906_BootTestCategory(); W906_BootTestCategory(); }");
        const size_t pDB   = ws.find("{ MyDBUpdateDB(); }");
        const size_t pPump = ws.find("ht9045::PumpInit(whyNotPump)");
        const size_t npos  = std::string::npos;
        const bool found = pRead != npos && pLot != npos && pSys != npos && pCat != npos && pDB != npos && pPump != npos;
        CHECK(found && pRead < pLot && pLot < pSys && pSys < pCat && pCat < pDB && pDB < pPump,
              "[4c] boot order = golden: DoReadLastData(true) (906 :9562) < ShowLotInfoDownloadFlag (:9610-9615) < SYSINIT (:9659) < BootTestCategory (V912 :10594) < MyDBUpdateDB (906 :10222) < PumpInit/InitialOK (:10464)");
        CHECK(found && ws.find('\n', pLot) > pCat && ws.find("*/", pLot) < pSys,
              "[4d] the call sits on the ShowLotInfoDownloadFlag line, after that step's comment and before BootTestCategory (same line: no line below moved)");

        const std::string mbCode = CodeOnly(mb);
        CHECK(CountAssign(mbCode, "true") == 1,
              "[4e] FileRW/MainBoot.cpp code has exactly one SystemInitialOK=true (the translated golden :9659)");
        CHECK(CountAssign(mbCode, "false") == 0 && CountAssign(wsCode, "false") == 0 && CountAssign(wsCode, "true") == 0,
              "[4f] neither wb_serve.cpp nor MainBoot.cpp code assigns SystemInitialOK=false (golden never clears it) and wb_serve.cpp has no second writer");
    }

    bHandlerModel = svHandlerModel;
    std::printf("\n=== SysinitBoot: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
