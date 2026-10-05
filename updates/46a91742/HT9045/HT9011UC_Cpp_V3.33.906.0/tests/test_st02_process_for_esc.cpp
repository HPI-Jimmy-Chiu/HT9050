// =============================================================================
//  tests/test_st02_process_for_esc.cpp -- AI(W906-ST02-ESC) 20261005 (St02-E): census 129 E-T1-007.  Suite: St02_ProcessForESC.
//  golden 906 0618 TfMain::ProcessForESC (main.cpp:7615-7706; 912 :8040-8131 identical) = THandlerTesterSide::ProcessForESC
//  (TesterComm/Handler/HandlerGpibMsg.cpp), called from golden TfMain::Timer1Timer :2858 = MainTimersSt02.cpp Timer1BinTick through
//  ht9045::W906_ESCProcessHook (installed by TesterCommWiring.cpp W906_TesterCommInit; here by the test, the same assignment).
//  Seeded at the source: the flag is set by the REAL THandlerTesterSide::ResetForESC (golden :7603, the CHECKEMPTY path), and the
//  REAL Timer1 slice runs it (ht9045::W906_St02Timer1BinTickAt, the dispatcher's 30 ms grid).  Memory + ctest roots only
//  (containment first, st02_test_containment.h); THandlerTesterSide is not attached to the hub: no engine, nothing is sent.
//    1. the reset: an untested IC on FTestSuck / BTestSuck -> TEST_PASS + [I41] BinOfESC with a "RESET" record, a tested one
//       unchanged; bDoEmptySocketOneCycle, bResetMode, iESC_IndexContactCount 0, OutArmTask 1, bLampReset, pbtReset,
//       fMain->BtnOneCycle->Down, RunTestProgram(false) ran (bAutoSiteMapWaitTestResult cleared), 3 "Reset Record" lines, the
//       flag cleared; once only.  Panel type 0 on purpose (golden :2858 runs for every NUMBER_PANEL_TYPE).
//    2. bBin16HangUp -> CloseGpibProgram (this object's: bFind false, one more MyDBIProcess) and bBin16HangUp false.
//    3. it waits (flag kept, nothing changes) until golden's guard holds: not Qorvo; an IC in TestSocket (then gone -> runs);
//       IsTest; InitialOK false (golden :2719 returns first); an S10F3 alarm does NOT stop it (golden :2858 before :2938).
//    4. the destroys, golden order with the && short-circuit kept: InArm busy -> OutArm's empty Destroy site is not looked at
//       (its event stays 2); InArm done -> OutArm looked at: CheckDestoryFinish clears it (golden MyKitSuck.cpp:2870-2871) and
//       still says not finished; the next pass runs.
//    5. ResetForESC: not Qorvo, or bDoEmptySocketOneCycle already set -> no flag (golden :7605-7611).
//    6. golden bRunTimer1: a Timer1 pass from inside ProcessForESC (a wait in RunTestProgram) does not re-enter it.
//    7. no TesterComm (hook 0, golden: no fTesterSide here) -> nothing, no crash.
//    8. the hook-up lines are code, not comment (argv[1] = the tree root).
//    9. under a box (St02-M 05:4x): the S-27 in-box slice (W906_ModalTimer1Segments) never calls ESC, and a box-wait pass
//       composed as the tree wires it (before !174: the St02 modal tick; after !174: + the fast clock's binpanel job) calls it
//       at most once -- golden runs ProcessForESC once per Timer1 call.
//  CONTROL: W906_ESC_CONTROL=1 leaves the hook uninstalled (= main before this change: nothing reads bTriggerESC) -> section 1
//    must fail (exit 1).  St01 runs it once that way to prove the test can go red.
// =============================================================================
#include "MachineType.h"
#include "cmydef.h"
#include "Config.h"
#include "cprod.h"
#include "LastSet.h"
#include "aHotPlateSubstrate.h"
#include "canary_support.h"
#include "csystem.h"
#include "cpublic.h"           // GetTimeInfo
#include "forms/fMain.h"
#include "Public/MyProductionRecord.h"
#include "TesterComm/Handler/HandlerTesterSide.h"
#include "st02_test_containment.h"

#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

extern int OutArmTask;                                 // aoutarm.h:77
extern int        W906_MyDBIProcess_Count;             // aHotPlateSubstrate.cpp:1241-1244 (the 2-arg MyDBIProcess adapter)
extern AnsiString W906_MyDBIProcess_LastS1;
extern AnsiString W906_MyDBIProcess_LastS2;
void W906_MyDBIProcess_Reset();
namespace ht9045 {
void W906_St02TimersReset();
void W906_St02Timer1BinTickAt(unsigned long now);
void W906_St02Timer1BinFastAt(unsigned long now);     // MainTimersSt02.cpp: the fast clock's binpanel job (section 9)
void W906_St02TimersTickModalAt(unsigned long now);   // MainTimersSt02.cpp: one box-wait pass on the test clock (section 9)
void W906_FastClockSetOwnsBin(bool);                  // FastClockJobs.cpp
}
void W906_ModalTimer1Segments();                      // MainTimersSt02.cpp (S-27)
void EscTest_Nozzle(int which, int item, int ev);     // test_st02_process_for_esc_nozzle.cpp (the full TMyKitSuck)
int  EscTest_NozzleEvent(int which);
void EscTest_NozzleClear();

static int g_total = 0, g_fail = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL (line %d): %s\n", line, what); }
    else     std::printf("  ok: %s\n", what);
}
#define CHECK(c) check((c), #c, __LINE__)

static const int kBin = 7;                             // [I41] BinOfESC for the test
static bool g_control = false;
static int g_n1 = 0;                                   // MyDBIProcess calls of one reset (section 1)
static unsigned long g_t = 100000;                     // the test clock

static void ClearIndexGrids()
{
    for (int i = 0; i < MAX_Index_Row; i++)
        for (int j = 0; j < NEW_MAX_Index_Col; j++)
        {
            FTestSuck.Item[i][j]  = NULL_IC;
            BTestSuck.Item[i][j]  = NULL_IC;
            TestSocket.Item[i][j] = NULL_IC;
            FTestSuck.PordRec[i][j].InitialRecord();
            BTestSuck.PordRec[i][j].InitialRecord();
        }
}

static bool HasReset(TMyKitSuck& k, int i, int j)
{
    return std::string(k.PordRec[i][j].asBuffer->GetString(eErrorCode).c_str()).find("RESET") != std::string::npos;
}

static void Install()
{
    ht9045::W906_ESCProcessHook = g_control ? 0 : &THandlerTesterSide::ProcessForESCHook;   // TesterCommWiring.cpp's assignment
}

// every guard open, no flag, the Index grids empty, a fresh Timer1 grid
static void Base()
{
    ht9045::W906_St02TimersReset();
    Install();
    InitialOK = true;
    bSystemClose = false;
    bSECSGEMAlarm = false;
    bSECSGEM_NoteAlarm = false;
    NUMBER_PANEL_TYPE = 0;
    TestIF_File.iGpibMode = InterfaceType_15BinQorvo;
    IniConfig.iI41_BinOfESC = kBin;
    bDoEmptySocketOneCycle = false;
    fTesterSide->bTriggerESC = false;
    fTesterSide->bFind = true;                         // RunTestProgram / CloseGpibProgram run their bodies (no hub: nothing is sent)
    IsTest = false;
    bResetMode = false;
    iESC_IndexContactCount = 5;
    OutArmTask = 0;
    bLampReset = false;
    iWhoTriggerPiggyBack = 0;
    bBin16HangUp = false;
    bAutoSiteMapWaitTestResult = true;
    fMain->BtnOneCycle->Down = false;
    ClearIndexGrids();
    EscTest_NozzleClear();
    W906_MyDBIProcess_Reset();
}

// four 30 ms Timer1 periods on the dispatcher's grid (golden main.dfm Interval 30)
static void Passes(int n = 4)
{
    for (int k = 0; k < n * 3; ++k) { g_t += 10; ht9045::W906_St02Timer1BinTickAt(g_t); }
}

// ---- source checks (comments stripped; #if 0 blocks skipped) -------------------------------------------------------------
static std::string ReadAll(const std::string& p)
{
    std::string s;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return s;
    char buf[4096];
    size_t n;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) s.append(buf, n);
    std::fclose(f);
    return s;
}

static std::string CodeOfLine(const std::string& l)
{
    std::string out;
    bool inStr = false, inBlock = false;
    for (size_t i = 0; i < l.size(); ++i)
    {
        const char c = l[i];
        const char n = i + 1 < l.size() ? l[i + 1] : '\0';
        if (inBlock) { if (c == '*' && n == '/') { inBlock = false; ++i; } continue; }
        if (inStr) { out += c; if (c == '\\' && n) { out += n; ++i; } else if (c == '"') inStr = false; continue; }
        if (c == '"') { inStr = true; out += c; continue; }
        if (c == '/' && n == '/') break;
        if (c == '/' && n == '*') { inBlock = true; ++i; continue; }
        out += c;
    }
    return out;
}

static std::vector<std::string> Lines(const std::string& text)
{
    std::vector<std::string> v;
    size_t p = 0;
    while (p <= text.size())
    {
        size_t e = text.find('\n', p);
        if (e == std::string::npos) e = text.size();
        std::string l = text.substr(p, e - p);
        if (!l.empty() && l[l.size() - 1] == '\r') l.erase(l.size() - 1);
        v.push_back(l);
        p = e + 1;
    }
    return v;
}

static int CodeCount(const std::string& text, const std::string& needle)
{
    int n = 0;
    bool gated = false;
    const std::vector<std::string> v = Lines(text);
    for (size_t i = 0; i < v.size(); ++i)
    {
        const std::string& l = v[i];
        if (l.compare(0, 5, "#if 0") == 0) { gated = true; continue; }
        if (gated) { if (l.compare(0, 6, "#endif") == 0) gated = false; continue; }
        if (CodeOfLine(l).find(needle) != std::string::npos) ++n;
    }
    return n;
}

static size_t CodePos(const std::string& text, const std::string& needle)   // the code line index of the first hit, or npos
{
    const std::vector<std::string> v = Lines(text);
    for (size_t i = 0; i < v.size(); ++i)
        if (CodeOfLine(v[i]).find(needle) != std::string::npos) return i;
    return std::string::npos;
}

static int g_count = 0;                                // section 9: hook calls
static void CountHook() { ++g_count; }

// re-entry probe (section 6): the hook runs ProcessForESC, then a Timer1 pass from inside it
static int g_depth = 0, g_maxDepth = 0;
static void ReentryHook()
{
    ++g_depth;
    if (g_depth > g_maxDepth) g_maxDepth = g_depth;
    if (g_depth == 1)                                  // a broken guard reads depth 2 (a clean FAIL, not an endless recursion)
    {
        THandlerTesterSide::ProcessForESCHook();
        g_t += 40;
        ht9045::W906_St02Timer1BinTickAt(g_t);         // a wait inside golden RunTestProgram pumping Timer1
    }
    --g_depth;
}

int main(int argc, char** argv)
{
    std::setvbuf(stdout, NULL, _IONBF, 0);
    std::printf("St02_ProcessForESC\n");
    if (!W906TestInsideCtestRoots("St02_ProcessForESC"))
        return 2;
    const char* ctl = std::getenv("W906_ESC_CONTROL");
    g_control = ctl && ctl[0] == '1' && ctl[1] == '\0';
    if (g_control)
        std::printf("  CONTROL RUN (W906_ESC_CONTROL=1): the hook is NOT installed -- section 1 must fail\n");
    CHECK(fMain != 0 && fMain->BtnOneCycle != 0);
    if (fMain == 0 || fMain->BtnOneCycle == 0) return 1;
    CHECK(fTesterSide == 0);
    fTesterSide = new THandlerTesterSide();            // not attached to the hub: no engine
    const int savedType = NUMBER_PANEL_TYPE, savedMode = TestIF_File.iGpibMode, savedBin = IniConfig.iI41_BinOfESC;
    GetTimeInfo();

    std::printf(" 1. the reset (golden :7626-7704), once\n");
    {
        Base();
        FTestSuck.Item[0][0] = HAS_IC;                 // untested
        FTestSuck.Item[0][1] = TEST_PASS + 3;          // tested: keeps its bin
        BTestSuck.Item[1][2] = HAS_IC;
        fTesterSide->ResetForESC("Start Empty Socket OneCycle for Tester Send Check Empty");
        CHECK(fTesterSide->bTriggerESC == true);       // the real setter (golden :7609)
        Passes();
        CHECK(fTesterSide->bTriggerESC == false);
        CHECK(bDoEmptySocketOneCycle == true && bResetMode == true && iESC_IndexContactCount == 0 && IsTest == false);
        CHECK(FTestSuck.Item[0][0] == TEST_PASS + kBin && FTestSuck.iBinData[0][0] == kBin && HasReset(FTestSuck, 0, 0));
        CHECK(BTestSuck.Item[1][2] == TEST_PASS + kBin && BTestSuck.iBinData[1][2] == kBin && HasReset(BTestSuck, 1, 2));
        CHECK(FTestSuck.Item[0][1] == TEST_PASS + 3 && !HasReset(FTestSuck, 0, 1));
        CHECK(OutArmTask == 1 && bLampReset == true && iWhoTriggerPiggyBack == pbtReset);
        CHECK(fMain->BtnOneCycle->Down == true);       // InitOneCycle("Reset For ESC") + Down (golden :7632-7636)
        CHECK(bAutoSiteMapWaitTestResult == false);    // RunTestProgram(false) ran its body (HandlerBridgeCtl.cpp, bNeedTest==false)
        CHECK(W906_MyDBIProcess_Count >= 3 && W906_MyDBIProcess_LastS1 == "Message" &&
              std::string(W906_MyDBIProcess_LastS2.c_str()).find("Reset Record: Total=") == 0);   // the last of golden's three
        g_n1 = W906_MyDBIProcess_Count;
        std::printf("  [info] MyDBIProcess calls in one reset: %d\n", g_n1);
        Passes();
        CHECK(W906_MyDBIProcess_Count == g_n1);        // once: the flag is gone
    }

    std::printf(" 2. bBin16HangUp -> CloseGpibProgram (golden :7671-7675)\n");
    {
        Base();
        bBin16HangUp = true;
        fTesterSide->ResetForESC("t2");
        Passes();
        CHECK(bBin16HangUp == false && fTesterSide->bFind == false);   // CloseGpibProgram's last line (HandlerBridgeCtl.cpp)
        CHECK(W906_MyDBIProcess_Count == g_n1 + 1);    // + CloseGpibProgram's "GPIB Close - ..." line
        CHECK(std::string(W906_MyDBIProcess_LastS2.c_str()).find("Reset Record: Total=") == 0);
    }

    std::printf(" 3. waits until golden's guard holds (flag kept, nothing changes)\n");
    {
        Base();
        fTesterSide->ResetForESC("t3a");
        TestIF_File.iGpibMode = 0;
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && bDoEmptySocketOneCycle == false);   // not Qorvo

        Base();
        TestSocket.Item[0][0] = HAS_IC;
        fTesterSide->ResetForESC("t3b");
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && bDoEmptySocketOneCycle == false);   // an IC in the socket
        TestSocket.Item[0][0] = NULL_IC;
        Passes();
        CHECK(fTesterSide->bTriggerESC == false && bDoEmptySocketOneCycle == true);   // gone -> runs

        Base();
        fTesterSide->ResetForESC("t3c");
        IsTest = true;
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && bDoEmptySocketOneCycle == false);   // under test
        IsTest = false;

        Base();
        fTesterSide->ResetForESC("t3d");
        InitialOK = false;
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && bDoEmptySocketOneCycle == false);   // golden :2719 first
        InitialOK = true;

        Base();
        bSECSGEMAlarm = true;                          // golden :2938-2959 returns AFTER :2858
        fTesterSide->ResetForESC("t3e");
        Passes();
        CHECK(fTesterSide->bTriggerESC == false && bDoEmptySocketOneCycle == true);
        bSECSGEMAlarm = false;
    }

    std::printf(" 4. the destroys: golden order, short-circuit kept (HandlerEscSuck.cpp)\n");
    {
        Base();
        EscTest_Nozzle(0, HAS_IC, 2);                  // InArm: destroying an IC -> not finished
        EscTest_Nozzle(1, NULL_IC, 2);                 // OutArm: a Destroy event on an empty site
        fTesterSide->ResetForESC("t4");
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && EscTest_NozzleEvent(1) == 2);   // OutArm never looked at
        EscTest_Nozzle(0, HAS_IC, 0);                  // InArm done
        g_t += 10; ht9045::W906_St02Timer1BinTickAt(g_t);
        g_t += 10; ht9045::W906_St02Timer1BinTickAt(g_t);
        g_t += 10; ht9045::W906_St02Timer1BinTickAt(g_t);   // one 30 ms period
        CHECK(fTesterSide->bTriggerESC == true && EscTest_NozzleEvent(1) == 0);   // looked at: cleared, still "not finished"
        Passes();
        CHECK(fTesterSide->bTriggerESC == false && bDoEmptySocketOneCycle == true);
        EscTest_Nozzle(0, NULL_IC, 0);
    }

    std::printf(" 5. ResetForESC (golden :7605-7611)\n");
    {
        Base();
        TestIF_File.iGpibMode = 0;
        fTesterSide->ResetForESC("t5a");
        CHECK(fTesterSide->bTriggerESC == false);
        Base();
        bDoEmptySocketOneCycle = true;
        fTesterSide->ResetForESC("t5b");
        CHECK(fTesterSide->bTriggerESC == false);
    }

    std::printf(" 6. golden bRunTimer1: no re-entry from inside ProcessForESC\n");
    if (!g_control)
    {
        Base();
        ht9045::W906_ESCProcessHook = &ReentryHook;
        g_depth = g_maxDepth = 0;
        fTesterSide->ResetForESC("t6");
        Passes(2);
        CHECK(g_maxDepth == 1);
        Install();
    }
    else
        std::printf("  (skipped in the control run)\n");

    std::printf(" 7. no TesterComm -> nothing\n");
    {
        Base();
        ht9045::W906_ESCProcessHook = 0;
        FTestSuck.Item[0][0] = HAS_IC;
        fTesterSide->ResetForESC("t7");
        Passes();
        CHECK(fTesterSide->bTriggerESC == true && FTestSuck.Item[0][0] == HAS_IC && bDoEmptySocketOneCycle == false);
        Install();
    }

    std::printf(" 8. the hook-up lines are code (argv[1] = the tree root, read only)\n");
    if (argc > 1)
    {
        const std::string root = argv[1];
        const std::string ms = ReadAll(root + "/MainTimersSt02.cpp"), cw = ReadAll(root + "/TesterComm/Handler/TesterCommWiring.cpp");
        const std::string gm = ReadAll(root + "/TesterComm/Handler/HandlerGpibMsg.cpp"), cm = ReadAll(root + "/CMakeLists.txt");
        CHECK(!ms.empty() && !cw.empty() && !gm.empty() && !cm.empty());
        CHECK(CodeCount(ms, "W906_ESCProcessHook();") == 1);
        CHECK(CodeCount(ms, "void (*W906_ESCProcessHook)() = 0;") == 1);
        const size_t pEsc = CodePos(ms, "W906_ESCProcessHook();"), pType = CodePos(ms, "if (NUMBER_PANEL_TYPE != 2)");
        const size_t pInit = CodePos(ms, "if (InitialOK == false)");
        CHECK(pEsc != std::string::npos && pType != std::string::npos && pInit != std::string::npos && pInit < pEsc && pEsc < pType);
        CHECK(CodeCount(cw, "ht9045::W906_ESCProcessHook = &THandlerTesterSide::ProcessForESCHook;") == 1);
        CHECK(CodeCount(gm, "void THandlerTesterSide::ProcessForESC()") == 1 && CodeCount(gm, "W906_EscDestroysFinished())") == 1);
        CHECK(CodeCount(cm, "TesterComm/Handler/HandlerEscSuck.cpp") == 1);
    }
    else
        std::printf("  (skipped: no tree root given)\n");

    std::printf(" 9. under a box: one ESC per Timer1 period, none from the S-27 in-box slice (St02-M 05:4x)\n");
    if (!g_control && argc > 1)
    {
        // A box wait in wb_serve (W906_ModalWaitTick, tools/wb_serve.cpp) runs W906_St02TimersTickFromModal first and
        // W906_ModalTimer1Segments (S-27) later.  Before !174 the St02 modal tick runs the Timer1 slice (St02PassAt's
        // g_modalDepth branch); after !174 the waits are W906_FastClockModalWait and the fast clock's binpanel job runs it.
        // The pass below is composed from the tree's own wiring, so it is right before and after !174.  The fast clock owns
        // the bin slice in wb_serve (FastClockWbServe.cpp) -- so here too.
        const std::string root = argv[1];
        const bool fastInBox = CodeCount(ReadAll(root + "/tools/wb_serve.cpp"), "W906_FastClockModalWait(") > 0;
        std::printf("  [info] box waits run the fast clock: %s\n", fastInBox ? "yes (after !174)" : "no (before !174)");
        Base();
        ht9045::W906_FastClockSetOwnsBin(true);
        ht9045::W906_ESCProcessHook = &CountHook;
        g_count = 0;
        for (int k = 0; k < 5; ++k) W906_ModalTimer1Segments();
        CHECK(g_count == 0);                           // the S-27 slice never calls ESC (the second call we left out)
        const int passes = 10;
        int most = 0;
        for (int k = 0; k < passes; ++k)
        {
            g_t += 30;
            const int before = g_count;
            if (fastInBox) ht9045::W906_St02Timer1BinFastAt(g_t);   // the binpanel job inside W906_FastClockModalWait
            ht9045::W906_St02TimersTickModalAt(g_t);   // W906_ModalWaitTick's first call (W906_St02TimersTickFromModal)
            W906_ModalTimer1Segments();                // W906_ModalWaitTick :7634
            if (g_count - before > most) most = g_count - before;
        }
        std::printf("  [info] %d ESC calls in %d box passes, at most %d in one\n", g_count, passes, most);
        CHECK(most == 1 && g_count >= passes - 1);     // once per period (the first pass may only start the 30 ms grid)
        ht9045::W906_FastClockSetOwnsBin(false);
        Install();
    }
    else
        std::printf("  (skipped: %s)\n", g_control ? "the control run" : "no tree root given");

    ht9045::W906_ESCProcessHook = 0;
    NUMBER_PANEL_TYPE = savedType;
    TestIF_File.iGpibMode = savedMode;
    IniConfig.iI41_BinOfESC = savedBin;
    bDoEmptySocketOneCycle = false;
    ClearIndexGrids();
    delete fTesterSide;
    fTesterSide = 0;
    std::printf("%s: %d/%d checks passed%s\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total, g_control ? " (CONTROL run)" : "");
    return g_fail ? 1 : 0;
}
