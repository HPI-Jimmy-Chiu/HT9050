// =============================================================================
//  tests/test_tester_connect_rules.cpp -- golden TfMain::ChangeTesterConnect D1 / D2 / D3 / D6 / D7 (GB P2d TODOs) as
//  TesterComm/Handler/HandlerTesterConnect.cpp hook bodies.  AI(W906-D1D7) 20260928 (St02-E).
//  Suite name (add_test): TesterConnect_Rules.  Golden line numbers are 906_0625_Steven main.cpp.
//
//    0. containment first: refuses to run (exit 1, nothing called) unless the cMyDB log roots are the ctest sandbox
//       (as test_mydb_p4_containment step 0) -- D3 / D6 write NewRecordProcess rows, D7 a RecordProcess row.
//    1. the seats (LogObjects.cpp EOF) are 0 before W906_TesterConnectRulesInstall() and hold the five bodies after it.
//    2. D1 :12072-12074 -- the golden truth table, called through the seat.
//    3. D2 :12076-12087 -- sim build (SOFT_SIMULTE): never refuses.  Ship build: an IC/tray under the machine (MOT[MMTrayZ].fHasTray)
//       refuses; MES1646 only for (Mode 10, Msg true); no refusal in one-cycle mode (bOneCycleOperateChangeON_line);
//       with nothing in the machine the answer equals golden's own HasICUnderMachine() || HasAnyICInMachine().
//    4. D3 :12093-12097 -- I27 off: false, nothing changes; I27 on, not yet in manual sort: true, bRunManualSortMode, MES2156;
//       I27 on, already in manual sort: false.
//    5. D6 :12144-12152 -- LastSet.iTester -> _2D_SORT, bRunManualSortMode false, MES2155 "Change to 2D_SORT".
//       CosFunction.bOffLineBin is held false: that arm (fBinSel->ReadFile(true, ...)) deletes a recipe file.
//    6. D7 :12212-12237 -- single site: iRunStartMode / cbRunStartMode->Text by iAutoSiteMapRunStartMode, plus 912's
//       "Silent run mode change" RecordProcess; multi-site VTEST without bAutoSiteMappingOpenSite: nothing changes.
//       The two SetMainRunStartMode(rsmAutoSiteMap) arms are NOT called: SetRunStartMode writes machine files.
//  "Logged" is read from cMyDB.cpp's ExString (NewRecordProcess / RecordProcess store their message there).
//  NOT here: the call sites in forms/fMain.cpp TfMain::ChangeTesterConnect (their own commit; the path through them
//  runs ModifyTester / SaveTestMode / CloseGpibProgram).  Every global the test touches is restored.
// =============================================================================
#include "TesterComm/Handler/HandlerTesterConnect.h"
#include "LogObjects.h"            // the seats
#include "MachineType.h"           // SOFT_SIMULTE
#include "cmydef.h"
#include "cprod.h"                 // LevelSet, TestIF_File, TestMode
#include "LastSet.h"
#include "Config.h"
#include "CosFunction.h"
#include "common.h"                // as9045LogPath, asSaveEventLogPath, asProductionLogPath, sProductionInfoFilePath
#include "canary_support.h"        // W906_ShowErrorMessage_Count / _LastCode
#include "Motor/mymotor.h"         // MOT[]
#include "csystem.h"               // HasICUnderMachine / HasAnyICInMachine
#include "forms/fMain.h"
#include "cAuthority.h"            // authMainForm[7]  AI(W906-GB-P2e-EN) 20260928 (St02-E)
#include "JsonBridge/actions/MainTesterConnect.h"   // act.main.testerConnect  AI(W906-GB-P2e-EN) 20260928 (St02-E)

#include <cstdio>
#include <cstring>
#include <string>

extern AnsiString ExString;   // cMyDB.cpp:1855 (cMyDB.h:143; not included: it redeclares canary_support.h's RecordProcess default)

static int g_fail = 0, g_total = 0;
static void check(bool ok, const char* what, int line)
{
    ++g_total;
    if (!ok) { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", what, line); }
    else     { std::printf("  PASS: %s\n", what); }
}
#define CHECK(c) check((c), #c, __LINE__)

static bool Has(const char* s, const char* sub) { return std::strstr(s, sub) != 0; }
static std::string Str(const AnsiString& a) { return std::string(a.c_str()); }

// D1 through the seat with the inputs golden reads
static bool D1(int level, int need8, bool i40, int tester, bool oneCycle, bool bRemote)
{
    AccessLevel = level;
    LevelSet.AccessLevel[8] = need8;
    IniConfig.bI40_bStartProductOnLine = i40;
    LastSet.iTester = tester;
    bOneCycleOperateChangeON_line = oneCycle;
    return W906_CtcD1AccessHook(bRemote);
}

int main()
{
    std::printf("TesterConnect_Rules\n");

    // ---- 0. containment first --------------------------------------------------------------------------------
    const AnsiString* const roots[] = { &as9045LogPath, &asSaveEventLogPath, &asProductionLogPath, &sProductionInfoFilePath };
    const char* const rootNames[] = { "as9045LogPath", "asSaveEventLogPath", "asProductionLogPath", "sProductionInfoFilePath" };
    bool sandboxed = true;
    for (int i = 0; i < 4; ++i)
    {
        std::printf("  %s = %s\n", rootNames[i], roots[i]->c_str());
        if (!Has(roots[i]->c_str(), "machine_log_scratch"))
            sandboxed = false;
    }
    CHECK(sandboxed);
    if (!sandboxed)
    {
        std::printf("FAIL: not sandboxed -- refusing to call the D3 / D6 / D7 bodies (they write log rows)\n");
        return 1;
    }

    // what the test changes; restored at the end
    const int savedLevel = AccessLevel, savedNeed8 = LevelSet.AccessLevel[8], savedTester = LastSet.iTester;
    const int savedRunStart = LastSet.iRunStartMode, savedTestConn = TestMode.iTestConnection;
    const int savedTestMode = TestIF_File.iTestMode, savedAsmStart = iAutoSiteMapRunStartMode;
    const int savedVtest = IniConfig.bVTESTFunction;
    const bool savedI40 = IniConfig.bI40_bStartProductOnLine, savedI27 = IniConfig.bI27_ManualSortMode;
    const bool savedOneCycle = bOneCycleOperateChangeON_line, savedManual = bRunManualSortMode;
    const bool savedOffLineBin = CosFunction.bOffLineBin, savedOpenSite = TestIF_File.bAutoSiteMappingOpenSite;
    const bool savedTrayZ = MOT[MMTrayZ].fHasTray, savedSpil = IniConfig.bSPILFunction;
    const AnsiString savedRunText = fMain->cbRunStartMode->Text;
    IniConfig.bSPILFunction = false;

    // ---- 1. the seats ------------------------------------------------------------------------------------------
    CHECK(W906_CtcD1AccessHook == 0 && W906_CtcD2IcRefuseHook == 0 && W906_CtcD3ManualSortHook == 0 &&
          W906_CtcD6To2DSortHook == 0 && W906_CtcD7AsmOnLineHook == 0);
    W906_TesterConnectRulesInstall();
    CHECK(W906_CtcD1AccessHook == &W906_CtcD1Access);
    CHECK(W906_CtcD2IcRefuseHook == &W906_CtcD2IcRefuse);
    CHECK(W906_CtcD3ManualSortHook == &W906_CtcD3ManualSort);
    CHECK(W906_CtcD6To2DSortHook == &W906_CtcD6To2DSort);
    CHECK(W906_CtcD7AsmOnLineHook == &W906_CtcD7AsmOnLine);
    CHECK(W906_WebLoginForceOperatorHook == 0);          // D4 is WebLogin.cpp (wb_serve), not linked here
    if (W906_CtcD1AccessHook == 0 || W906_CtcD2IcRefuseHook == 0 || W906_CtcD3ManualSortHook == 0 ||
        W906_CtcD6To2DSortHook == 0 || W906_CtcD7AsmOnLineHook == 0)
    {
        std::printf("FAIL: a seat is empty after the installer\n");
        return 1;
    }

    // ---- 2. D1 -------------------------------------------------------------------------------------------------
    std::printf("  -- D1 (iDefEngineerLevel=%d)\n", iDefEngineerLevel);
    CHECK(D1(1, 1, false, OFF_LINE, false, false) == true);     // AccessLevel >= LevelSet.AccessLevel[8]
    CHECK(D1(0, 1, false, OFF_LINE, false, false) == false);    // Operator, [8]=Engineer, no I40
    CHECK(D1(0, 1, false, ON_LINE,  false, true)  == true);     // bRemote (SECS)
    CHECK(D1(0, 1, true,  OFF_LINE, false, false) == true);     // I40: an Operator may go Off-Line -> On-Line
    CHECK(D1(0, 1, true,  ON_LINE,  false, false) == false);    // ... but not On-Line -> Off-Line
    CHECK(D1(1, 2, true,  OFF_LINE, false, false) == false);    // I40 only covers AccessLevel < iDefEngineerLevel ...
    CHECK(D1(1, 2, true,  OFF_LINE, true,  false) == true);     // ... or the one-cycle Off-Line operation

    // ---- 3. D2 -------------------------------------------------------------------------------------------------
    std::printf("  -- D2\n");
    LastSet.iTester = OFF_LINE;
    bOneCycleOperateChangeON_line = false;
    MOT[MMTrayZ].fHasTray = false;
    const bool emptyMachine = !(HasICUnderMachine() || HasAnyICInMachine());
    std::printf("    nothing added: HasICUnderMachine||HasAnyICInMachine = %d\n", emptyMachine ? 0 : 1);
    MOT[MMTrayZ].fHasTray = true;
    CHECK(HasICUnderMachine());
    int n0 = W906_ShowErrorMessage_Count;
    const bool rButton = W906_CtcD2IcRefuseHook(10, true);
    const int nButton = W906_ShowErrorMessage_Count - n0;
    const std::string codeButton = Str(W906_ShowErrorMessage_LastCode);
    n0 = W906_ShowErrorMessage_Count;
    const bool rRemote = W906_CtcD2IcRefuseHook(ON_LINE, false);
    const int nRemote = W906_ShowErrorMessage_Count - n0;
    bOneCycleOperateChangeON_line = true;
    const bool rOneCycle = W906_CtcD2IcRefuseHook(10, true);
    bOneCycleOperateChangeON_line = false;
    MOT[MMTrayZ].fHasTray = false;
    const bool rEmpty = W906_CtcD2IcRefuseHook(10, true);
#ifdef SOFT_SIMULTE
    std::printf("    SOFT_SIMULTE build: golden's #ifndef SOFT_SIMULTE block is compiled out (last message '%s')\n",
                codeButton.c_str());
    CHECK(rButton == false && rRemote == false && rOneCycle == false && rEmpty == false);
    CHECK(nButton == 0 && nRemote == 0);
#else
    std::printf("    shipping build: button refused=%d (messages %d, last %s), remote refused=%d (messages %d)\n",
                rButton ? 1 : 0, nButton, codeButton.c_str(), rRemote ? 1 : 0, nRemote);
    CHECK(rButton == true && nButton == 1 && codeButton == "MES1646");
    CHECK(rRemote == true && nRemote == 0);
    CHECK(rOneCycle == false);
    CHECK(rEmpty == !emptyMachine);
#endif

    // ---- 4. D3 -------------------------------------------------------------------------------------------------
    std::printf("  -- D3\n");
    IniConfig.bI27_ManualSortMode = false;
    bRunManualSortMode = false;
    ExString = "";
    CHECK(W906_CtcD3ManualSortHook() == false && bRunManualSortMode == false && Str(ExString).empty());
    IniConfig.bI27_ManualSortMode = true;
    CHECK(W906_CtcD3ManualSortHook() == true && bRunManualSortMode == true);
    CHECK(Str(ExString) == "Change To Manual Sort Mode by iTester Button");
    ExString = "";
    CHECK(W906_CtcD3ManualSortHook() == false && bRunManualSortMode == true && Str(ExString).empty());

    // ---- 5. D6 -------------------------------------------------------------------------------------------------
    std::printf("  -- D6\n");
    CosFunction.bOffLineBin = false;
    LastSet.iTester = ON_LINE;
    bRunManualSortMode = true;
    ExString = "";
    W906_CtcD6To2DSortHook();
    std::printf("    LastSet.iTester=%d TestMode.iTestConnection=%d ExString=%s\n", LastSet.iTester, TestMode.iTestConnection,
                ExString.c_str());
    CHECK(LastSet.iTester == _2D_SORT);
    CHECK(TestMode.iTestConnection == _2D_SORT);        // TfMain::ModifyTester (forms/fMain.cpp)
    CHECK(bRunManualSortMode == false);
    CHECK(Str(ExString) == "Change to 2D_SORT");

    // ---- 6. D7 -------------------------------------------------------------------------------------------------
    std::printf("  -- D7\n");
    TestIF_File.iTestMode = SingleSite;
    iAutoSiteMapRunStartMode = 0;
    LastSet.iRunStartMode = rsmAutoSiteMap;
    W906_CtcD7AsmOnLineHook();
    CHECK(LastSet.iRunStartMode == rsmContinuStart);
    CHECK(Str(fMain->cbRunStartMode->Text) == Str(StartModeName[rsmContinuStart]));
    CHECK(Str(ExString) == "Silent run mode change by iTester On/Off Line : " + Str(StartModeName[rsmContinuStart]));
    iAutoSiteMapRunStartMode = 1;
    W906_CtcD7AsmOnLineHook();
    CHECK(LastSet.iRunStartMode == rsmContinuRetest);
    CHECK(Str(fMain->cbRunStartMode->Text) == Str(StartModeName[rsmContinuRetest]));
    TestIF_File.iTestMode = SingleSite + 1;              // any multi-site mode
    IniConfig.bVTESTFunction = true;
    TestIF_File.bAutoSiteMappingOpenSite = false;
    LastSet.iRunStartMode = rsmContinuStart;
    ExString = "";
    W906_CtcD7AsmOnLineHook();
    CHECK(LastSet.iRunStartMode == rsmContinuStart && Str(ExString).empty());

    // ---- 8. act.main.testerConnect: golden's enable gate imgTester->Enabled=authMainForm[7] (AI(W906-GB-P2e-EN) 20260928 (St02-E)) ---------
    //   ChangeLevelAttr main.cpp:12539 is golden's only writer of imgTester->Enabled; a disabled image is never clicked.
    //   The enabled case stops at SystemStart (golden imgTesterClick's first return), so no mode changes in either case.
    {
        const bool savedAuth7 = authMainForm[7], savedStart = SystemStart;
        const int testerBefore = LastSet.iTester;
        authMainForm[7] = false;
        const std::string off = ht9045::sjson::DoTesterConnectAction("{}");
        std::printf("  [8] authMainForm[7]=false -> %s\n", off.c_str());
        CHECK(Has(off.c_str(), "\"guard\":\"disabled\""));
        CHECK(Has(off.c_str(), "\"executed\":false"));
        CHECK(LastSet.iTester == testerBefore);
        authMainForm[7] = true;
        SystemStart = true;
        const std::string on = ht9045::sjson::DoTesterConnectAction("{}");
        std::printf("  [8] authMainForm[7]=true, SystemStart -> %s\n", on.c_str());
        CHECK(!Has(on.c_str(), "\"disabled\""));
        CHECK(LastSet.iTester == testerBefore);
        authMainForm[7] = savedAuth7;  SystemStart = savedStart;
    }

    // ---- restore -----------------------------------------------------------------------------------------------
    AccessLevel = savedLevel;  LevelSet.AccessLevel[8] = savedNeed8;  LastSet.iTester = savedTester;
    LastSet.iRunStartMode = savedRunStart;  TestMode.iTestConnection = savedTestConn;
    TestIF_File.iTestMode = savedTestMode;  iAutoSiteMapRunStartMode = savedAsmStart;  IniConfig.bVTESTFunction = savedVtest;
    IniConfig.bI40_bStartProductOnLine = savedI40;  IniConfig.bI27_ManualSortMode = savedI27;
    bOneCycleOperateChangeON_line = savedOneCycle;  bRunManualSortMode = savedManual;
    CosFunction.bOffLineBin = savedOffLineBin;  TestIF_File.bAutoSiteMappingOpenSite = savedOpenSite;
    MOT[MMTrayZ].fHasTray = savedTrayZ;  IniConfig.bSPILFunction = savedSpil;
    fMain->cbRunStartMode->Text = savedRunText;

    std::printf("%s: %d/%d checks passed\n", g_fail ? "FAIL" : "PASS", g_total - g_fail, g_total);
    return g_fail ? 1 : 0;
}
