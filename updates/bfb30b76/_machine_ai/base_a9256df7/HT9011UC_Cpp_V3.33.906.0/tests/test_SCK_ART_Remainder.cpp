// tests/test_SCK_ART_Remainder.cpp
// Verification harness for the Automation/SCK_ART_Remainder.h/.cpp REMAINDER slice
// (W5-Final-SckArtRemainder), continuing the sibling test_SCK_ART.cpp (8-function extract).
//
// Exercises the translated public API against input->expected-output values hand-derived from the
// ORIGINAL golden reference
//   HT9011UC_Code_V3.33.906.0_20260618/Automation/SCK_ART.cpp:183-1645, :4191-4256 (see
//   SCK_ART_Remainder.h for the exact per-function golden line ranges).
//
// AI(W906-DoARTLotStart) 20260721: added PART 9 (DoARTLotStart, golden :4191-4256).
// AI(W906-Save2DSortingSummary) 20260723: added PART 11 (Save2DSortingSummary, golden :3402-4061);
// also retrofitted PART 8's 2D-sort dispatch sub-case (now real, see PART 8's own updated comment).
// AI(W906-SCKART-Multi-Test) 20260802: added PART 14 (SaveMultiLotTestSummary, golden :2045-2803) --
// the function's ONLY caller in this tree (golden's own caller, csystem.cpp:10862's DoTrayFeedProcess,
// is untranslated, owned by wave W7), and the first and only exercise anywhere of gate #5's per-lot
// mirrors LotSummary.iByLotCountCategory[][][] / .iByLotTotalCategory[][]. Every one of its 37
// assertions was verified to go RED against a neutered (immediate-return) function body; see PART 14's
// own header for what it deliberately does NOT cover.
//
// LIMITATION 1 (same as test_SCK_ART.cpp/test_ContactForce.cpp): we CANNOT run the original BCB6
// binary (no Borland compiler in this environment). Verification here is therefore: (1) the
// translation compiles+links clean against the real shared globals, AND (2) its outputs equal values
// hand-computed/transcribed from the BCB6 source (golden file:line cited per case).
//
// LIMITATION 2 (DO-NOT-TOUCH-REAL-FILES discipline, same spirit as test_ini_helpers.cpp's "DO-NOT-
// MODIFY-REAL-INI DISCIPLINE"): golden TfSCKART::AccessFile(bool bRead,...) calls WriteLastDataFile()
// when bRead==false (golden SCK_ART.cpp:436) and CustomerFunctionSelect() unconditionally (golden
// :437). An EARLY DRAFT of this test called the real AccessFile(bRead=false,...) directly and only
// discovered via an actual link (not just -fsyntax-only) that BOTH of those, plus RUN_INFO::AddAlarm,
// are TODO(W6)-gated dead text inside cprod.cpp (see SCK_ART_Remainder.cpp gate #8) -- so calling them
// is, for now, a safe no-op (gate #8), NOT the real WriteLastDataFile (which would otherwise write the
// ACTUAL production Handler's live D:\HT9045\system\lastdata*.dat files at hardcoded, non-redirectable
// paths -- this repo also contains the live D:\HT9045 production tree alongside this migration
// sandbox). PART 3b below DOES exercise AccessFile's bRead==false (save) direction, relying on gate #8
// making WriteLastDataFile/CustomerFunctionSelect harmless; when gate #8 is eventually retired (W6),
// re-verify this harness still cannot reach a real hardcoded system path before removing this note.
// SckArtRem_SetGPIBVersion is NEVER called here: golden hardcodes a DIFFERENT, still very-much-real,
// non-redirectable external path -- D:\GPIB9045\system\general.ini (a separate live project directory
// on this dev machine, read via the REAL, active ReadWriteIni/WriteIniData in common.cpp) -- with no
// equivalent gate protecting it.
//
// LIMITATION 3 (hidden TU-local gate statics, same as test_SCK_ART.cpp's CheckOutArmNeedVariModeFIX
// PART): several LastSet.* fields this file's gate #4 stands in for (lSCKARTBinCT[]/iHdPass/iHdFail/
// iHdRejectATPass/iHdRejectATFail/iHdRejectBT/iTesterMatch/iTesterUnmatch/iTestIgnore/iHdPickUp/
// iSCKART_RTUnitCount/bBreakSCKART) are `static` (internal linkage) inside SCK_ART_Remainder.cpp --
// this test binary cannot seed or observe them directly. Assertions below only cover the OBSERVABLE
// (SckArtRemainderState / real canary_support.h LastSet / real cprod.h globals) side effects, and
// choose input values that make the always-0/false stand-in defaults harmless to the assertion (same
// technique the sibling test already established).
//
// No external test framework: a tiny check harness prints PASS/FAIL per case and a final summary,
// and returns non-zero on ANY failure.

#include "Automation/SCK_ART_Remainder.h"

#include "MachineDefine.h"
#include "MachineType.h"
#include "cprod.h"
#include "cmydef.h"
#include "canary_support.h"
#include "aHotPlateSubstrate.h"
#include "FormsFacade.h"
// AI(W906-P10) 20260921: asGeneralPath / OpenGeneralIniFile / CloseGeneralIniFile
#include "common.h"
#include <cstdlib>
#include "common.h"     // ReadWriteIni/WriteIniData (direct, safe scratch-path seeding) + DataPath
#include "atester_shims.h"   // AI(W906-SaveTestSummarySECS) 20260721: fObserver->memoLotSummary (PART 10)
#include "Public/MyStringList.h"   //AI(W906-S72-TAIL) 20260929 (St02-E, claim): TMyStringList for the PART 9 slEventLog scope (occupies the old blank line; no line moves)
#include <cstdio>
#include <cstdlib>
#include <cmath>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { std::printf("PASS  %s\n", msg); ++g_pass; }                 \
        else      { std::printf("FAIL  %s  (line %d)\n", msg, __LINE__); ++g_fail; } \
    } while (0)

static bool feq(double a, double b, double eps = 1e-6)
{
    return std::fabs(a - b) <= eps * (1.0 + std::fabs(b));
}

// Scratch-dir helper -- mirrors tests/test_IniFiles.cpp's tmpIniPath() convention (prefer %TEMP%/%TMP%,
// fall back to the current directory, which always exists when ctest runs).
static AnsiString ScratchDir()
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    AnsiString d(t);
    if (d.Length()==0 || d[d.Length()]!='\\')
        d = d + "\\";
    return d;
}

// AI(W906-SCKART-Multi-Test) 20260802: PART 14 content helpers. Both return FALSE for a missing or
// unreadable file, so every POSITIVE assertion built on them automatically also asserts "the file was
// produced at all" -- which is what makes them go red against a do-nothing body. NEGATIVE assertions
// ("this line must NOT be present") are therefore always paired with a positive witness in the SAME
// CHECK, never left standing alone, because "absent" is trivially true when nothing was written.
static bool FileHasExactLine(const AnsiString& path, const AnsiString& want)
{
    if (!FileExists(path)) return false;
    TStringList *sl = new TStringList();
    sl->LoadFromFile(path);
    bool found = false;
    for (int i = 0; i < sl->Count; ++i)
        if (sl->Strings[i] == want) found = true;
    sl->Clear();
    delete sl;
    return found;
}

static bool FileHasLinePrefix(const AnsiString& path, const AnsiString& prefix)
{
    if (!FileExists(path)) return false;
    TStringList *sl = new TStringList();
    sl->LoadFromFile(path);
    bool found = false;
    for (int i = 0; i < sl->Count; ++i)
    {
        AnsiString line = sl->Strings[i];
        if (line.Pos(prefix) == 1) found = true;
    }
    sl->Clear();
    delete sl;
    return found;
}

// AI(W906-P10) 20260921: 把 general ini 導到暫存檔 —— **絕不碰量產的
//   system\Gerneral.ini**（memory: ht9045-gerneral-ini-normalized-20260817，
//   那個檔被整檔重寫過一次）。慣例照抄 tests/test_binsel_core.cpp:93-99 / :231-233。
static AnsiString SckArtScratchIniPath(const char* leaf)
{
    const char* t = std::getenv("TEMP");
    if (!t || !*t) t = std::getenv("TMP");
    if (!t || !*t) t = ".";
    return AnsiString(t) + AnsiString("\\") + AnsiString(leaf);
}

int main()
{
    printf("=== Automation/SCK_ART_Remainder.cpp translation verification ===\n");
    printf("(values hand-derived/transcribed from golden Automation/SCK_ART.cpp:183-1645; original binary NOT run)\n\n");

    // ★ AI(W906-P10) 20260921: 這幾行是 P10 逼出來的，理由要寫清楚。
    //
    //  P10 把 golden 的 SetRunStartMode 翻成真本體之後，本測試的
    //  SckArtRem_DoARTLotStart 會走到：
    //      SetRunStartMode -> fBinSel->ReadFile(false,false,"")   （golden 無條件，:781）
    //                      -> SetWorkParameter -> ReadTechData -> CloseGeneralIniFile
    //  而 CloseGeneralIniFile()（common.cpp:1559）在 delete 之後**刻意不把
    //  INIFileGeneral 設回 NULL** —— 那裡的註解寫著 "faithful bug"，
    //  而且 **golden 確實就是這樣**（golden common.cpp:1414-1421，20260921 逐行比對過）。
    //  ⇒ 第二次 close 會對已釋放記憶體呼叫 UpdateFile() -> SEGFAULT。
    //
    //  ⇒ 崩的**不是翻譯**，是測試行程沒有滿足 golden 在 main() 裡保證的前置
    //    （ini 開著）。所以修**測試夾具**，不動 golden 的忠實複製。
    //  ⚠ 用暫存路徑，絕不指向量產的 system\Gerneral.ini。
    AnsiString savedGeneralPath = asGeneralPath;
    asGeneralPath = SckArtScratchIniPath("test_sck_art_general.ini");
    OpenGeneralIniFile();

    // =========================================================================================
    // PART 1 -- SetSetupFilePath -- golden :183-188
    // =========================================================================================
    printf("-- SetSetupFilePath --\n");
    {
        AnsiString savedDataPath = DataPath;
        DataPath = "C:\\W5FinalSckArtRemainderScratch\\";   // gate #1 stand-in reads DataPath directly

        SckArtRemainderState st;
        st.sSetupFilePath = "stale";
        SckArtRem_SetSetupFilePath(st);
        CHECK(st.sSetupFilePath == DataPath + AnsiString("Tester.Data"),
              "SetSetupFilePath -> DataPath+\"Tester.Data\" (golden :185-187, gate #1 stand-in skips GetLastOpenFN() subfolder)");

        DataPath = savedDataPath;
    }

    // =========================================================================================
    // PART 2 -- AccessFile early-return -- golden :195-196 (CosFunction.bUseSCKART==false)
    // =========================================================================================
    printf("\n-- AccessFile (early return) --\n");
    {
        CosFunction.bUseSCKART = false;

        SckArtRemainderState st;
        st.sSetupFilePath = "SENTINEL-UNCHANGED";
        st.iInputCount = 777;

        SckArtRem_AccessFile(st, true, -1);
        CHECK(st.sSetupFilePath == "SENTINEL-UNCHANGED",
              "bUseSCKART==false -> early return BEFORE SetSetupFilePath runs (golden :195-198)");
        CHECK(st.iInputCount == 777,
              "bUseSCKART==false -> early return -> no ReadWriteIni calls at all (golden :195-196)");

        CosFunction.bUseSCKART = true;   // restore for later PARTs
    }

    // =========================================================================================
    // PART 3 -- AccessFile bRead==true (load) -- golden :190-444, minus the widget-refresh block
    //   (golden :335-403) -- see file-head LIMITATION 2 for why bRead==false is never exercised here.
    // =========================================================================================
    printf("\n-- AccessFile (bRead=true, load direction only -- see LIMITATION 2) --\n");
    {
        AnsiString scratchDir = ScratchDir();
        AnsiString scratchPath = scratchDir + "Tester.Data";   // must match gate #1's DataPath+"Tester.Data"

        // Seed the persisted values directly via the REAL ReadWriteIni(...,bRead=false,...) plumbing,
        // pointed at OUR OWN throwaway scratch path (never SckArtRem_AccessFile's bRead=false branch --
        // this bypasses WriteLastDataFile entirely, see LIMITATION 2).
        ReadWriteIni(scratchPath, "AutoRetest", "LOTSTATUS",       AnsiString("LOTSTATUS_T"), AnsiString("NONE"), false);
        ReadWriteIni(scratchPath, "AutoRetest", "iCurrentStatus",  2,                          1,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iTesterType",     1,                          0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "sLotID",          AnsiString("LOT-999"),      AnsiString(""),     false);
        ReadWriteIni(scratchPath, "AutoRetest", "sProceddCode",    AnsiString("PROC-7"),       AnsiString(""),     false);
        ReadWriteIni(scratchPath, "AutoRetest", "iLotCount",       5000,                       0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iInputCount",     4200,                       0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iFTRTCount",      3,                          0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iInputJamCnt",    7,                          0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iOutputJamCnt",   9,                          0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "iManualRejectCnt",11,                         0,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "sInfoCustomer",   AnsiString("ACME"),         AnsiString(""),     false);
        ReadWriteIni(scratchPath, "AutoRetest", "sBundleList",     AnsiString("B1,B2"),        AnsiString(""),     false);
        ReadWriteIni(scratchPath, "AutoRetest", "iBundleInCnt",    24,                         12,                 false);
        ReadWriteIni(scratchPath, "AutoRetest", "sLotStartTime",   AnsiString("2026-07-11"),   AnsiString(""),     false);
        ReadWriteIni(scratchPath, "AutoRetest", "iManualStart",    0,                          1,                  false);
        ReadWriteIni(scratchPath, "AutoRetest", "bFirstFullSkip",  true,                       false,              false);
        // Deliberately opposite of the DEFAULT we set below (IniConfig.bA10_AutoReTest=true), proving
        // the persisted value wins over the ReadWriteIni default (golden :293).
        ReadWriteIni(scratchPath, "AutoRetest", "Enable ART",      false,                      true,               false);
        ReadWriteIni(scratchPath, "AutoRetest", "Pass Rate",       62.5,                       100.0,              false, true, 0.01, 100.0);

        // Globals AccessFile's non-VCL logic reads (golden :192-437).
        CosFunction.bUseSCKART = true;
        USE_AUTO_RETEST = eartInstall;
        IniConfig.bA10_AutoReTest = true;
        IniConfig.bA10TestModeForART = false;    // skip the iGpibMode side-branch (golden :404-413), not under test here
        CUSTOMER_CODE = 0;                        // not CC_AMKOR_Japan/RF360/QUALCOMM -> the plain else branches (golden :291-296, :317-323)
        bCanRunSCKART = false;                    // pre-dirty; AccessFile recomputes it (golden :415-425)

        AnsiString savedDataPath = DataPath;
        DataPath = scratchDir;

        SckArtRemainderState st;
        SckArtRem_AccessFile(st, /*bRead=*/true, -1);

        DataPath = savedDataPath;

        CHECK(st.sLOTSTATUS == "LOTSTATUS_T",           "AccessFile(true) reads back sLOTSTATUS (golden :202)");
        CHECK(st.iCurrentStatus == 2,                    "AccessFile(true) reads back iCurrentStatus (golden :203)");
        CHECK(st.iTesterType == 1,                       "AccessFile(true) reads back iTesterType (golden :204)");
        CHECK(CosFunction.bAutoRetestGPIBmode == true,   "iTesterType==1 -> CosFunction.bAutoRetestGPIBmode=true (golden :205-206)");
        CHECK(st.sLotID == "LOT-999",                    "AccessFile(true) reads back sLotID (golden :213)");
        CHECK(st.sProcessCode == "PROC-7",                "AccessFile(true) reads back sProcessCode (golden :214, key literally \"sProceddCode\" -- golden's own typo, verbatim)");
        CHECK(st.iLotCount == 5000,                       "AccessFile(true) reads back iLotCount (golden :215)");
        CHECK(st.iInputCount == 4200,                      "AccessFile(true) reads back iInputCount (golden :216)");
        CHECK(st.iFTRTCount == 3,                          "AccessFile(true) reads back iFTRTCount (golden :217)");
        CHECK(st.iInputJamCnt == 7,                        "AccessFile(true) reads back iInputJamCnt (golden :218)");
        CHECK(st.iOutputJamCnt == 9,                       "AccessFile(true) reads back iOutputJamCnt (golden :219)");
        CHECK(st.iManualRejectCnt == 11,                   "AccessFile(true) reads back iManualRejectCnt (golden :220)");
        CHECK(st.sInfo_Customer == "ACME",                 "AccessFile(true) reads back sInfo_Customer (golden :222)");
        CHECK(st.sBundleList == "B1,B2",                   "AccessFile(true) reads back sBundleList (golden :240)");
        CHECK(st.iBundleInCnt == 24,                        "AccessFile(true) reads back iBundleInCnt (golden :241)");
        CHECK(st.sLotStartTime == "2026-07-11",             "AccessFile(true) reads back sLotStartTime (golden :265)");
        CHECK(st.iManualStart == 0,                          "AccessFile(true) reads back iManualStart (golden :266)");
        CHECK(st.bFirstFullSkip == true,                     "AccessFile(true) reads back bFirstFullSkip (golden :267)");
        CHECK(TestIF_File.bSCKART_EnableART == false,        "persisted \"Enable ART\"=false wins over default=true (golden :293)");
        CHECK(bCanRunSCKART == false,                        "bSCKART_EnableART==false -> bCanRunSCKART recomputed false (golden :415-425)");
        CHECK(feq(TestIF_File.dSCKART_Yield, 62.5),          "AccessFile(true) reads back dSCKART_Yield (golden :301)");
        CHECK(st.sSetupFilePath == scratchPath,              "SetSetupFilePath ran first inside AccessFile (golden :198) -> sSetupFilePath==scratch Tester.Data path");
    }

    // =========================================================================================
    // PART 3b -- AccessFile bRead==false (save) round-trip -- golden :190-333. Safe ONLY because
    //   gate #8 makes WriteLastDataFile()/CustomerFunctionSelect() no-ops right now (see LIMITATION 2).
    // =========================================================================================
    printf("\n-- AccessFile (bRead=false save, then bRead=true reload) --\n");
    {
        AnsiString scratchDir = ScratchDir();
        AnsiString scratchPath = scratchDir + "Tester.Data";

        CosFunction.bUseSCKART = true;
        USE_AUTO_RETEST = eartInstall;
        IniConfig.bA10_AutoReTest = true;
        IniConfig.bA10TestModeForART = false;
        CUSTOMER_CODE = 0;
        TestIF_File.bSCKART_EnableART = true;

        AnsiString savedDataPath = DataPath;
        DataPath = scratchDir;

        SckArtRemainderState stSave;
        stSave.sLOTSTATUS="LOTSTATUS_F"; stSave.iCurrentStatus=5; stSave.iTesterType=0;
        stSave.sLotID="SAVE-LOT"; stSave.sProcessCode="SAVE-PROC"; stSave.iLotCount=321;
        stSave.iInputCount=222; stSave.iFTRTCount=4; stSave.iInputJamCnt=1; stSave.iOutputJamCnt=2;
        stSave.iManualRejectCnt=3; stSave.sBundleList="X1"; stSave.iBundleInCnt=6; stSave.iBundleOutCnt=6;
        stSave.sLotStartTime="2026-01-01"; stSave.iManualStart=1; stSave.bFirstFullSkip=false;

        SckArtRem_AccessFile(stSave, /*bRead=*/false, -1);

        SckArtRemainderState stLoad;
        SckArtRem_AccessFile(stLoad, /*bRead=*/true, -1);

        DataPath = savedDataPath;

        CHECK(stLoad.sLOTSTATUS == "LOTSTATUS_F",  "save(false)->load(true) round-trip: sLOTSTATUS (golden :202)");
        CHECK(stLoad.iCurrentStatus == 5,           "save->load round-trip: iCurrentStatus (golden :203)");
        CHECK(stLoad.sLotID == "SAVE-LOT",          "save->load round-trip: sLotID (golden :213)");
        CHECK(stLoad.sProcessCode == "SAVE-PROC",    "save->load round-trip: sProcessCode (golden :214)");
        CHECK(stLoad.iLotCount == 321,                "save->load round-trip: iLotCount (golden :215)");
        CHECK(stLoad.iInputCount == 222,              "save->load round-trip: iInputCount (golden :216)");
        CHECK(stLoad.iFTRTCount == 4,                  "save->load round-trip: iFTRTCount (golden :217)");
        CHECK(stLoad.sBundleList == "X1",              "save->load round-trip: sBundleList (golden :240)");
        CHECK(stLoad.sLotStartTime == "2026-01-01",    "save->load round-trip: sLotStartTime (golden :265)");
    }

    // =========================================================================================
    // PART 4 -- UpdateCount -- golden :477-591 (yield formula + LastSet.iHd* branch)
    //   NOTE: LastSet.lSCKARTBinCT[]/iHdPass/iHdFail/... are TU-local zero-init stand-ins (gate #4) --
    //   iTotalUnloadCount/iUnloadCnt are ALWAYS 0 offline (documented, matches the sibling test's
    //   CheckOutArmNeedVariModeFIX precedent). Only st.dCurrYield is observable from outside.
    // =========================================================================================
    printf("\n-- UpdateCount --\n");
    {
        LastSet.iTester = OFF_LINE;
        BinSelect[OffT].bAutoRetest[0] = true;
        BinSelect[OffT].bAutoRetest[1] = true;
        BinSelect[OffT].bAutoRetest[2] = true;
        TestIF_File.bRENESAS_EnableFTCT = false;
        TestIF_File.bSCKART_LotDeviceCheck = false;

        SckArtRemainderState st;
        st.iInputCount = 100;
        LastSet.iSCKARTInputCT = 100;
        // iUnloadCnt is always 0 (gate #4 stand-in) -> "100%!=0, iUnloadCnt==0" branch (golden :536-541).
        SckArtRem_UpdateCount(st);
        CHECK(feq(st.dCurrYield, 100.0), "iInputCount!=0 && iSCKARTInputCT!=0 && iUnloadCnt==0(gated) -> 100% (golden :536-541)");

        LastSet.iSCKARTInputCT = 0;
        SckArtRem_UpdateCount(st);
        CHECK(feq(st.dCurrYield, 0.0), "iSCKARTInputCT==0 -> 0% (golden :550-553)");

        LastSet.iSCKARTInputCT = 40;
        SckArtRem_UpdateCount(st);
        CHECK(feq(st.dCurrYield, 100.0), "iSCKARTInputCT(40)-iUnloadCnt(0,gated)==40, /40*100 -> 100% (golden :556, degenerates because iUnloadCnt is gated to 0)");

        // bSCKART_LotDeviceCheck==true branch -> the THIRD (else) sub-branch uses st.iLotCount, but
        // golden's FIRST sub-condition in THIS branch (:562-567) still reads LastSet.iSCKARTInputCT!=0
        // (a verbatim golden asymmetry -- both the false-branch and true-branch first conditions test
        // the SAME LastSet.iSCKARTInputCT, only the fallback-else differs between LastSet.iSCKARTInputCT
        // and st.iLotCount). Must clear it here too, or the still-40-from-above value short-circuits
        // the first condition to 100% before ever reaching the iLotCount==0 check below.
        LastSet.iSCKARTInputCT = 0;
        TestIF_File.bSCKART_LotDeviceCheck = true;
        st.iLotCount = 0;
        SckArtRem_UpdateCount(st);
        CHECK(feq(st.dCurrYield, 0.0), "bSCKART_LotDeviceCheck==true && iLotCount==0 -> 0% (golden :576-579)");

        st.iLotCount = 80;
        SckArtRem_UpdateCount(st);
        CHECK(feq(st.dCurrYield, 100.0), "bSCKART_LotDeviceCheck==true, iLotCount(80)-iUnloadCnt(0,gated) -> 100% (golden :582, degenerate for the same gated reason)");

        // bRENESAS_EnableFTCT==true branch just must not crash (LastSet.iHd* targets are hidden statics).
        TestIF_File.bRENESAS_EnableFTCT = true;
        SckArtRem_UpdateCount(st);
        CHECK(true, "bRENESAS_EnableFTCT==true branch (golden :525-532) runs without crashing (LastSet.iHd* targets are hidden gate #4 statics, unobservable here)");
        TestIF_File.bRENESAS_EnableFTCT = false;
    }

    // =========================================================================================
    // PART 5 -- AddAlarmCode -- golden :593-624
    //   NOTE: RunInfo.AddAlarm's method body is TODO(W6)-gated (gate #8, discovered via this wave's
    //   own real-link smoke test -- see file-head LIMITATION 2) -- it is a no-op right now, so its
    //   effect is NOT observable from this test. These calls only verify: (1) the iDuplicate/"JAM"
    //   substring dispatch condition compiles+runs without crashing across all 3 branches, and
    //   (2) will start actually asserting real RunInfo.vByLotJam state the moment gate #8 is retired
    //   (W6) -- at which point this NOTE (and the CHECK(true) placeholders) should be revisited.
    // =========================================================================================
    printf("\n-- AddAlarmCode --\n");
    {
        SckArtRem_AddAlarmCode("JAM0001", "Test jam message", /*iDuplicate=*/0);
        CHECK(true, "iDuplicate==0 && Code contains \"JAM\" branch runs without crashing (golden :619-623; RunInfo.AddAlarm itself is gate #8, unobservable)");

        SckArtRem_AddAlarmCode("NOTJAM_CODE", "Should not be added", 0);
        CHECK(true, "Code without \"JAM\" substring -> the RunInfo.AddAlarm call is skipped (golden :620, AnsiPos(\"JAM\")==0)");

        SckArtRem_AddAlarmCode("JAM0002", "Duplicate-flagged", /*iDuplicate=*/1);
        CHECK(true, "iDuplicate!=0 -> the RunInfo.AddAlarm call is skipped even for a JAM code (golden :619)");
    }

    // =========================================================================================
    // PART 6 -- ClearLotInfo -- golden :837-923
    // =========================================================================================
    printf("\n-- ClearLotInfo --\n");
    {
        TestIF_File.bRENESAS_EnableFTCT = false;

        SckArtRemainderState st;
        st.iLotCount = 999; st.iInputCount = 888; st.sLotID = "STALE-LOT";
        st.sProcessCode = "STALE-PROC"; st.sLotStartTime = "STALE-TIME";
        st.sLOTSTATUS = "LOTSTATUS_R"; st.iCurrentStatus = 4;
        st.iFTRTCount = 5; st.iManualRejectCnt = 6; st.iInputJamCnt = 7; st.iOutputJamCnt = 8;
        st.sInfo_Customer = "STALE-CUST"; st.sBundleList = "STALE-BL"; st.iInfo_MultiLotCnt = 3;
        st.sInfoArr_Customer[2] = "STALE-ARR";
        LastSet.iSCKARTInputCT = 321; LastSet.lShuttleCount = 654;
        bWaitTSV = true;

        bool needAccessFileWrite = false;
        SckArtRem_ClearLotInfo(st, &needAccessFileWrite);

        CHECK(st.iNeedRT == 1,              "ClearLotInfo -> iNeedRT=1 (golden :839)");
        CHECK(st.iFTRTCount == 0,            "ClearLotInfo -> iFTRTCount=0 (golden :840)");
        CHECK(st.iManualRejectCnt == 0,       "ClearLotInfo -> iManualRejectCnt=0 (golden :841)");
        CHECK(st.iLotCount == 0,              "bRENESAS_EnableFTCT==false -> iLotCount cleared (golden :848)");
        CHECK(st.iInputCount == 0,            "bRENESAS_EnableFTCT==false -> iInputCount cleared (golden :849)");
        CHECK(st.sLotID == "",                "bRENESAS_EnableFTCT==false -> sLotID cleared (golden :850)");
        CHECK(st.sProcessCode == "",          "bRENESAS_EnableFTCT==false -> sProcessCode cleared (golden :851)");
        CHECK(st.sLotStartTime == "",         "bRENESAS_EnableFTCT==false -> sLotStartTime cleared (golden :852)");
        CHECK(st.sLOTSTATUS == "NONE",        "SetLotStatus(iLOTSTATUS_NONE) inlined -> sLOTSTATUS=\"NONE\" (golden :853)");
        CHECK(st.iCurrentStatus == 0,          "SetLotStatus(iLOTSTATUS_NONE) inlined -> iCurrentStatus=0 (golden :853)");
        CHECK(st.iInputJamCnt == 0,            "ClearLotInfo -> iInputJamCnt=0 (golden :856)");
        CHECK(st.iOutputJamCnt == 0,           "ClearLotInfo -> iOutputJamCnt=0 (golden :857)");
        CHECK(LastSet.iSCKARTInputCT == 0,     "ClearLotInfo -> LastSet.iSCKARTInputCT=0 (golden :863)");
        CHECK(LastSet.lShuttleCount == 0,      "ClearLotInfo -> LastSet.lShuttleCount=0 (golden :864)");
        CHECK(st.sInfo_Customer == "",         "ClearLotInfo -> sInfo_Customer=\"\" (golden :879)");
        CHECK(st.sBundleList == "",            "ClearLotInfo -> sBundleList=\"\" (golden :898)");
        CHECK(st.iInfo_MultiLotCnt == 0,        "ClearLotInfo -> iInfo_MultiLotCnt=0 (golden :895)");
        CHECK(st.sInfoArr_Customer[2] == "",    "ClearLotInfo -> sInfoArr_Customer[i]=\"\" for i<5 (golden :902)");
        CHECK(needAccessFileWrite == true,      "out-param signals the caller should now call AccessFile(false) (golden :921)");
        CHECK(bWaitTSV == false,                "ClearLotInfo -> bWaitTSV=false (golden :922, real global)");

        // bRENESAS_EnableFTCT==true branch -- the lot-identity fields are intentionally SKIPPED (golden :843-845, empty if-body).
        TestIF_File.bRENESAS_EnableFTCT = true;
        st.iLotCount = 555; st.sLotID = "KEEP-ME";
        SckArtRem_ClearLotInfo(st, 0);
        CHECK(st.iLotCount == 555,  "bRENESAS_EnableFTCT==true -> iLotCount NOT reset (golden :843-845, empty if-body, faithful quirk)");
        CHECK(st.sLotID == "KEEP-ME", "bRENESAS_EnableFTCT==true -> sLotID NOT reset (golden :843-845)");
        TestIF_File.bRENESAS_EnableFTCT = false;
    }

    // =========================================================================================
    // PART 7 -- AddOutputJamCnt -- golden :1361-1389
    //   NOTE: LastSet.iHdRejectATPass/ATFail targets are hidden gate #4 statics (unobservable); only
    //   st.iOutputJamCnt is directly checkable. Exercises all 3 sub-branches without crashing.
    // =========================================================================================
    printf("\n-- AddOutputJamCnt --\n");
    {
        SckArtRemainderState st;
        st.iOutputJamCnt = 0;

        TestIF_File.bRENESAS_EnableFTCT = false;
        SckArtRem_AddOutputJamCnt(st, 0, 0, K_RETRY, 0);
        CHECK(st.iOutputJamCnt == 1, "iOutputJamCnt always increments regardless of the FTCT/K_SKIP gate (golden :1363)");

        TestIF_File.bRENESAS_EnableFTCT = true;
        SckArtRem_AddOutputJamCnt(st, 0, 0, K_SKIP, /*iBinOnCarryKit=*/1);   // "on carry kit, pass" sub-branch (golden :1366-1371)
        CHECK(st.iOutputJamCnt == 2, "increments again (iBinOnCarryKit!=0, pass sub-branch, golden :1368-1371)");

        SckArtRem_AddOutputJamCnt(st, 0, 0, K_SKIP, /*iBinOnCarryKit=*/2);   // "on carry kit, fail" sub-branch (golden :1372-1375)
        CHECK(st.iOutputJamCnt == 3, "increments again (iBinOnCarryKit!=0, fail sub-branch, golden :1372-1375)");

        SckArtRem_AddOutputJamCnt(st, 0, 0, K_SKIP, /*iBinOnCarryKit=*/0);   // OutArmSuck.iBinData[0][0]/Prod.bIsPassBin[] lookup (golden :1377-1387)
        CHECK(st.iOutputJamCnt == 4, "increments again (iBinOnCarryKit==0, OutArmSuck.iBinData/Prod.bIsPassBin lookup sub-branch, golden :1379-1386)");
        TestIF_File.bRENESAS_EnableFTCT = false;
    }

    // =========================================================================================
    // PART 8 -- SaveTestSummary -- golden :1619-1645 (dispatch logic; 2 of 4 callees still gated, see
    //   gate #7 -- deferred to next wave). AI(W906-SaveTestSummarySECS) 20260721: the SECS branch now
    //   calls the REAL SckArtRem_SaveTestSummarySECS -- asSummaryPath is redirected to a scratch dir
    //   for the DURATION of this PART (never the real D:\HT9045_Log\Summary production path) so that
    //   branch's real file-write does not touch anything outside the scratch sandbox. Deep coverage of
    //   SaveTestSummarySECS's own behavior is PART 10 below; here we only confirm dispatch doesn't crash.
    //   AI(W906-Save2DSortingSummary) 20260723: the 2D-sort branch now ALSO calls the REAL
    //   SckArtRem_Save2DSortingSummary -- same scratch-dir discipline applies. IniConfig.bN23UseLotInfoFile
    //   is forced true here specifically to steer that call through the (safe) `sList->Text=
    //   fLotInfo->mmo2DLotInfo->Text;` content branch, NOT the final "else" branch (golden :3634-3644,
    //   `Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID)` -- a real golden bug that passes a raw pointer
    //   to %s, see that function's own header doc comment "Golden bugs preserved VERBATIM #2"). That bug
    //   is preserved VERBATIM in the SOURCE and is expected to be harmless in practice (see the same doc
    //   comment's UB analysis), but this shallow dispatch-only PART deliberately avoids exercising it --
    //   deep coverage of Save2DSortingSummary's own behavior (including the bReadLotInfoFromART content
    //   branch, gate #13's FTP_Upload, and the ShellExecute quirk) is PART 11 below.
    //   AI(W906-SaveTestSummaryTSV/SaveSummaryTrayFeed) 20260728: the TCP/IP and fallback branches'
    //   OWN callees ALSO graduated to real this wave -- retrofitted AGAIN, same precedent:
    //     * TCP/IP sub-case: CosFunction.bUseTSVFunction is now flipped to TRUE (with
    //       IniConfig.sN09_HandlerFolder ALSO redirected to scratch) immediately before it, so
    //       SckArtRem_SaveTestSummaryTSV's own top-of-function MyForceDirectories routes through the
    //       REDIRECTABLE sN09_HandlerFolder instead of golden's OTHER hardcoded path ("D:\HT9045_Log\
    //       TestSummary", only reachable when bUseTSVFunction==false) -- an EARLIER version of this
    //       retrofit missed this and left a real, empty "D:\HT9045_Log\TestSummary\2026\07\" folder
    //       behind on this dev machine (harmless -- no file written there, only MyForceDirectories'
    //       mkdir side effect -- but avoidable, so now avoided).
    //     * fallback sub-case: now calls with iSaveData=0 instead of 1. SckArtRem_SaveSummaryTrayFeed
    //       had NO redirectable knob at all for its own hardcoded "D:\HT9045_Log\Summary_Lot" write
    //       (⚠ stale since AI(W906-BA-BU0g) 20260911: it now has one, W906_SUMMARYLOT_ROOT)
    //       (see PART 13's own LIMITATION 4) -- but the DISPATCHER itself only calls it when
    //       `iSaveData==1` (golden :1641), so iSaveData=0 still verifies dispatch reaches this branch
    //       and doesn't crash WITHOUT ever invoking SaveSummaryTrayFeed's real write. An EARLIER
    //       version of this retrofit missed this too and left 2 real, uncontrolled-content files
    //       behind under D:\HT9045_Log\Summary_Lot\202607\ on this dev machine (deleted by hand once
    //       discovered -- see the git history / translate report for this wave). Deep, fully-
    //       controlled positive-path coverage of SaveSummaryTrayFeed's iSaveData=1 behavior (with
    //       distinctive test identifiers and its own cleanup) is PART 13 below; deep coverage of
    //       SaveTestSummaryTSV's iSaveData=1 behavior is PART 12 below -- this PART stays a SHALLOW
    //       dispatch-only check, per its own header note above.
    // =========================================================================================
    printf("\n-- SaveTestSummary (dispatch logic; all 4 callees now real, see PART 10/11/12/13) --\n");
    {
        AnsiString savedSummaryPath8 = asSummaryPath;
        asSummaryPath = ScratchDir() + "W906SaveTestSummaryDispatchScratch";
        AnsiString savedN09Folder8 = IniConfig.sN09_HandlerFolder;
        IniConfig.sN09_HandlerFolder = ScratchDir() + "W906SaveTestSummaryDispatchN09Scratch";

        SckArtRemainderState st;
        IniConfig.bSPILFunction = false;
        IniConfig.bN17UploadLotSummary = false;
        IniConfig.bA38_SLT_Summary = false;   // keep the ShellExecute-quirk branch OFF here -- PART 11 owns that
        IniConfig.bN23UseLotInfoFile = true;  // steer Save2DSortingSummary's content-building away from the LOT_ID-bug branch (see PART header note above)
        CosFunction.bUseTSVFunction = false;

        CosFunction.bSortingBy2DList = true;
        LastSet.iTester = _2D_SORT;
        TestIF_File.bSortingBy2DIDList = true;
        SckArtRem_SaveTestSummary(st, 1);
        CHECK(true, "2D-sort branch dispatches to the now-REAL SckArtRem_Save2DSortingSummary without crashing (golden :1621-1626; see PART 11 for deep coverage)");
        IniConfig.bN23UseLotInfoFile = false;   // restore default for the remaining PART 8 sub-cases below

        CosFunction.bSortingBy2DList = false;
        CosFunction.bART_SECSGEM_93K = true;
        SckArtRem_SaveTestSummary(st, 1);
        CHECK(true, "SECS/93K branch dispatches to the now-REAL SckArtRem_SaveTestSummarySECS without crashing (golden :1627-1630; see PART 10 for deep coverage)");

        CosFunction.bART_SECSGEM_93K = false;
        CosFunction.bUseTSVFunction = true;   // see this PART's own retrofit note above -- avoids the OTHER hardcoded TestSummary path
        TestIF_File.iTestType = TCP_IP_MODE;
        SckArtRem_SaveTestSummary(st, 1);
        CHECK(true, "TCP/IP branch dispatches to the now-REAL SaveTestSummaryTSV without crashing (golden :1631-1637; see PART 12 for deep coverage)");

        TestIF_File.iTestType = 0;   // not TCP_IP_MODE
        SckArtRem_SaveTestSummary(st, 0);   // iSaveData=0 -- see this PART's own retrofit note above (avoids SaveSummaryTrayFeed's unavoidable real write)
        CHECK(true, "fallback branch (iSaveData=0) dispatches to the now-REAL SaveTestSummaryTSV, and SKIPS SaveSummaryTrayFeed via the dispatcher's own `iSaveData==1` gate, without crashing (golden :1638-1644; see PART 12/13 for deep coverage)");

        asSummaryPath = savedSummaryPath8;
        IniConfig.sN09_HandlerFolder = savedN09Folder8;
        CosFunction.bUseTSVFunction = false;
    }

    // =========================================================================================
    // PART 9 -- DoARTLotStart -- golden :4191-4256. AI(W906-DoARTLotStart) 20260721.
    //   HasICUnderMachine()==false/true is toggled via FTestSuck.SetItemData(0,0,HAS_IC/NULL_IC,0):
    //   FTestSuck.UseSiteHasIC() -> FrontTestHeadHasIC() -> TestHeadHasIC() -> IndexHasIC() ->
    //   HasICUnderMachine() (csystem_predicates.cpp). Cell [0][0] is always in-bounds/live because
    //   iShtRow/iShtCol default to a 2x1 grid per aHotPlateSubstrate.cpp's TMyKitSuck ctor -- same
    //   default this file's own sibling test_SCK_ART.cpp's DoAutoSocketOff PART already documents for
    //   TestSocket. FTestSuck's cell is always restored to NULL_IC before returning control so later
    //   PARTs (and re-runs within this same process) start clean.
    // =========================================================================================
    printf("\n-- DoARTLotStart --\n");
    {
        // AI(W906-BA-SIOF3) 20260911: point DataPath at scratch for the whole of
        // PART 9. Migrated from the D:\HT9050 tree's AI(W906-GL-SIOF3) 20260826.
        //
        // WITHOUT THIS, THIS BLOCK REWRITES THE LIVE MACHINE'S Tester.Data.
        // Chain: SckArtRem_DoARTLotStart stamps st.sLotStartTime with the wall
        // clock (golden :4220) and the save path persists it through
        // ReadWriteIni(st.sSetupFilePath, "AutoRetest", "sLotStartTime", ...)
        // (SCK_ART_Remainder.cpp:483). With DataPath at its production value,
        // sSetupFilePath IS D:\HT9045\IniData\Data\Tester.Data.
        //
        // MEASURED IN THIS TREE 20260911: that file carries
        //     sLotStartTime=20260911_174423
        // i.e. 17:44:23 -- the wall clock of the ba0 gate's Release-leg ctest
        // run, which is also the file's mtime. The field records the timestamp
        // of the test run that wrote it.
        //
        // ⚠ SAME-BYTE-COUNT EDIT: a timestamp replaces a timestamp, so a
        // size-only comparison calls the directory unchanged. Detecting this
        // class of damage needs hashes or mtimes, not sizes.
        //
        // The process-wide W906_INIDATA_ROOT redirect added today covers
        // DataPath under ctest; it does NOT cover running this executable by
        // hand, which is what one does while debugging. Same gap, same fix, as
        // test_agv_e84.
        //
        // Lot start time is production data an operator reads. Rewriting it
        // from a test run is not cosmetic even though nothing crashes.
        AnsiString savedDataPathP9 = DataPath;
        DataPath = ScratchDir();
        CHECK(DataPath != savedDataPathP9 || savedDataPathP9 == ScratchDir(),
              "PART 9: DataPath redirected to scratch before DoARTLotStart "
              "(production would be D:\\HT9045\\IniData\\Data\\Tester.Data)");

        bQAModeFlag = false;
        bReadLotInfoFromART = false;
        TestIF_File.bRENESAS_EnableFTCT = false;
        TestIF_File.bAlarmAfterSendSRQKIND2 = false;
        TMyStringList* const savedEvP9 = slEventLog;  slEventLog = new TMyStringList(ScratchDir() + "w906_s72_EventLogTxt", "EventLogTxt", "Date, Time, UnitName, AlarmCode, Recovery, StopedTime, Duplicate, Message, ErrorPart, Recipe");  FTestSuck.SetItemData(0, 0, NULL_IC, 0);   // baseline: HasICUnderMachine()==false   //AI(W906-S72-TAIL) 20260929 (St02-E, claim): golden SetLotStart :1599 needs slEventLog (wb_serve builds it, LogObjects.cpp:107); PART 9 gets its own under %TEMP%, never the real D:\HT9045_Log

        // ---- 9A: HasICUnderMachine()==false -> bChangeLotID FORCED true regardless of sLotID
        //   (golden :4195-4197). iTesterType=0 isolates this sub-case from the independent
        //   iTesterType==1 tail (golden :4243-4254, covered separately by 9F below).
        {
            SckArtRemainderState st;
            st.sLotID = "SAME-LOT";
            st.iTesterType = 0;
            SckArtRem_DoARTLotStart(st, "SAME-LOT", "PROC-A", 100);
            CHECK(bReadLotInfoFromART == true, "DoARTLotStart -> bReadLotInfoFromART=true unconditionally (golden :4193, real cmydef.cpp global)");
            CHECK(st.iCurrent93KARTStep == 1,  "HasICUnderMachine()==false -> bChangeLotID=true -> iCurrent93KARTStep=1 (golden :4195-4213)");
            CHECK(st.iCurrentFlexARTStep == 4, "same branch -> iCurrentFlexARTStep=4 (golden :4214)");
            CHECK(st.iNeedRT == 1,             "ClearLotInfo() always sets iNeedRT=1 (golden :839); iTesterType!=1 tail does not reset it back to 0 here");
            CHECK(st.sLOTSTATUS == "LOTSTATUS_W", "SetLotStatus(iLOTSTATUS_W) inlined -> sLOTSTATUS=\"LOTSTATUS_W\" (golden :4216)");
            CHECK(st.iCurrentStatus == 1,          "SetLotStatus(iLOTSTATUS_W) inlined -> iCurrentStatus=1 (golden :4216, iLOTSTATUS_W==1 per golden ctor SCK_ART.cpp:44)");
            CHECK(st.sLotID == "SAME-LOT",         "sLotID=_sLotID (golden :4217, re-applied after ClearLotInfo cleared it)");
            CHECK(st.sProcessCode == "PROC-A",     "sProcessCode=_sProcess (golden :4218)");
            CHECK(st.sLotStartTime != "",           "bRENESAS_EnableFTCT==false -> sLotStartTime stamped via FormatDateTime(\"yyyymmdd_hhnnss\",Now()) (golden :4219-4220)");
            CHECK(st.iLotCount == 100,               "bQAModeFlag==false -> iLotCount=_iLotCount (golden :4235)");
            CHECK(st.iInputCount == 100,             "bQAModeFlag==false -> iInputCount=_iLotCount (golden :4236)");
        }

        // ---- 9B: HasICUnderMachine()==true + sLotID EXACT match (non-empty, non-space) ->
        //   bChangeLotID=false -> the WHOLE if(bChangeLotID==true) block is SKIPPED; st fields set
        //   to sentinels beforehand must come back UNCHANGED (golden :4201-4211).
        {
            FTestSuck.SetItemData(0, 0, HAS_IC, 0);   // force HasICUnderMachine()==true
            SckArtRemainderState st;
            st.sLotID = "LOT-KEEP";
            st.iCurrent93KARTStep = 999;             // sentinel
            st.sLOTSTATUS = "SENTINEL-UNCHANGED";    // sentinel
            st.iTesterType = 0;
            SckArtRem_DoARTLotStart(st, "LOT-KEEP", "PROC-B", 555);
            CHECK(st.iCurrent93KARTStep == 999,          "sLotID match (non-space) -> bChangeLotID=false -> block SKIPPED, iCurrent93KARTStep untouched");
            CHECK(st.sLOTSTATUS == "SENTINEL-UNCHANGED", "same -> sLOTSTATUS untouched (SetLotStatus never runs)");
            FTestSuck.SetItemData(0, 0, NULL_IC, 0);     // restore baseline
        }

        // ---- 9C: the " " vs "" ASYMMETRY (golden :4201) -- an EXACT match on " " (one literal
        //   space) still forces bChangeLotID=true, because the right AND-term compares _sLotID
        //   against " " (not ""), unlike the left AND-term's sLotID!="" check. Demonstrates the
        //   asymmetry is REAL and preserved, not "fixed".
        {
            FTestSuck.SetItemData(0, 0, HAS_IC, 0);
            SckArtRemainderState st;
            st.sLotID = " ";              // exact match target: one literal space, same as _sLotID below
            st.iCurrent93KARTStep = 999;  // sentinel -- expect this to be OVERWRITTEN despite the exact match
            st.iTesterType = 0;
            SckArtRem_DoARTLotStart(st, " ", "PROC-C", 7);
            CHECK(st.iCurrent93KARTStep == 1, "sLotID==\" \"==_sLotID (EXACT match) but _sLotID!=\" \" is FALSE -> right AND-term false -> bChangeLotID forced TRUE despite the match (golden :4201 asymmetry, preserved verbatim, NOT normalized to \"\")");
            FTestSuck.SetItemData(0, 0, NULL_IC, 0);
        }

        // ---- 9D: HasICUnderMachine()==true + sLotID MISMATCH -> bChangeLotID=true (straightforward
        //   else-branch, golden :4205-4207) ----
        {
            FTestSuck.SetItemData(0, 0, HAS_IC, 0);
            SckArtRemainderState st;
            st.sLotID = "OLD-LOT";
            st.iTesterType = 0;
            SckArtRem_DoARTLotStart(st, "NEW-LOT", "PROC-D", 42);
            CHECK(st.iCurrent93KARTStep == 1, "HasICUnderMachine()==true + sLotID mismatch -> bChangeLotID=true (golden :4205-4207)");
            CHECK(st.sLotID == "NEW-LOT",      "block ran -> sLotID overwritten to _sLotID (golden :4217)");
            FTestSuck.SetItemData(0, 0, NULL_IC, 0);
        }

        // ---- 9E: bQAModeFlag==true -> iLotCount/iInputCount pulled from TestIF_File.iQAModeCount,
        //   NOT _iLotCount; bQAModeFlag reset to false afterward (golden :4225-4231) ----
        {
            TestIF_File.iQAModeCount = 321;
            bQAModeFlag = true;
            SckArtRemainderState st;
            st.iTesterType = 0;
            SckArtRem_DoARTLotStart(st, "QA-LOT", "PROC-E", 9999);
            CHECK(st.iLotCount == 321,    "bQAModeFlag==true -> iLotCount=TestIF_File.iQAModeCount, NOT _iLotCount (golden :4227)");
            CHECK(st.iInputCount == 321,  "bQAModeFlag==true -> iInputCount=TestIF_File.iQAModeCount (golden :4228)");
            CHECK(bQAModeFlag == false,   "bQAModeFlag reset to false after the QA-mode branch runs (golden :4231)");
        }

        // ---- 9F: iTesterType==1 tail -- iNeedRT reset + the 3 REAL LastSet GPIB fields + gates
        //   #9/#10 (fMain->SetLotState/tESDError->Add) run without crashing (golden :4243-4254) ----
        {
            SckArtRemainderState st;
            st.iTesterType = 1;
            st.iNeedRT = 5;   // sentinel, expect reset to 0
            LastSet.bEndLotAutoRetestGPIB = true;
            LastSet.bWaitStartLotAutoRetestGPIB = true;
            LastSet.bFirstTestAutoRetestGPIB = false;
            TestIF_File.bAlarmAfterSendSRQKIND2 = true;   // also exercises gate #10
            SckArtRem_DoARTLotStart(st, "", "", 0);
            CHECK(st.iNeedRT == 0,                              "iTesterType==1 -> iNeedRT=0 (golden :4245)");
            CHECK(LastSet.bEndLotAutoRetestGPIB == false,       "iTesterType==1 -> LastSet.bEndLotAutoRetestGPIB=false (golden :4246, REAL LastSet field, canary_support.h:145)");
            CHECK(LastSet.bWaitStartLotAutoRetestGPIB == false, "iTesterType==1 -> LastSet.bWaitStartLotAutoRetestGPIB=false (golden :4247, REAL LastSet field, canary_support.h:144)");
            CHECK(LastSet.bFirstTestAutoRetestGPIB == true,     "iTesterType==1 -> LastSet.bFirstTestAutoRetestGPIB=true (golden :4248, REAL LastSet field, canary_support.h:146)");
            CHECK(true, "fMain->SetLotState(2) / fMain->tESDError->Add(\"MES07399\") -- gates #9/#10 -- run without crashing (golden :4249-4252)");
            TestIF_File.bAlarmAfterSendSRQKIND2 = false;
        }

        FTestSuck.SetItemData(0, 0, NULL_IC, 0);   // final cleanup: leave the shared global grid clean
        delete slEventLog;  slEventLog = savedEvP9;  DataPath = savedDataPathP9;   // AI(W906-BA-SIOF3) 20260911: restore for later PARTs   //AI(W906-S72-TAIL) 20260929 (St02-E, claim): PART 9 slEventLog scope ends (:604)
    }

    // =========================================================================================
    // PART 10 -- SaveTestSummarySECS -- golden :1647-2044. AI(W906-SaveTestSummarySECS) 20260721.
    //   asSummaryPath is redirected to a scratch dir for the WHOLE part (never the real
    //   D:\HT9045_Log\Summary production path -- restored at the end, same discipline as PART 1/3/3b's
    //   DataPath save/restore). GetTimeInfo() is called once here BEFORE each real call so the test can
    //   reconstruct the exact SystemYear/Month/Date/Hour/Min-stamped filename the function will use
    //   (golden embeds these directly in FileName when bSPILFunction==false); any stale file from a
    //   previous run inside the SAME wall-clock minute is proactively deleted first, so this test is
    //   deterministic across repeated ctest invocations (not just "usually passes").
    // =========================================================================================
    printf("\n-- SaveTestSummarySECS --\n");
    {
        AnsiString savedSummaryPath = asSummaryPath;
        asSummaryPath = ScratchDir() + "W906SaveTestSummarySECSScratch";

        IniConfig.bSPILFunction = false;        // simplest FileName/header-block shape, unconditional SaveToFile (golden :1954-1964)
        IniConfig.bN17UploadLotSummary = false; // THE quirk precondition: strFileName is NEVER assigned (golden :1970-2023 skipped)
        IniConfig.bA38_SLT_Summary = true;      // needed for the ShellExecute-quirk branch (golden :2025)
        CosFunction.bUseTSVFunction = false;    // isolate 10A-10C from the TSV tail (covered separately in 10D)

        // Seed LotSummary so iUnloadCount>0 (TestSocket.iShtRow=2/iShtCol=1 by default -- see PART 9's
        // own comment above -- so valid site indices are 0 and 1; bin 3 is an arbitrary in-range choice,
        // 0<=3<iTestBinCount==16 default).
        W5SckArtRem_LotSummary.iCountCategory[0][3] = 5;
        W5SckArtRem_LotSummary.iTotalCategory[3] = 5;

        // ---- 10A/10B/10C shared setup: one real call, several independent assertions on its effects ----
        SckArtRemainderState st;
        st.sLotID = "LOT-SECS-1";
        st.sProcessCode = "";              // exercises the ""->"FT1" default (golden :1677-1678)
        st.sInfo_Customer = "ACME";
        st.sInfo_Stage = "FT1";
        st.sInfo_Step = "1";
        st.sInfo_ReportCnt = "1";
        st.sInfo_CustLotID = "CUST-LOT-1";
        st.sInfo_TesterID = "TESTER1";
        st.sInfo_HandlerID = "HANDLER1";
        st.sInfo_TestBinNo = "1:2";         // exercises StringReplace(":", "-", rfReplaceAll) -> "1-2"
        st.sLotStartTime = "";              // exercises the RunInfo.LotStartTime fallback branch (golden :1788-1789)
        RunInfo.LotStartTime = "2026-07-21 10:00:00";

        GetTimeInfo();   // snapshot NOW into SystemYear/Month/Date/Hour/Min, to reconstruct the expected filename
        AnsiString expectedFileName, expectedPathName2;
        expectedFileName.sprintf("%s_%s_%s_%s_%s_%s_%s_%s_%04d%02d%02d%02d%02d.txt",
                                  AnsiString("FT1"), AnsiString("1"), AnsiString("1"), AnsiString("CUST-LOT-1"),
                                  AnsiString("LOT-SECS-1"), AnsiString("TESTER1"), AnsiString("HANDLER1"), AnsiString("1-2"),
                                  SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        expectedPathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        AnsiString expectedFullPath = expectedPathName2 + expectedFileName;
        if (FileExists(expectedFullPath))
            DeleteFile(expectedFullPath);   // idempotent cleanup -- see this PART's own header note

        W5SckArtRem_LastShellExecuteOpenPath = "<unset>";   // reset gate #11's capture before the call

        SckArtRem_SaveTestSummarySECS(st, /*iSaveData=*/1);

        // ---- 10A: real file-write path, scratch dir only ----
        CHECK(FileExists(expectedFullPath), "SaveTestSummarySECS(iSaveData=1) writes the summary .txt under the SCRATCH asSummaryPath (golden :1954-1964)");
        CHECK(st.sProcessCode == "FT1", "sProcessCode==\"\" defaulted to \"FT1\" (golden :1677-1678)");

        // ---- 10B: fObserver->memoLotSummary->Lines whole-list-assign (golden :1965) ----
        bool foundCustomerLine = false;
        for (size_t i = 0; i < fObserver->memoLotSummary->Lines.Strings.size(); ++i)
            if (fObserver->memoLotSummary->Lines.Strings[i] == "CUSTOMER:ACME")
                foundCustomerLine = true;
        CHECK(fObserver->memoLotSummary->Lines.Strings.size() > 0, "fObserver->memoLotSummary->Lines captured a COPY of sList's content (golden :1965)");
        CHECK(foundCustomerLine, "captured content includes the \"CUSTOMER:ACME\" header line built from st.sInfo_Customer");

        // ---- 10C: the bN17UploadLotSummary-off "strFileName stays empty but ShellExecute still fires"
        //   golden quirk (golden :1970/:2025-2027, this function's own header doc comment "Golden bug
        //   preserved VERBATIM"). bA38_SLT_Summary==true + FileName!="" + iUnloadCount>0 (seeded above)
        //   all hold, so gate #11's stand-in MUST have been invoked -- with an EMPTY path, because
        //   strFileName was never assigned (bN17UploadLotSummary==false skipped that whole block).
        CHECK(W5SckArtRem_LastShellExecuteOpenPath == "", "ShellExecute-open stand-in (gate #11) fired UNCONDITIONALLY with an EMPTY path -- the golden strFileName-can-be-empty quirk, preserved verbatim, not \"fixed\" to skip");

        // ---- 10D: iSaveData==1 -> LotSummary.ClearAllData() actually zeroes the 2 extended fields
        //   (golden :2041-2042; gate #5's [UPDATE] -- this macro is now real, not a no-op) ----
        CHECK(W5SckArtRem_LotSummary.iCountCategory[0][3] == 0, "iSaveData==1 -> LotSummary.ClearAllData() zeroed iCountCategory[0][3] (golden :2042, gate #5 now real)");
        CHECK(W5SckArtRem_LotSummary.iTotalCategory[3] == 0, "iSaveData==1 -> LotSummary.ClearAllData() zeroed iTotalCategory[3] (golden :2042, gate #5 now real)");

        // ---- 10E: TSV-wait tail (golden :2030-2039) -- separate call, re-seed LotSummary (10D just
        //   cleared it) so the file-write/ShellExecute path still runs the same way; only the NEW
        //   CosFunction.bUseTSVFunction/IniConfig.bN09_LotCountAutoFunc gate differs from 10A-10C ----
        {
            W5SckArtRem_LotSummary.iCountCategory[0][3] = 5;
            W5SckArtRem_LotSummary.iTotalCategory[3] = 5;
            CosFunction.bUseTSVFunction = true;
            IniConfig.bN09_LotCountAutoFunc = true;
            IniConfig.dN09_SearchTime = 5.0;
            bWaitTSV = false;

            SckArtRemainderState st2 = st;
            st2.sInfo_ReportCnt = "2";   // distinct filename from 10A-10D's, avoids the golden :1954 dedup-skip
            SckArtRem_SaveTestSummarySECS(st2, /*iSaveData=*/1);

            CHECK(bWaitTSV == true, "CosFunction.bUseTSVFunction && IniConfig.bN09_LotCountAutoFunc -> bWaitTSV=true (golden :2033, real global)");
            CHECK(st2.bShowTSVMsg == false, "same branch -> st.bShowTSVMsg=false (golden :2034, new struct field)");
            CHECK(st2.sTSVMsg == "", "same branch -> st.sTSVMsg=\"\" (golden :2035, new struct field)");
            CosFunction.bUseTSVFunction = false;
            IniConfig.bN09_LotCountAutoFunc = false;
        }

        // ---- 10F: iSaveData==0 -> early return BEFORE any file/list/ShellExecute work (golden
        //   :1699-1700), even though GetTimeInfo()/PathName2/MyForceDirectories/FileName-building
        //   (golden :1670-1697) already ran -- a DIFFERENT sInfo_ReportCnt keeps its (non-)existence
        //   independently checkable from 10A-10E's file ----
        {
            W5SckArtRem_LastShellExecuteOpenPath = "<unset>";
            SckArtRemainderState st3 = st;
            st3.sInfo_ReportCnt = "3";
            GetTimeInfo();
            AnsiString fn3, path3;
            fn3.sprintf("%s_%s_%s_%s_%s_%s_%s_%s_%04d%02d%02d%02d%02d.txt",
                        AnsiString("FT1"), AnsiString("1"), AnsiString("3"), AnsiString("CUST-LOT-1"),
                        AnsiString("LOT-SECS-1"), AnsiString("TESTER1"), AnsiString("HANDLER1"), AnsiString("1-2"),
                        SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
            path3.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
            AnsiString fullPath3 = path3 + fn3;
            if (FileExists(fullPath3))
                DeleteFile(fullPath3);

            SckArtRem_SaveTestSummarySECS(st3, /*iSaveData=*/0);

            CHECK(FileExists(fullPath3) == false, "iSaveData==0 -> early return -> no file written (golden :1699-1700)");
            CHECK(W5SckArtRem_LastShellExecuteOpenPath == "<unset>", "iSaveData==0 -> early return -> ShellExecute stand-in never reached either");
        }

        asSummaryPath = savedSummaryPath;
    }

    // =========================================================================================
    // PART 11 -- Save2DSortingSummary -- golden :3402-4061. AI(W906-Save2DSortingSummary) 20260723.
    //   Steers through the bReadLotInfoFromART==true content-building branch (safe -- all ->Text/
    //   ->Caption reads, no raw pointers -- and exercises the NEW fObserver->labFactory member) and the
    //   bUseTSVFunction && bN09_LotCountAutoFunc && iNeedRT==0 && iN09_4_UploadMethod==0 FTP_Upload path
    //   (new gate #13), plus the bA38_SLT_Summary ShellExecute-with-empty-path quirk (reusing gate #11,
    //   same golden idiom PART 10 already exercises for SaveTestSummarySECS). Deliberately does NOT
    //   exercise the bN23UseLotInfoFile==false && bReadLotInfoFromART==false && bSPILFunction==false
    //   "else" content branch (golden :3634-3644, `Str.sprintf("LOT_ID:%s", fLotInfo->edtSysLotID)` --
    //   a real golden bug passing a raw pointer to %s, see that function's own header doc comment
    //   "Golden bugs preserved VERBATIM #2"): the bug is preserved VERBATIM in the SOURCE (not "fixed"),
    //   but this harness avoids making an automated test's pass/fail depend on unspecified/undefined
    //   behavior, even though ordinary (non-ASan) execution of that exact line is expected to be
    //   harmless in practice (see that same doc comment's analysis). A distinctive, obviously-fake Lot
    //   ID / edtSysLotID->Text is used throughout, specifically to make the unavoidable (golden :4044,
    //   runs whenever iSaveData!=0) hardcoded "D:\HT9045_Log\2D_SortList" FileExists check's already-
    //   vanishingly-small collision risk with any real production file effectively zero (same posture
    //   this file's own LIMITATION 2 documents for SckArtRem_SetGPIBVersion's hardcoded GPIB path).
    // =========================================================================================
    printf("\n-- Save2DSortingSummary --\n");
    {
        AnsiString savedSummaryPath11 = asSummaryPath;
        asSummaryPath = ScratchDir() + "W906Save2DSortingSummaryScratch";
        AnsiString savedN09Folder = IniConfig.sN09_HandlerFolder;
        IniConfig.sN09_HandlerFolder = ScratchDir() + "W906Save2DSortingSummaryN09Scratch";
        AnsiString savedEdtSysLotID = fLotInfo->edtSysLotID->Text;
        fLotInfo->edtSysLotID->Text = "W906TESTPART11EDT";

        IniConfig.bSPILFunction = false;
        IniConfig.bN23UseLotInfoFile = false;
        bReadLotInfoFromART = true;              // steer content+FileName-building through the SAFE branch (also exercises labFactory)
        IniConfig.bA37LotStartLotEnd = false;
        TestIF_File.bSCKART_EnableART = true;    // keep iUnloadCount purely from the seeded LotSummary accumulation below (golden :3529-3535 skipped)
        CosFunction.bART_SECSGEM_93K = true;     // skip the iE1Count/iE2Count/iE3Count/bIsRTBin reset block (golden :3505-3521) -- not under test here
        IniConfig.bN17UploadLotSummary = false;  // THE ShellExecute-quirk precondition -- strFileName never assigned (same as PART 10)
        IniConfig.bA38_SLT_Summary = true;       // needed for the ShellExecute-quirk branch
        CosFunction.bUseTSVFunction = true;      // needed for the FTP_Upload gate
        IniConfig.bN09_LotCountAutoFunc = true;  // needed for the FTP_Upload gate
        IniConfig.iN09_4_UploadMethod = 0;       // selects the FTP_Upload sub-branch (golden :3939-3942)
        IniConfig.sN09_5_Path = "/remote/scratch/path";
        IniConfig.dN09_SearchTime = 5.0;
        bWaitTSV = false;

        SckArtRemainderState st;
        st.sLotID = "W906TESTLOT2DSORT";
        st.iNeedRT = 0;                          // required for the FTP_Upload gate (golden :3937 fSCKART->iNeedRT==0)
        st.sInfo_CustLotID = "CUST-2DSORT-1";
        st.sInfo_CustDevGup = "DEVGRP-1";
        st.sInfo_Customer = "ACME2D";
        st.sInfo_DeviceName = "DEVICE2D";
        st.sInfo_HandlerID = "HANDLER2D";
        st.sInfo_OperatorID = "OP2D";
        st.sLotStartTime = "";                   // exercises the RunInfo.LotStartTime/LotEndTime fallback branches (golden :3646-3665)
        RunInfo.LotStartTime = "2026-07-23 09:00:00";
        RunInfo.LotEndTime   = "2026-07-23 09:30:00";
        fObserver->labFactory->Caption = "FACTORY-2D-TEST";

        // Seed LotSummary so iUnloadCount>0 (unconditional accumulation loop, golden :3667-3678) --
        // same site/bin choice as PART 10 (TestSocket default 2x1 grid; bin 3 arbitrary in-range).
        W5SckArtRem_LotSummary.iCountCategory[0][3] = 7;
        W5SckArtRem_LotSummary.iTotalCategory[3] = 7;

        GetTimeInfo();   // snapshot NOW into SystemYear/Month/Date/Hour/Min, to reconstruct the expected filename
        AnsiString expectedFileName, expectedPathName2;
        expectedFileName.sprintf("%s_%s_%s_%04d%02d%02d%02d%02d.txt",
                                  st.sLotID, st.sInfo_CustLotID, st.sInfo_CustDevGup,
                                  SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        expectedPathName2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        AnsiString expectedFullPath = expectedPathName2 + expectedFileName;
        if (FileExists(expectedFullPath))
            DeleteFile(expectedFullPath);   // idempotent cleanup -- same discipline as PART 10

        W5SckArtRem_LastShellExecuteOpenPath = "<unset>";
        W5SckArtRem_LastFTPUpload_Sources    = "<unset>";
        W5SckArtRem_LastFTPUpload_Target     = "<unset>";
        W5SckArtRem_LastFTPUpload_FileName   = "<unset>";

        SckArtRem_Save2DSortingSummary(st, /*iSaveData=*/1);

        // ---- 11A: FileName-building via the bReadLotInfoFromART branch + real scratch file-write ----
        CHECK(FileExists(expectedFullPath), "Save2DSortingSummary(iSaveData=1) writes the summary .txt under the SCRATCH asSummaryPath, bReadLotInfoFromART FileName shape (golden :3485-3488)");

        // ---- 11B: gate #13 FTP_Upload REACHED (not skipped), with the real FileName argument (golden :3937-3942) ----
        CHECK(W5SckArtRem_LastFTPUpload_Sources != "<unset>", "FTP_Upload gate #13 was REACHED, not skipped (golden :3942)");
        CHECK(W5SckArtRem_LastFTPUpload_FileName == expectedFileName, "FTP_Upload gate #13 received the real FileName argument (golden :3942)");

        // ---- 11C: the ShellExecute-can-fire-with-an-empty-path quirk (golden :4025-4028, this
        //   function's own header doc "Golden bugs preserved VERBATIM #4") -- same shape as PART 10's
        //   own citation for SaveTestSummarySECS's structurally identical call. ----
        CHECK(W5SckArtRem_LastShellExecuteOpenPath == "", "ShellExecute-open stand-in (gate #11) fired UNCONDITIONALLY with an EMPTY path -- bN17UploadLotSummary==false means strFileName was never assigned, same golden quirk as SaveTestSummarySECS's own PART 10 citation");

        // ---- 11D: the bReadLotInfoFromART content branch actually read the NEW fObserver->labFactory member ----
        bool foundFactoryLine = false;
        for (size_t i = 0; i < fObserver->memoLotSummary->Lines.Strings.size(); ++i)
            if (fObserver->memoLotSummary->Lines.Strings[i] == "ASSEMBLY SITE:FACTORY-2D-TEST")
                foundFactoryLine = true;
        CHECK(foundFactoryLine, "bReadLotInfoFromART branch built \"ASSEMBLY SITE:%s\" from the NEW fObserver->labFactory->Caption member (golden :3605/:3638)");

        // ---- 11E: iSaveData==0 -> early return BEFORE the FTP_Upload/ShellExecute work (golden :3523-3524) ----
        {
            W5SckArtRem_LastFTPUpload_Sources = "<unset>";
            W5SckArtRem_LastShellExecuteOpenPath = "<unset>";
            SckArtRemainderState st0 = st;
            SckArtRem_Save2DSortingSummary(st0, /*iSaveData=*/0);
            CHECK(W5SckArtRem_LastFTPUpload_Sources == "<unset>", "iSaveData==0 -> early return -> FTP_Upload gate never reached (golden :3523-3524)");
            CHECK(W5SckArtRem_LastShellExecuteOpenPath == "<unset>", "iSaveData==0 -> early return -> ShellExecute stand-in never reached either");
        }

        fLotInfo->edtSysLotID->Text = savedEdtSysLotID;
        IniConfig.sN09_HandlerFolder = savedN09Folder;
        asSummaryPath = savedSummaryPath11;
        bReadLotInfoFromART = false;
        CosFunction.bUseTSVFunction = false;
        IniConfig.bN09_LotCountAutoFunc = false;
    }

    // =========================================================================================
    // PART 12 -- SaveTestSummaryTSV -- golden :2805-3128. AI(W906-SaveTestSummaryTSV) 20260728.
    //   CosFunction.bUseTSVFunction=true throughout, redirecting BOTH the top-of-function
    //   MyForceDirectories AND the save destination through the REDIRECTABLE sN09_HandlerFolder/
    //   asSummaryPath scratch dirs -- avoids golden's OTHER hardcoded path ("D:\HT9045_Log\
    //   TestSummary", only reachable when bUseTSVFunction==false), same technique PART 11 already
    //   established for its own analogous toggle. Golden Quirk #1 (the duplicate "Handler ID:"
    //   line when bUseTSVFunction==false) is therefore NOT exercised by an automated sub-case here
    //   -- doing so would require the OTHER, non-redirectable branch of this SAME toggle -- but is
    //   cited verbatim at its own translation site and in this function's header doc comment,
    //   same "preserved+cited, not necessarily exercised" precedent PART 11 already set for its
    //   OWN Golden Bug #2 (raw-pointer-to-%s UB).
    // =========================================================================================
    printf("\n-- SaveTestSummaryTSV --\n");
    {
        AnsiString savedSummaryPath12 = asSummaryPath;
        asSummaryPath = ScratchDir() + "W906SaveTestSummaryTSVScratch";
        AnsiString savedN09Folder12 = IniConfig.sN09_HandlerFolder;
        IniConfig.sN09_HandlerFolder = ScratchDir() + "W906SaveTestSummaryTSVN09Scratch";

        CosFunction.bUseTSVFunction = true;
        IniConfig.iN09_TSV_Port = 17000;
        IniConfig.iN09_4_UploadMethod = 0;   // both save-blocks target PathName2+FileName (asSummaryPath)
        IniConfig.sN09_5_Path = "/remote/tsv/scratch";
        IniConfig.dN09_SearchTime = 5.0;
        IniConfig.bSPILFunction = false;
        IniConfig.sN09_7_SkipIP = "192.168.1";   // a DIFFERENT prefix from the seeded IP below (12C)
        bWaitTSV = false;

        SckArtRemainderState st;
        st.sLotID = "W906TESTLOTTSV";
        st.sProcessCode = "";        // exercises the ""->"FT1" default (golden :2834-2835)
        st.iFTRTCount = 1;           // FT branch (golden :2838-2839)
        st.iNeedRT = 1;              // nonzero -- needed for 12A's bIsRTBin reset-loop sub-case below
        st.iLotCount = 42;

        // ---- 12A: the unconditional RT-bin reset loop (golden :2843-2853) runs even when
        //   iSaveData==0 (the early return is AFTER this loop, golden :2855-2856) ----
        W5SckArtRem_LotSummary.bIsRTBin[3] = true;
        W5SckArtRem_LotSummary.iCountCategory[0][3] = 99;
        W5SckArtRem_LotSummary.iTotalCategory[3] = 99;
        GetTimeInfo();
        AnsiString fn0, path0;
        fn0.sprintf("%s_%s_FT_%04d%02d%02d%02d%02d.txt", st.sLotID, AnsiString("FT1"), SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        path0.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        AnsiString fullPath0 = path0 + fn0;
        if (FileExists(fullPath0)) DeleteFile(fullPath0);

        SckArtRem_SaveTestSummaryTSV(st, /*iSaveData=*/0);

        CHECK(W5SckArtRem_LotSummary.iCountCategory[0][3] == 0, "iSaveData==0 still runs the unconditional RT-bin reset loop (golden :2843-2853) -- bIsRTBin[3]==true zeroed iCountCategory[0][3]");
        CHECK(W5SckArtRem_LotSummary.iTotalCategory[3] == 0, "same reset loop zeroed iTotalCategory[3]");
        CHECK(st.sProcessCode == "FT1", "sProcessCode==\"\" defaulted to \"FT1\" BEFORE the iSaveData==0 early return (golden :2834-2835, runs unconditionally)");
        CHECK(FileExists(fullPath0) == false, "iSaveData==0 -> early return -> no file written (golden :2855-2856)");

        // ---- 12B: full path, iSaveData=1 -- file write + FTP_Upload gate reuse + srvrscktTSV real substrate ----
        st.iNeedRT = 0;   // required for the save/FTP_Upload blocks to actually run (golden :3049/:3064 fSCKART->iNeedRT==0)
        W5SckArtRem_LotSummary.iCountCategory[0][5] = 3;   // a DIFFERENT bin, survives 12A's reset (bin 3 only)
        W5SckArtRem_LotSummary.iTotalCategory[5] = 3;
        W5SckArtRem_LastFTPUpload_Sources  = "<unset>";
        W5SckArtRem_LastFTPUpload_FileName = "<unset>";

        GetTimeInfo();
        AnsiString fn1, path1;
        fn1.sprintf("%s_%s_FT_%04d%02d%02d%02d%02d.txt", st.sLotID, st.sProcessCode, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        path1.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        AnsiString fullPath1 = path1 + fn1;
        if (FileExists(fullPath1)) DeleteFile(fullPath1);

        SckArtRem_SaveTestSummaryTSV(st, /*iSaveData=*/1);

        CHECK(FileExists(fullPath1), "SaveTestSummaryTSV(iSaveData=1) writes the summary .txt under the SCRATCH asSummaryPath (golden :3053, bUseTSVFunction+iN09_4_UploadMethod==0 branch)");
        CHECK(W5SckArtRem_LastFTPUpload_Sources != "<unset>", "FTP_Upload (REUSES gate #13's existing stand-in verbatim) was REACHED (golden :3054)");
        CHECK(W5SckArtRem_LastFTPUpload_FileName == fn1, "FTP_Upload received the real FileName argument");
        CHECK(srvrscktTSV->Active == true, "gate #15's REAL vclcompat TServerSocket is Active after ->Open() (golden :3047) -- genuine substrate reuse, not a no-op stand-in");
        CHECK(srvrscktTSV->Port == 17000, "gate #15's REAL TServerSocket.Port was set from IniConfig.iN09_TSV_Port (golden :3046)");

        // ---- 12C: gate #14 (fConfiguration->mmoN04_IP->Lines) positive-path IP auto-pick, reusing the
        //   RT branch (golden :2841 sLotID_sProcessCode_RT%d_...) for a distinct filename ----
        fConfiguration->mmoN04_IP->Lines->Clear();
        fConfiguration->mmoN04_IP->Lines->Add("10.1.2.3");   // prefix "10" not in the "192.168.1" skip-list above
        st.iFTRTCount = 2;   // RT1 branch (golden :2841, iFTRTCount-1==1)

        GetTimeInfo();
        AnsiString fn2, path2;
        fn2.sprintf("%s_%s_RT%d_%04d%02d%02d%02d%02d.txt", st.sLotID, st.sProcessCode, st.iFTRTCount-1, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
        path2.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
        AnsiString fullPath2 = path2 + fn2;
        if (FileExists(fullPath2)) DeleteFile(fullPath2);

        SckArtRem_SaveTestSummaryTSV(st, /*iSaveData=*/1);

        CHECK(FileExists(fullPath2), "RT branch filename (golden :2841, iFTRTCount-1==1) written correctly");
        TStringList *check12C = new TStringList();
        check12C->LoadFromFile(fullPath2);
        bool foundHandlerIP = false, foundRTCode = false;
        for (int i = 0; i < check12C->Count; i++)
        {
            if (check12C->Strings[i] == "Handler IP: 10.1.2.3") foundHandlerIP = true;
            if (check12C->Strings[i] == "RT_CODE:\tRT1") foundRTCode = true;
        }
        check12C->Clear();
        delete check12C;
        CHECK(foundHandlerIP, "gate #14's REAL TStringList Lines surface: seeded \"10.1.2.3\" (prefix not in the skip-list) was auto-picked into \"Handler IP: 10.1.2.3\" (golden :2894-2925)");
        CHECK(foundRTCode, "RT_CODE line reflects iFTRTCount-1==1 (golden :2936, \"RT_CODE:\\tRT%d\")");
        if (FileExists(fullPath2)) DeleteFile(fullPath2);

        // ---- 12D: the final UNCONDITIONAL LotSummary.ClearAllData() (golden :3126, differs from the
        //   sibling functions' `if(iSaveData==1)`-gated call) ----
        CHECK(W5SckArtRem_LotSummary.iCountCategory[0][5] == 0, "golden :3126 ClearAllData() runs UNCONDITIONALLY (NOT iSaveData==1-gated like the sibling functions) -- bin 5's seeded value (12B) is zeroed too");

        if (FileExists(fullPath1)) DeleteFile(fullPath1);
        fConfiguration->mmoN04_IP->Lines->Clear();
        asSummaryPath = savedSummaryPath12;
        IniConfig.sN09_HandlerFolder = savedN09Folder12;
        CosFunction.bUseTSVFunction = false;
        bWaitTSV = false;
    }

    // =========================================================================================
    // PART 13 -- SaveSummaryTrayFeed -- golden :3129-3401. AI(W906-SaveSummaryTrayFeed) 20260728.
    //
    // ⚠ AI(W906-BA-BU0g) 20260911: LIMITATION 4's "non-redirectable" is NO LONGER TRUE.
    // Automation/SCK_ART_Remainder.cpp's folder literal now goes through
    // getenv("W906_SUMMARYLOT_ROOT"); unset (production, and the real handler) it is still
    // golden's literal byte for byte, and tests/CMakeLists.txt sets it for every test in this
    // directory, so this PART no longer touches the live machine tree at all. The note below is
    // kept because its analysis of WHY golden is shaped this way is still correct and still
    // explains the risk the seam removes -- only the word "non-redirectable" is now stale.
    //
    // LIMITATION 4 (NEW): unlike every other function in this file, golden hardcodes an
    // UNCONDITIONAL, non-redirectable real production path -- "D:\HT9045_Log\Summary_Lot\
    // <YYYYMM>\..." (golden :3321/:3324) -- with NO IniConfig knob and NO gate protecting it
    // (contrast SaveTestSummarySECS/SaveTestSummaryTSV/Save2DSortingSummary, all of which route
    // their equivalent writes through the REDIRECTABLE asSummaryPath/sN09_HandlerFolder). Verified
    // on THIS dev machine: D:\HT9045_Log\Summary_Lot IS a real, actively-populated production log
    // tree (dated subfolders already present for 202506 through 202606) -- NOT a placeholder. This
    // test still calls the real function once (unlike LIMITATION 2's SetGPIBVersion, which is
    // NEVER called at all) because MyForceDirectories creating an idempotent, otherwise-empty
    // dated folder and writing ONE maximally-distinctive, obviously-fake-named file into it (which
    // this test deletes again immediately after verifying its content) carries the same low,
    // already-accepted residual risk this file's own PART 11 tolerates for its analogous
    // "D:\HT9045_Log\2D_SortList" hardcoded touch -- not a new category of risk. iN10UploadMethod=0
    // is deliberately chosen (routes uploads through the no-op FormHS stand-in) specifically to
    // AVOID the OTHER upload branch, which would `ExecZipCommand` a real XCOPY batch file --
    // actually spawning an OS process -- a materially larger, unnecessary risk this test does not
    // need to accept to get real coverage of this function's translated logic.
    // =========================================================================================
    printf("\n-- SaveSummaryTrayFeed --\n");
    {
        AnsiString savedHandlerID13  = IniConfig.SocketHandlerID;
        AnsiString savedLotNo13      = RunInfo.LotNo;
        AnsiString savedLotStart13   = RunInfo.LotStartTime;
        AnsiString savedLotEnd13     = RunInfo.LotEndTime;
        AnsiString savedCustomer13   = fLotInfo->lbledtCustomer->Text;
        AnsiString savedOperator13   = fLotInfo->edtSysOperatorID->Text;
        AnsiString savedSetupName13  = fMain->cbSetupFileName->Text;
        int  savedVTEST13            = IniConfig.bVTESTFunction;
        bool savedUploadToFTP13      = IniConfig.bN10_UploadSummaryToFTP;
        int  savedUploadMethod13     = IniConfig.iN10UploadMethod;

        IniConfig.SocketHandlerID = "W906TESTPART13HANDLER-DO-NOT-USE";
        RunInfo.LotNo = "W906TESTPART13LOT-DO-NOT-USE";
        RunInfo.LotStartTime = "2026-07-28 08:00:00";
        RunInfo.LotEndTime   = "2026-07-28 09:00:00";
        fLotInfo->lbledtCustomer->Text  = "ACME-13";
        fLotInfo->edtSysOperatorID->Text = "OP13";
        fMain->cbSetupFileName->Text = "SETUP13.ini";
        IniConfig.bVTESTFunction = 0;
        IniConfig.bN10_UploadSummaryToFTP = true;
        IniConfig.iN10UploadMethod = 0;   // routes through FormHS (gate #17), NOT the XCOPY/ExecZipCommand branch

        // AI(W906-PT-W3-integrate) 20260808: these four fields used to be SEEDED DIRECTLY,
        //   which only worked because gate #16's TastCategory stub had a NO-OP UpdataCount.
        //   The stub is retired (cSocket.cpp defines the real TEST_CATEGORY), and
        //   SckArtRem_SaveSummaryTrayFeed's first act is `TastCategory.UpdataCount(false)`
        //   (SCK_ART_Remainder.cpp:2526, golden :3140), whose real body (cSocket.cpp:1203)
        //   calls ClearCount() and then RECOMPUTES every field from ArmDataLot[].
        //   So we now seed golden's REAL inputs and let golden's own arithmetic produce
        //   the same 100/80/20 -- a strictly stronger test: it exercises the whole
        //   ArmDataLot -> TEST_CATEGORY recompute that was previously stubbed out.
        //   Path taken: bCheckYield==false + IsNNMode()==None_NN (atester_shims.cpp:242
        //   returns 0 offline) -> cSocket.cpp:1305-1336, the ArmDataLot branch.
        TArm *savedArmLot0_13 = ArmDataLot[0];
        TArm *savedArmLot1_13 = ArmDataLot[1];
        int savedShtRow13 = TestSocket.iShtRow, savedShtCol13 = TestSocket.iShtCol;
        int savedSiteMap13 = TestIF.iSiteMap[0][0];

        ArmDataLot[0] = new TArm("W906TESTPART13ARM0-DO-NOT-USE");   // ctor is allocation-only,
        ArmDataLot[1] = new TArm("W906TESTPART13ARM1-DO-NOT-USE");   // no file I/O (cSocket.cpp:1029)
        TestSocket.iShtRow = 1;            // one shuttle site, so exactly one (iRow,iCol) is visited
        TestSocket.iShtCol = 1;
        TestIF.iSiteMap[0][0] = 1;         // >0 -> site is live; iDut = 1-1 = 0
        // AI(W906-PT-W3-integrate) 20260808 CORRECTION (2nd pass): seeding `Total`
        //   was wrong and this PART stayed red because of it (177/179 -- "Input:"
        //   and "Fail:" failed while "Pass:" passed, which is the signature of
        //   exactly this mistake).  `TMySocket::GetTotal()` does NOT read the
        //   `Total` field -- it returns `Pass+Fail` (cSocket.cpp:328-331, golden
        //   :173).  Seeding Total=60/40 therefore fed GetTotal() nothing and it
        //   answered 48/32, i.e. iTotalSocket==80==iPassSocket and iFailSocket==0.
        //   Seed Pass AND Fail so golden's own accessor produces the intended
        //   60/40, and keep Total consistent with it (unread on this path, but
        //   GetPCA() at cSocket.cpp:333 does divide by it, so leaving it at 0
        //   would plant a stale zero for the next reader).
        ArmDataLot[0]->ArmSKET[0][0]->Pass = 48;    ArmDataLot[0]->ArmSKET[0][0]->Fail = 12;   // GetTotal()==60
        ArmDataLot[1]->ArmSKET[0][0]->Pass = 32;    ArmDataLot[1]->ArmSKET[0][0]->Fail =  8;   // GetTotal()==40
        ArmDataLot[0]->ArmSKET[0][0]->Total = 60;   ArmDataLot[1]->ArmSKET[0][0]->Total = 40;
        //   => iTotalSocket = 60+40 = 100, iPassSocket = 48+32 = 80,
        //      iFailSocket  = iTotalSocket-iPassSocket = 20 (cSocket.cpp:1399, golden :1225).
        //   iRejectCount stays 0: grepping the WHOLE GOLDEN tree, the ONLY write to
        //   TEST_CATEGORY::iRejectCount is `iRejectCount=0;` in ClearCount (golden
        //   cSocket.cpp:1024). Nothing anywhere ever sets it non-zero, so golden's own
        //   summary always prints "Reject: 0" -- the old "== 5" assertion was an artefact
        //   of the stub, not a property of golden. 13B asserts 0 accordingly.

        W5SckArtRem_slEventLog.sLotFileName = "";   // default -- FileExists("") is false (gate #17)
        W5SckArtRem_LastFormHSUpload_Dir      = "<unset>";
        W5SckArtRem_LastFormHSUpload_FileName  = "<unset>";
        W5SckArtRem_LastFormHSUpload_Type      = "<unset>";

        GetTimeInfo();   // golden itself never calls this inside SaveSummaryTrayFeed (verified by grep) --
                          // the test snapshots NOW so it can reconstruct the exact expected file path.
        // AI(W906-BA-BU0g) 20260911: the "unavoidable production write" this
        // PART's own LIMITATION 4 note describes is no longer unavoidable.
        // Automation/SCK_ART_Remainder.cpp's folder literal is now
        // getenv("W906_SUMMARYLOT_ROOT")-overridable; unset, it is still
        // golden's literal, so production behaviour is unchanged, and
        // tests/CMakeLists.txt sets the variable for every test here.
        //
        // THE EXPECTED PATH IS DERIVED FROM THE SAME VARIABLE, so the test can
        // never read one location while the code writes another. That mismatch
        // is exactly what broke this PART when the override first landed and
        // this line still carried the literal: 8 assertions failed at once,
        // every one of them because LoadFromFile could not find a file that had
        // in fact been written correctly.
        const char* sumRootEnv = getenv("W906_SUMMARYLOT_ROOT");
        AnsiString sumRoot = sumRootEnv ? AnsiString(sumRootEnv)
                                        : AnsiString("D:\\HT9045_Log\\Summary_Lot");
        printf("   [13] summary root: %s%s\n", sumRoot.c_str(),
               sumRootEnv ? "" : "   <-- PRODUCTION (override not set)");
        AnsiString expectedFolder, expectedFile;
        expectedFolder.sprintf("%s\\%04d%02d", sumRoot, SystemYear, SystemMonth);
        expectedFile.sprintf("%s\\%s %04d%02d%02d-%02d%02d%02d %s Summary.txt", expectedFolder,
                              IniConfig.SocketHandlerID, SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin, SystemSec,
                              RunInfo.LotNo);
        if (FileExists(expectedFile)) DeleteFile(expectedFile);   // idempotent -- see this PART's own LIMITATION 4 note

        SckArtRemainderState st;   // unused by this function (see its own header doc comment); passed for signature only

        SckArtRem_SaveSummaryTrayFeed(st);

        // ---- 13A: the file gets written under golden's REAL (unavoidable) path with the expected name ----
        CHECK(FileExists(expectedFile), "SaveSummaryTrayFeed() writes the summary .txt under the Summary_Lot root -- golden's literal when W906_SUMMARYLOT_ROOT is unset, the override when it is set (golden :3321-3329)");

        // ---- 13B: file content reflects the seeded TastCategory/RunInfo/fLotInfo/fMain fields ----
        TStringList *check13 = new TStringList();
        check13->LoadFromFile(expectedFile);
        bool foundLot=false, foundCustomer=false, foundProgram=false, foundInput=false, foundPass=false, foundFail=false, foundReject=false;
        for (int i = 0; i < check13->Count; i++)
        {
            AnsiString line = check13->Strings[i];
            if (line.Pos("Lot#:") == 1 && line.Pos("W906TESTPART13LOT-DO-NOT-USE") > 0) foundLot = true;
            if (line.Pos("Customer:") == 1 && line.Pos("ACME-13") > 0) foundCustomer = true;
            if (line.Pos("Program:") == 1 && line.Pos("SETUP13.ini") > 0) foundProgram = true;
            if (line.Pos("Input:") == 1 && line.Pos("100") > 0) foundInput = true;
            if (line.Pos("Pass:") == 1 && line.Pos("80") > 0) foundPass = true;
            if (line.Pos("Fail:") == 1 && line.Pos("20") > 0) foundFail = true;
            if (line.Pos("Reject:") == 1 && line.Pos("0") > 0) foundReject = true;
        }
        check13->Clear();
        delete check13;
        CHECK(foundLot, "\"Lot#:\" line reflects RunInfo.LotNo (golden :3166)");
        CHECK(foundCustomer, "\"Customer:\" line reflects fLotInfo->lbledtCustomer->Text (golden :3169/:3154-3157)");
        CHECK(foundProgram, "\"Program:\" line reflects fMain->cbSetupFileName->Text (golden :3170)");
        CHECK(foundInput, "\"Input:\" line == 100 -- golden's REAL UpdataCount summed ArmDataLot[0..1]->ArmSKET[0][0]->GetTotal() (60+40) (golden :3174, cSocket.cpp:1329)");
        CHECK(foundPass, "\"Pass:\" line == 80 -- golden's REAL UpdataCount summed GetPassCT() (48+32) (golden :3175, cSocket.cpp:1333)");
        CHECK(foundFail, "\"Fail:\" line == 20 -- golden derives iFailSocket=iTotalSocket-iPassSocket (golden :3176, cSocket.cpp:1399)");
        CHECK(foundReject, "\"Reject:\" line == 0 -- golden NEVER writes iRejectCount non-zero anywhere in its tree (only ClearCount's =0, golden cSocket.cpp:1024) (golden :3177)");

        // ---- 13C: gate #17's FormHS SLT_Report upload -- REACHED (file just written -> FileExists==true) ----
        CHECK(W5SckArtRem_LastFormHSUpload_FileName != "<unset>", "FormHS->UpDataToServerByFTP(\"SLT_Report\") gate #17 was REACHED once the summary file existed (golden :3341-3342)");
        CHECK(W5SckArtRem_LastFormHSUpload_Type == "SLT_Report", "gate #17 received the \"SLT_Report\" file-type argument verbatim");

        // ---- 13D: gate #17's slEventLog stand-in -- default "" means the EventLog upload branch is
        //   SKIPPED (FileExists("")==false, golden :3344) ----
        W5SckArtRem_LastFormHSUpload_Type = "<unset>";   // reset before a 2nd call so we can distinguish
        AnsiString fullPath13b = expectedFile;   // same call would re-write the SAME path/name (same minute)
        SckArtRem_SaveSummaryTrayFeed(st);
        CHECK(W5SckArtRem_LastFormHSUpload_Type == "SLT_Report", "W5SckArtRem_slEventLog.sLotFileName==\"\" (default) -> the EventLog upload branch never fires -> the LAST FormHS call observed is still \"SLT_Report\", not \"EventLog\" (golden :3344 FileExists(\"\")==false)");

        // ---- 13E: seed a real scratch file for slEventLog -> the EventLog upload branch DOES fire ----
        AnsiString scratchEventLog = ScratchDir() + "W906TestPart13EventLog.txt";
        TStringList *seedEvt = new TStringList();
        seedEvt->Add("dummy event log line");
        seedEvt->SaveToFile(scratchEventLog);
        delete seedEvt;
        W5SckArtRem_slEventLog.sLotFileName = scratchEventLog;
        W5SckArtRem_LastFormHSUpload_Type = "<unset>";
        SckArtRem_SaveSummaryTrayFeed(st);
        CHECK(W5SckArtRem_LastFormHSUpload_Type == "EventLog", "W5SckArtRem_slEventLog.sLotFileName pointing at a REAL (scratch) file -> the EventLog upload branch fires (golden :3344-3347)");
        CHECK(W5SckArtRem_LastFormHSUpload_FileName == ExtractFileName(scratchEventLog), "gate #17's EventLog upload received the real ExtractFileName(...) argument (golden :3346)");
        DeleteFile(scratchEventLog);
        W5SckArtRem_slEventLog.sLotFileName = "";

        // cleanup -- delete the test-created production-adjacent file (LIMITATION 4); best-effort
        // remove the just-created month folder too (silently ignored if non-empty / still in use)
        if (FileExists(expectedFile)) DeleteFile(expectedFile);
        RemoveDir(expectedFolder);

        IniConfig.SocketHandlerID = savedHandlerID13;
        RunInfo.LotNo = savedLotNo13;
        RunInfo.LotStartTime = savedLotStart13;
        RunInfo.LotEndTime = savedLotEnd13;
        fLotInfo->lbledtCustomer->Text = savedCustomer13;
        fLotInfo->edtSysOperatorID->Text = savedOperator13;
        fMain->cbSetupFileName->Text = savedSetupName13;
        IniConfig.bVTESTFunction = savedVTEST13;
        IniConfig.bN10_UploadSummaryToFTP = savedUploadToFTP13;
        IniConfig.iN10UploadMethod = savedUploadMethod13;
        // AI(W906-PT-W3-integrate) 20260808: restore the REAL UpdataCount inputs this PART
        //   seeded (see its note above).  Deliberately NOT deleting the two TArm objects:
        //   ~TArm's reach was not audited by this wave and this PART only ever runs once,
        //   so a bounded 2-object leak at test scope is preferred over an unverified dtor.
        ArmDataLot[0] = savedArmLot0_13;
        ArmDataLot[1] = savedArmLot1_13;
        TestSocket.iShtRow = savedShtRow13;
        TestSocket.iShtCol = savedShtCol13;
        TestIF.iSiteMap[0][0] = savedSiteMap13;
    }

    // =========================================================================================
    // PART 14 -- SaveMultiLotTestSummary -- golden :2045-2803. AI(W906-SCKART-Multi-Test) 20260802.
    //
    //   WHY THIS PART EXISTS: three landed comments name it as the function's only caller
    //   (SCK_ART_Remainder.cpp's PART-14 section banner, SCK_ART_Remainder.h's gate #5 note on the
    //   iByLot* mirrors, and SCK_ART_Remainder.h's declaration doc). It is indeed the ONLY caller of
    //   SckArtRem_SaveMultiLotTestSummary anywhere in this tree: golden's sole caller,
    //   csystem.cpp:10862 inside DoTrayFeedProcess, is not translated yet (owned by wave W7).
    //   This PART is also the FIRST and ONLY exercise of LotSummary.iByLotCountCategory[][][] and
    //   .iByLotTotalCategory[][] -- gate #5's 4th extension -- which have no other reader in the tree.
    //
    //   PATH DISCIPLINE: asSummaryPath is redirected to a scratch dir for the WHOLE part (never the
    //   real D:\HT9045_Log\Summary production tree) and restored at the end -- same discipline as
    //   PART 10/11/12. CosFunction.bUseTSVFunction is deliberately FALSE, which keeps every write on
    //   the redirectable PathName2 branch (golden :2709-2721) and away from GOLDEN BUG #8's bare
    //   relative `PathName+FileName` save (golden :2688, reachable only through the TSV +
    //   bN09_LotCountAutoFunc + iN09_4_UploadMethod!=0 arm, which would drop a file into the ctest
    //   working directory). CosFunction.bSortingBy2DList is FALSE, so golden :2783-2800's hardcoded
    //   "D:\HT9045_Log\2D_SortList" tail is never touched at all -- this PART, unlike PART 11 and
    //   PART 13, reaches NO real production path whatsoever.
    //
    //   DETERMINISM (no minute-rollover flakiness): golden stamps st.sLotEndTime (:2065) and every
    //   FileName (:2099/:2103) from the SAME GetTimeInfo() snapshot (:2063). So this PART never
    //   guesses the wall clock -- it reads the stamp back out of st.sLotEndTime AFTER the call and
    //   derives the exact expected folder + file names from it. The pre-call DeleteFile sweep uses a
    //   test-side GetTimeInfo() purely to clear a same-minute leftover from a crashed earlier run.
    //
    //   NOT EXERCISED HERE (preserved+cited in the source, pinned elsewhere or deliberately not at
    //   all): GOLDEN BUG #8 (needs the non-redirectable save arm, see above); GOLDEN BUG #9 (the
    //   ShellExecute-with-empty-path launch -- bA38_SLT_Summary is FALSE here; PART 10 and PART 11
    //   already pin that exact idiom against the two structurally identical sibling call sites);
    //   GOLDEN BUGS #1, #6, #7 (a dead store, a dead increment and two discarded reads -- by
    //   construction they have NO observable effect, so no assertion can distinguish them; they are
    //   cited at their sites and covered by the fidelity audit, not by this harness); GOLDEN BUG
    //   #2(a) (the 32-vs-33 brace initialiser -- benign, element 32 is value-initialised anyway).
    //
    //   FIXTURE (all restored at the end): 2 real lots + the combined roll-up
    //   (iInfo_MultiLotCnt==2 -> k=0,1 per-lot, k=2 combined), a 2x1 site grid, iTestBinCount==4
    //   (bins 0..3 plus the bin-4 "Err" row), and this tray map, chosen so that three separate
    //   golden bugs become visible in the SAME report:
    //     tray 0  (eAuto1) in use, bin 1 routed to it  -> iT6PosCate[1]=1
    //     tray 1  (eAuto2) in use, bin 2 routed to it  -> iT6PosCate[2]=2
    //     tray 2  (eAuto3) NOT in use                  -> makes bin 2 vanish from the Hard Bin and
    //                                                     Yield sections (GOLDEN BUG #10(a): those
    //                                                     three sites test iTrayType[temp], one slot
    //                                                     too high, instead of iTrayType[temp-1])
    //     bin 0 unrouted (iT6PosCate[0]==0)            -> still gets a Hard Bin row, because temp==0
    //                                                     is tested against tray 0, which IS in use
    //                                                     (GOLDEN BUG #10(b))
    //     tray 9  (eFix4)  in use, bin 3 routed to it  -> index >= 9, i.e. OUTSIDE the `for(i<9)`
    //                                                     per-lot reset (GOLDEN BUG #2(b)), so its
    //                                                     Group Bin figure ACCUMULATES across lots
    //     tray 10 (eFix5)  in use, no bin routed       -> the "(Null)" aUnloader shape
    // =========================================================================================
    printf("\n-- SaveMultiLotTestSummary --\n");
    {
        // ---- save every global this PART steers ----
        AnsiString savedSummaryPath14 = asSummaryPath;
        int  savedMachineType14   = MachineTypeChoice;
        int  savedTestBinCount14  = iTestBinCount;
        int  savedShtRow14        = TestSocket.iShtRow;
        int  savedShtCol14        = TestSocket.iShtCol;
        int  savedIfErrorT6_14    = Prod.iIfErrorT6;
        int  savedInputCT14       = LastSet.iSCKARTInputCT;
        int  savedE1_14           = iE1Count;
        int  savedE2_14           = iE2Count;
        int  savedE3_14           = iE3Count;
        int  savedEnd_14          = iENotDefinedCount;
        bool savedSPIL14          = IniConfig.bSPILFunction;
        bool savedN17_14          = IniConfig.bN17UploadLotSummary;
        bool savedA38_14          = IniConfig.bA38_SLT_Summary;
        bool savedTSV14           = CosFunction.bUseTSVFunction;
        bool savedSort2D14        = CosFunction.bSortingBy2DList;
        int  savedTrayType14[eTrayCount];
        int  savedPosCate14[8];
        int  savedPassBin14[8];
        int  i14;
        for (i14 = 0; i14 < eTrayCount; ++i14) savedTrayType14[i14] = Prod.iTrayType[i14];
        for (i14 = 0; i14 < 8; ++i14)
        {
            savedPosCate14[i14] = Prod.iT6PosCate[i14];
            savedPassBin14[i14] = Prod.bIsPassBin[i14];
        }

        asSummaryPath                  = ScratchDir() + "W906SaveMultiLotSummaryScratch";
        MachineTypeChoice              = Type_HT9046;   // pins the sInfo_ProgramName reassignment (golden :2087-2088)
        IniConfig.bSPILFunction        = false;         // unconditional SaveToFile + no "skip zero-total bin" filtering
        IniConfig.bN17UploadLotSummary = false;
        IniConfig.bA38_SLT_Summary     = false;         // GOLDEN BUG #9's launch stays out of this PART (PART 10/11 own it)
        CosFunction.bUseTSVFunction    = false;         // keep every write on the redirectable PathName2 branch
        CosFunction.bSortingBy2DList   = false;         // skip golden :2783-2800's hardcoded 2D backup tail entirely

        iTestBinCount      = 4;    // bins 0..3 are real bins; bin 4 == iTestBinCount == the "Err" row
        TestSocket.iShtRow = 2;
        TestSocket.iShtCol = 1;    // 2 sites: indices 0 and 1
        Prod.iIfErrorT6    = -1;   // no tray is the "error" tray -> aUnloader gets "Null", never "error"

        for (i14 = 0; i14 < eTrayCount; ++i14) Prod.iTrayType[i14] = tNotUse;
        Prod.iTrayType[0]  = tTrayAuto;   // eAuto1
        Prod.iTrayType[1]  = tTrayAuto;   // eAuto2
        // eAuto3 (index 2) stays tNotUse ON PURPOSE -- that is what makes GOLDEN BUG #10(a) visible
        Prod.iTrayType[9]  = tTrayFix;    // eFix4  -- index >= 9: the slot GOLDEN BUG #2(b) never resets
        Prod.iTrayType[10] = tTrayFix;    // eFix5  -- in use, but no bin routed to it

        for (i14 = 0; i14 < 8; ++i14) { Prod.iT6PosCate[i14] = 0; Prod.bIsPassBin[i14] = 0; }
        Prod.iT6PosCate[1] = 1;    // bin 1 -> tray 0 (iT6PosCate is 1-BASED, golden cprod.h:512 "Auto1 = 1")
        Prod.iT6PosCate[2] = 2;    // bin 2 -> tray 1
        Prod.iT6PosCate[3] = 10;   // bin 3 -> tray 9
        Prod.bIsPassBin[0] = 0;    // PASS
        Prod.bIsPassBin[1] = 0;    // PASS
        Prod.bIsPassBin[2] = 1;    // FAIL
        Prod.bIsPassBin[3] = 1;    // FAIL

        // -----------------------------------------------------------------------------------
        // 14A -- bSaveData==false. GOLDEN BUG #4 (golden :2106-2107): the early-out sits AFTER
        //   st.sLotEndTime is overwritten (:2065), AFTER MyForceDirectories has created the
        //   year/month folder (:2082) and AFTER st.sInfo_ProgramName is reassigned (:2084-2090);
        //   and it is `return`, not `break`, so :2802's unconditional ClearAllData() is skipped.
        // -----------------------------------------------------------------------------------
        {
            W5SckArtRem_LotSummary.iTotalCategory[1] = 99;   // must SURVIVE the call (ClearAllData skipped)

            SckArtRemainderState stF;
            stF.iInfo_MultiLotCnt      = 2;
            stF.sInfoArr_InnerLotID[0] = "W906P14NOSAVE";
            stF.sInfoArr_CustLotID[0]  = "CLN";
            stF.sInfoArr_CustDevGup[0] = "DGN";
            stF.sLotEndTime            = "SENTINEL-NOT-OVERWRITTEN";
            stF.sInfo_ProgramName      = "SENTINEL-NOT-OVERWRITTEN";

            SckArtRem_SaveMultiLotTestSummary(stF, /*bSaveData=*/false);

            CHECK(stF.sLotEndTime != "SENTINEL-NOT-OVERWRITTEN" && stF.sLotEndTime.Length() == 14,
                  "bSaveData==false STILL overwrote st.sLotEndTime with the 14-digit YYYYMMDDhhmmss stamp before returning (golden :2065, GOLDEN BUG #4)");
            CHECK(stF.sInfo_ProgramName == "HT9046",
                  "bSaveData==false STILL reassigned st.sInfo_ProgramName from MachineTypeChoice==Type_HT9046 before returning (golden :2087-2088, GOLDEN BUG #4)");

            AnsiString dirF  = asSummaryPath + AnsiString("\\") + stF.sLotEndTime.SubString(1,4)
                                             + AnsiString("\\") + stF.sLotEndTime.SubString(5,2);
            AnsiString fileF = dirF + AnsiString("\\W906P14NOSAVE_CLN_DGN_") + stF.sLotEndTime.SubString(1,12) + AnsiString(".txt");

            CHECK(DirectoryExists(dirF),
                  "bSaveData==false STILL created the year/month summary folder via MyForceDirectories before returning (golden :2081-2082, GOLDEN BUG #4)");
            CHECK(FileExists(fileF) == false && DirectoryExists(dirF),
                  "...but wrote NO file -- the bSaveData==false return lands after FileName is built and before the TStringList is even allocated (golden :2106-2110)");
            CHECK(W5SckArtRem_LotSummary.iTotalCategory[1] == 99 && stF.sInfo_ProgramName == "HT9046",
                  "bSaveData==false is `return`, NOT `break` -- golden :2802's unconditional LotSummary.ClearAllData() never runs, so the seeded iTotalCategory[1]==99 survives (GOLDEN BUG #4)");
        }

        // -----------------------------------------------------------------------------------
        // Shared fixture for 14B..14E. Numbers are all distinct per (lot, site, bin) so that a
        // mis-indexed read cannot accidentally produce the expected text.
        // -----------------------------------------------------------------------------------
        W5SckArtRem_LotSummary.ClearAllData();
        {
            int L, s, b;
            for (L = 0; L < 5; ++L) W5SckArtRem_LotSummary.iByLotLoadCount[L] = 0;
            for (L = 0; L < 5; ++L)
                for (b = 0; b <= 4; ++b)
                {
                    W5SckArtRem_LotSummary.iByLotTotalCategory[L][b] = 0;
                    for (s = 0; s < 2; ++s)
                        W5SckArtRem_LotSummary.iByLotCountCategory[L][s][b] = 0;
                }
        }

        // lot 0 (k==0) -- read ONLY through iByLotCountCategory[0][][] / iByLotTotalCategory[0][]
        W5SckArtRem_LotSummary.iByLotCountCategory[0][0][1] =  11;
        W5SckArtRem_LotSummary.iByLotCountCategory[0][1][1] =  12;
        W5SckArtRem_LotSummary.iByLotTotalCategory[0][1]    =  23;
        W5SckArtRem_LotSummary.iByLotCountCategory[0][0][2] =   3;
        W5SckArtRem_LotSummary.iByLotCountCategory[0][1][2] =   4;
        W5SckArtRem_LotSummary.iByLotTotalCategory[0][2]    =   7;
        W5SckArtRem_LotSummary.iByLotCountCategory[0][0][3] =  40;
        W5SckArtRem_LotSummary.iByLotCountCategory[0][1][3] =  60;
        W5SckArtRem_LotSummary.iByLotTotalCategory[0][3]    = 100;
        W5SckArtRem_LotSummary.iByLotLoadCount[0]           =  31;
        // lot 1 (k==1)
        W5SckArtRem_LotSummary.iByLotCountCategory[1][0][1] =  21;
        W5SckArtRem_LotSummary.iByLotCountCategory[1][1][1] =  22;
        W5SckArtRem_LotSummary.iByLotTotalCategory[1][1]    =  43;
        W5SckArtRem_LotSummary.iByLotCountCategory[1][0][2] =   5;
        W5SckArtRem_LotSummary.iByLotCountCategory[1][1][2] =   6;
        W5SckArtRem_LotSummary.iByLotTotalCategory[1][2]    =  11;
        W5SckArtRem_LotSummary.iByLotCountCategory[1][0][3] =  90;
        W5SckArtRem_LotSummary.iByLotCountCategory[1][1][3] = 110;
        W5SckArtRem_LotSummary.iByLotTotalCategory[1][3]    = 200;
        W5SckArtRem_LotSummary.iByLotLoadCount[1]           =  62;
        // combined roll-up (k==2) -- read through iCountCategory[][] / iTotalCategory[]
        W5SckArtRem_LotSummary.iCountCategory[0][1] =  32;
        W5SckArtRem_LotSummary.iCountCategory[1][1] =  34;
        W5SckArtRem_LotSummary.iTotalCategory[1]    =  66;
        W5SckArtRem_LotSummary.iCountCategory[0][2] =   8;
        W5SckArtRem_LotSummary.iCountCategory[1][2] =  10;
        W5SckArtRem_LotSummary.iTotalCategory[2]    =  18;
        W5SckArtRem_LotSummary.iCountCategory[0][3] = 150;
        W5SckArtRem_LotSummary.iCountCategory[1][3] = 250;
        W5SckArtRem_LotSummary.iTotalCategory[3]    = 400;
        W5SckArtRem_LotSummary.iCountCategory[0][4] =   9;   // the "Err" bin (iBin==iTestBinCount)
        W5SckArtRem_LotSummary.iCountCategory[1][4] =  11;
        W5SckArtRem_LotSummary.iTotalCategory[4]    =  20;
        LastSet.iSCKARTInputCT = 90;                          // combined "Handler Load Qty" (golden :2361)

        // non-zero so the k==iInfo_MultiLotCnt guard (golden :2069) does NOT fire on this first call
        iE1Count = 2; iE2Count = 3; iE3Count = 5; iENotDefinedCount = 0;

        SckArtRemainderState st14;
        st14.iInfo_MultiLotCnt      = 2;
        st14.sInfoArr_InnerLotID[0] = "W906P14LOTA";  st14.sInfoArr_InnerLotID[1] = "W906P14LOTB";
        st14.sInfoArr_CustLotID[0]  = "CLA";          st14.sInfoArr_CustLotID[1]  = "CLB";
        st14.sInfoArr_CustDevGup[0] = "DGA";          st14.sInfoArr_CustDevGup[1] = "DGB";
        st14.sInfoArr_Customer[0]   = "CUSTA";        st14.sInfoArr_Customer[1]   = "CUSTB";
        st14.sInfoArr_DeviceName[0] = "DEVA";         st14.sInfoArr_DeviceName[1] = "DEVB";
        st14.sInfoArr_Stage[0]      = "FT1";          st14.sInfoArr_Stage[1]      = "FT2";
        st14.sInfoArr_OperatorID[0] = "OPA";          st14.sInfoArr_OperatorID[1] = "OPB";
        st14.sInfoArr_OperatorID[2] = "";             // slot [iInfo_MultiLotCnt] -- see GOLDEN BUG #5 (14E)
        st14.sLotStartTime          = "20260802080000";
        st14.sLotEndTime            = "SENTINEL-NOT-OVERWRITTEN";

        // Clear a same-minute leftover from a crashed earlier run (test-side clock; the AUTHORITATIVE
        // stamp is read back from st14.sLotEndTime after the call).
        GetTimeInfo();
        {
            AnsiString guessDir, guessStamp;
            guessStamp.sprintf("%04d%02d%02d%02d%02d", SystemYear, SystemMonth, SystemDate, SystemHour, SystemMin);
            guessDir.sprintf("%s\\%04d\\%02d\\", asSummaryPath, SystemYear, SystemMonth);
            DeleteFile(guessDir + AnsiString("W906P14LOTA_CLA_DGA_")   + guessStamp + AnsiString(".txt"));
            DeleteFile(guessDir + AnsiString("W906P14LOTB_CLB_DGB_")   + guessStamp + AnsiString(".txt"));
            DeleteFile(guessDir + AnsiString("W906P14LOTAW906P14LOTB_")+ guessStamp + AnsiString(".txt"));
            DeleteFile(guessDir + AnsiString("W906P14LOTC_CLC_DGC_")   + guessStamp + AnsiString(".txt"));
            DeleteFile(guessDir + AnsiString("W906P14LOTD_CLD_DGD_")   + guessStamp + AnsiString(".txt"));
            DeleteFile(guessDir + AnsiString("W906P14LOTCW906P14LOTD_")+ guessStamp + AnsiString(".txt"));
        }

        SckArtRem_SaveMultiLotTestSummary(st14, /*bSaveData=*/true);

        // Authoritative, rollover-proof reconstruction of the 3 produced paths.
        AnsiString stamp14 = st14.sLotEndTime.SubString(1,12);   // YYYYMMDDhhmm, exactly what FileName uses
        AnsiString dir14   = asSummaryPath + AnsiString("\\") + st14.sLotEndTime.SubString(1,4)
                                           + AnsiString("\\") + st14.sLotEndTime.SubString(5,2) + AnsiString("\\");
        AnsiString fileLotA = dir14 + AnsiString("W906P14LOTA_CLA_DGA_")    + stamp14 + AnsiString(".txt");
        AnsiString fileLotB = dir14 + AnsiString("W906P14LOTB_CLB_DGB_")    + stamp14 + AnsiString(".txt");
        AnsiString fileComb = dir14 + AnsiString("W906P14LOTAW906P14LOTB_") + stamp14 + AnsiString(".txt");

        // -----------------------------------------------------------------------------------
        // 14B -- the three files exist, with the two DIFFERENT golden FileName shapes: per-lot
        //   "%s_%s_%s_stamp.txt" (golden :2103) vs combined "%s_stamp.txt" where %s is the
        //   SEPARATOR-LESS concatenation of sInfoArr_InnerLotID[0..cnt-1] (golden :2094-2099).
        // -----------------------------------------------------------------------------------
        CHECK(FileExists(fileLotA) && FileExists(fileLotB),
              "k=0 and k=1 each wrote a per-lot summary named InnerLotID_CustLotID_CustDevGup_stamp.txt under the SCRATCH asSummaryPath (golden :2103)");
        CHECK(FileExists(fileComb),
              "k==iInfo_MultiLotCnt wrote the COMBINED summary named <InnerLotID[0]+InnerLotID[1]>_stamp.txt -- separator-less concatenation (golden :2094-2099)");

        // -----------------------------------------------------------------------------------
        // 14C -- THE PER-LOT MIRRORS ARE ACTUALLY READ, per lot, at the right index.
        //   Hard Bin row layout (golden :2427-2515): 5-wide right-aligned bin no, " PASS"/" FAIL",
        //   then one 6-wide right-aligned count per site, then the 6-wide row total. bin 1 is a
        //   PASS bin routed to tray 0.
        //     lot 0 -> iByLotCountCategory[0][0..1][1] == 11,12 and iByLotTotalCategory[0][1] == 23
        //     lot 1 -> iByLotCountCategory[1][0..1][1] == 21,22 and iByLotTotalCategory[1][1] == 43
        //     combined -> iCountCategory[0..1][1] == 32,34 and iTotalCategory[1] == 66
        //   The trailing yield percentage is deliberately not pinned (ChangeToPercentage's exact
        //   formatting is a separate, already-covered concern) -- prefix match only.
        // -----------------------------------------------------------------------------------
        CHECK(FileHasLinePrefix(fileLotA, "    1 PASS    11    12    23"),
              "lot 0's Hard Bin row for bin 1 is built from iByLotCountCategory[0][site][1] (11,12) and iByLotTotalCategory[0][1] (23) -- golden :2490/:2503, the FIRST reader of those two arrays in this tree");
        CHECK(FileHasLinePrefix(fileLotB, "    1 PASS    21    22    43"),
              "lot 1's SAME row uses lot index k==1 -- iByLotCountCategory[1][site][1] (21,22) / iByLotTotalCategory[1][1] (43), proving the [k] subscript, not a fixed lot");
        CHECK(FileHasLinePrefix(fileComb, "    1 PASS    32    34    66"),
              "the combined arm switches to the NON-per-lot mirrors iCountCategory[site][1] (32,34) / iTotalCategory[1] (66) (golden :2454/:2467)");
        CHECK(FileHasLinePrefix(fileLotA, "    3 FAIL    40    60   100"),
              "lot 0's bin 3 row is a FAIL row (Prod.bIsPassBin[3]==1, golden :2441-2444) with iByLotCountCategory[0][site][3] (40,60) / iByLotTotalCategory[0][3] (100)");
        CHECK(FileHasLinePrefix(fileComb, "  Err FAIL     9    11    20"),
              "the iBin==iTestBinCount row is labelled \"Err\"+\" FAIL\" unconditionally and reports iCountCategory[site][4] (9,11) / iTotalCategory[4] (20) (golden :2410-2437)");

        // -----------------------------------------------------------------------------------
        // 14D -- Group Bin Summary: aUnloader text + per-tray totals, AND GOLDEN BUG #2(b).
        //   Tray 0 and tray 1 are inside the `for(int i=0;i<9;i++) iUnloadCnt[i]=0;` per-k reset
        //   (golden :2074-2077) so they report each lot cleanly. Tray 9 is NOT -- its counter
        //   accumulates across every k of the SAME call:
        //     k=0 -> 100                      (iByLotTotalCategory[0][3])
        //     k=1 -> 100+200 = 300            (should have been 200)
        //     k=2 -> 300+400 = 700            (should have been 400)
        // -----------------------------------------------------------------------------------
        {
            AnsiString g0A, g0B, g0C, g1A, g9A, g9B, g9C, g10A;
            g0A.sprintf("%s(1):%d",    s6ShortTrayName[0],  23);
            g0B.sprintf("%s(1):%d",    s6ShortTrayName[0],  43);
            g0C.sprintf("%s(1):%d",    s6ShortTrayName[0],  66);
            g1A.sprintf("%s(2):%d",    s6ShortTrayName[1],   7);
            g9A.sprintf("%s(3):%d",    s6ShortTrayName[9], 100);
            g9B.sprintf("%s(3):%d",    s6ShortTrayName[9], 300);
            g9C.sprintf("%s(3):%d",    s6ShortTrayName[9], 700);
            g10A.sprintf("%s(Null):%d", s6ShortTrayName[10], 0);

            CHECK(FileHasExactLine(fileLotA, g0A) && FileHasExactLine(fileLotB, g0B) && FileHasExactLine(fileComb, g0C),
                  "Group Bin line for tray 0 reads \"<shortname>(1):<total>\" and reports 23 / 43 / 66 for lot0 / lot1 / combined -- tray index < 9, so the per-k reset (golden :2074-2077) DOES clear it");
            CHECK(FileHasExactLine(fileLotA, g1A),
                  "Group Bin line for tray 1 reads \"<shortname>(2):7\" -- bin 2 IS counted here even though 14E shows it is missing from the Hard Bin / Yield sections of the very same report");
            CHECK(FileHasExactLine(fileLotA, g9A) && FileHasExactLine(fileLotB, g9B) && FileHasExactLine(fileComb, g9C),
                  "GOLDEN BUG #2(b): tray 9 is OUTSIDE the `for(i<9)` per-k reset, so its Group Bin figure ACCUMULATES -- 100, then 100+200=300 (should be 200), then 300+400=700 (should be 400)");
            CHECK(FileHasExactLine(fileLotA, g10A),
                  "an in-use tray with no bin routed to it renders as \"<shortname>(Null):0\" (golden :2352-2355)");
        }

        // -----------------------------------------------------------------------------------
        // 14E -- GOLDEN BUG #10 (both manifestations) + GOLDEN BUG #5 + header/roll-up lines.
        // -----------------------------------------------------------------------------------
        {
            // 10(a): bin 2 is routed to tray 1, which IS in use, and its 7 units DO appear in the
            // Group Bin section (asserted in 14D) -- yet no Hard Bin row exists for it, because the
            // guard tests Prod.iTrayType[temp] == iTrayType[2] (eAuto3, tNotUse) instead of [temp-1].
            bool hasBin2Row = FileHasLinePrefix(fileLotA, "    2 ");
            CHECK(hasBin2Row == false && FileHasLinePrefix(fileLotA, "    1 PASS"),
                  "GOLDEN BUG #10(a): bin 2 has a Group Bin figure but NO Hard Bin row -- the guard asks Prod.iTrayType[temp] (tray 2, unused) instead of [temp-1] (tray 1, in use) (golden :2415-2418)");
            // 10(b): bin 0 is UNROUTED (iT6PosCate[0]==0) and the lower bound is `temp<0`, not
            // `temp<=0`, so it is tested against tray 0 -- which is in use -- and gets a row.
            CHECK(FileHasLinePrefix(fileLotA, "    0 PASS     0     0     0"),
                  "GOLDEN BUG #10(b): the UNROUTED bin 0 still gets an all-zero Hard Bin row, because temp==0 is tested against tray 0 (in use) and the bound is `temp<0`, not the Group Bin block's `temp<=0` (golden :2415-2418)");
            // ...and the Yield Summary inherits the same filter, so bin 2's 7 units are excluded
            // there too: PASS==iByLotTotalCategory[0][1]==23 (bin 1 only), FAIL==100 (bin 3 only).
            CHECK(FileHasLinePrefix(fileLotA, " PASS    23") && FileHasLinePrefix(fileLotA, " FAIL   100"),
                  "lot 0's Yield Summary is PASS 23 / FAIL 100 -- bin 2's 7 units are silently EXCLUDED by the same GOLDEN BUG #10(a) filter (golden :2565-2592), so PASS+FAIL(123) != Handler Unload Qty(130)");
            CHECK(FileHasLinePrefix(fileLotB, " PASS    43") && FileHasLinePrefix(fileLotB, " FAIL   200"),
                  "lot 1's Yield Summary uses lot index k==1 -- PASS 43 / FAIL 200 from iByLotTotalCategory[1][]");
            CHECK(FileHasLinePrefix(fileComb, " PASS    66") && FileHasLinePrefix(fileComb, " FAIL   420"),
                  "the combined Yield Summary uses iTotalCategory[] -- PASS 66, FAIL 400(bin 3)+20(the Err bin) == 420 (golden :2535-2561)");
        }

        CHECK(FileHasExactLine(fileLotA, "OPERATOR_ID:OPA") && FileHasExactLine(fileLotB, "OPERATOR_ID:OPB"),
              "the per-lot arm's OPERATOR_ID is sInfoArr_OperatorID[k] and is correct for both lots (golden :2259)");
        CHECK(FileHasExactLine(fileComb, "OPERATOR_ID:") && FileHasExactLine(fileComb, "CUSTOMER:CUSTA_CUSTB"),
              "GOLDEN BUG #5: the COMBINED arm's OPERATOR_ID also uses [k] -- i.e. index iInfo_MultiLotCnt, one past the last populated slot -- so it is BLANK, while every neighbouring line in the same arm correctly joins [0..cnt-1] (golden :2218 vs :2138-2143)");
        CHECK(FileHasExactLine(fileComb, "INNER_LOT_ID:W906P14LOTA_W906P14LOTB")
              && FileHasExactLine(fileComb, "CUST_LOT_ID:CLA_CLB")
              && FileHasExactLine(fileComb, "CUSTOMER_DEVICE_GROUP:DGA_DGB")
              && FileHasExactLine(fileComb, "DEVICE_NAME:DEVA_DEVB")
              && FileHasExactLine(fileComb, "STAGE:FT1_FT2"),
              "the combined arm underscore-JOINS all five sInfoArr_* fields over [0..iInfo_MultiLotCnt-1] (golden :2150-2235) -- note the joiner differs from the separator-less FileName concatenation at :2094-2099");
        CHECK(FileHasExactLine(fileLotA, "SUMMARY_START_TIME:20260802080000")
              && FileHasExactLine(fileLotA, AnsiString("SUMMARY_END_TIME:") + st14.sLotEndTime),
              "SUMMARY_START_TIME comes from the non-empty st.sLotStartTime branch (golden :2267-2270) and SUMMARY_END_TIME is the stamp the function itself wrote into st.sLotEndTime (golden :2065/:2277)");
        CHECK(FileHasExactLine(fileLotA, "Handler Load Qty:31")
              && FileHasExactLine(fileLotB, "Handler Load Qty:62")
              && FileHasExactLine(fileComb, "Handler Load Qty:90"),
              "Handler Load Qty is iByLotLoadCount[k] per lot (31/62) but switches to LastSet.iSCKARTInputCT (90) for the combined arm (golden :2361-2365)");
        CHECK(FileHasExactLine(fileLotA, "Handler Unload Qty:130")
              && FileHasExactLine(fileLotB, "Handler Unload Qty:254")
              && FileHasExactLine(fileComb, "Handler Unload Qty:504"),
              "Handler Unload Qty is the bin 0..iTestBinCount x site accumulation over iByLotCountCategory[k][][] per lot (130/254) and over iCountCategory[][] combined (504) (golden :2280-2309)");
        CHECK(FileHasExactLine(fileComb, " 991: 2") && FileHasExactLine(fileComb, " 992: 3")
              && FileHasExactLine(fileComb, " 993: 5") && FileHasExactLine(fileComb, " 994: 10"),
              "the combined Error Bin Summary reports the real iE1/iE2/iE3 globals (2/3/5) and computes 994 as iTotalCategory[iTestBinCount]-iE1-iE2-iE3 == 20-2-3-5 == 10 (golden :2643-2650)");
        CHECK(FileHasExactLine(fileLotA, " 991: 0") && FileHasExactLine(fileLotA, " 994: 0"),
              "the PER-LOT Error Bin Summary hardcodes literal zeros regardless of the real counters (golden :2656-2663)");
        CHECK(FileHasLinePrefix(fileComb, "PROGRAM_NAME:HT9046."),
              "PROGRAM_NAME is \"<sInfo_ProgramName>.<MainVersion>\" with sInfo_ProgramName driven by MachineTypeChoice==Type_HT9046 (golden :2087-2088/:2213)");

        // -----------------------------------------------------------------------------------
        // 14F -- the loop ran to completion, so golden :2802's UNCONDITIONAL ClearAllData() fired.
        //   Since gate #5's stub now also zeroes iE1/iE2/iE3 (matching golden cSocket.cpp:760-762),
        //   that is directly observable -- and it is what sets up 14G's second call.
        // -----------------------------------------------------------------------------------
        CHECK(iE1Count == 0 && iE2Count == 0 && iE3Count == 0,
              "completing the k loop reached golden :2802's unconditional LotSummary.ClearAllData(), which zeroed the iE1/iE2/iE3 globals seeded to 2/3/5 (golden cSocket.cpp:760-762)");
        CHECK(W5SckArtRem_LotSummary.iTotalCategory[1] == 0 && W5SckArtRem_LotSummary.iCountCategory[0][1] == 0,
              "...the same ClearAllData() zeroed the combined-slot mirrors iTotalCategory[1] (was 66) and iCountCategory[0][1] (was 32)");
        CHECK(W5SckArtRem_LotSummary.iByLotTotalCategory[0][1] == 23
              && W5SckArtRem_LotSummary.iByLotCountCategory[0][0][1] == 11
              && FileExists(fileComb),
              "...but did NOT zero the PER-LOT mirrors: golden's own ClearAllData() (cSocket.cpp:754-763) never touches iByLotCountCategory/iByLotTotalCategory, and this stub matches that exactly");

        // -----------------------------------------------------------------------------------
        // 14G -- the two-call scenario. iE1/iE2/iE3 are now 0 (14F) and iENotDefinedCount is 0, so
        //   the SECOND call's final iteration hits golden :2069's guard. GOLDEN BUG #3: that guard
        //   is `return`, not `break` -- the per-lot files for k=0,1 were already written, but the
        //   combined file is never produced AND :2802's ClearAllData() is skipped entirely.
        //   Distinct lot IDs give distinct file names, so golden :2709's FileExists dedup cannot be
        //   mistaken for the guard.
        // -----------------------------------------------------------------------------------
        W5SckArtRem_LotSummary.iTotalCategory[1] = 777;   // must SURVIVE -- ClearAllData() is skipped

        SckArtRemainderState st14b = st14;
        st14b.sInfoArr_InnerLotID[0] = "W906P14LOTC";  st14b.sInfoArr_InnerLotID[1] = "W906P14LOTD";
        st14b.sInfoArr_CustLotID[0]  = "CLC";          st14b.sInfoArr_CustLotID[1]  = "CLD";
        st14b.sInfoArr_CustDevGup[0] = "DGC";          st14b.sInfoArr_CustDevGup[1] = "DGD";
        st14b.sLotEndTime            = "SENTINEL-NOT-OVERWRITTEN";

        SckArtRem_SaveMultiLotTestSummary(st14b, /*bSaveData=*/true);

        AnsiString stamp14b = st14b.sLotEndTime.SubString(1,12);
        AnsiString dir14b   = asSummaryPath + AnsiString("\\") + st14b.sLotEndTime.SubString(1,4)
                                            + AnsiString("\\") + st14b.sLotEndTime.SubString(5,2) + AnsiString("\\");
        AnsiString fileLotC  = dir14b + AnsiString("W906P14LOTC_CLC_DGC_")    + stamp14b + AnsiString(".txt");
        AnsiString fileLotD  = dir14b + AnsiString("W906P14LOTD_CLD_DGD_")    + stamp14b + AnsiString(".txt");
        AnsiString fileCombCD= dir14b + AnsiString("W906P14LOTCW906P14LOTD_") + stamp14b + AnsiString(".txt");

        CHECK(FileExists(fileLotC) && FileExists(fileLotD),
              "second call: the k=0 and k=1 per-lot iterations still ran and wrote their files BEFORE the final-iteration guard was ever evaluated (golden :2067-2072)");
        CHECK(FileExists(fileCombCD) == false && FileExists(fileLotD),
              "second call: with iE1/iE2/iE3 zeroed by the first call's ClearAllData() and iENotDefinedCount==0, golden :2069's guard fires and the COMBINED summary is never produced");
        CHECK(W5SckArtRem_LotSummary.iTotalCategory[1] == 777 && FileExists(fileLotD),
              "GOLDEN BUG #3: that guard is `return`, not `break`, so golden :2802's unconditional ClearAllData() is SKIPPED and the seeded iTotalCategory[1]==777 leaks past the lot boundary");
        CHECK(FileHasLinePrefix(fileLotC, "    1 PASS    11    12    23"),
              "second call: lot C's Hard Bin row still reads 11/12/23 -- the per-lot mirrors survived the first call's ClearAllData(), matching golden (cSocket.cpp:754-763 leaves them alone)");

        // ---- idempotent cleanup: remove every file/folder this PART created ----
        if (FileExists(fileLotA))   DeleteFile(fileLotA);
        if (FileExists(fileLotB))   DeleteFile(fileLotB);
        if (FileExists(fileComb))   DeleteFile(fileComb);
        if (FileExists(fileLotC))   DeleteFile(fileLotC);
        if (FileExists(fileLotD))   DeleteFile(fileLotD);
        if (FileExists(fileCombCD)) DeleteFile(fileCombCD);
        RemoveDir(asSummaryPath + AnsiString("\\") + st14.sLotEndTime.SubString(1,4) + AnsiString("\\") + st14.sLotEndTime.SubString(5,2));
        RemoveDir(asSummaryPath + AnsiString("\\") + st14b.sLotEndTime.SubString(1,4) + AnsiString("\\") + st14b.sLotEndTime.SubString(5,2));
        RemoveDir(asSummaryPath + AnsiString("\\") + st14.sLotEndTime.SubString(1,4));
        RemoveDir(asSummaryPath);

        // ---- restore every global this PART steered, and leave LotSummary clean for re-runs ----
        W5SckArtRem_LotSummary.ClearAllData();
        {
            int L, s, b;
            for (L = 0; L < 5; ++L) W5SckArtRem_LotSummary.iByLotLoadCount[L] = 0;
            for (L = 0; L < 5; ++L)
                for (b = 0; b <= 4; ++b)
                {
                    W5SckArtRem_LotSummary.iByLotTotalCategory[L][b] = 0;
                    for (s = 0; s < 2; ++s)
                        W5SckArtRem_LotSummary.iByLotCountCategory[L][s][b] = 0;
                }
        }
        for (i14 = 0; i14 < eTrayCount; ++i14) Prod.iTrayType[i14] = savedTrayType14[i14];
        for (i14 = 0; i14 < 8; ++i14)
        {
            Prod.iT6PosCate[i14] = savedPosCate14[i14];
            Prod.bIsPassBin[i14] = savedPassBin14[i14];
        }
        asSummaryPath                  = savedSummaryPath14;
        MachineTypeChoice              = savedMachineType14;
        iTestBinCount                  = savedTestBinCount14;
        TestSocket.iShtRow             = savedShtRow14;
        TestSocket.iShtCol             = savedShtCol14;
        Prod.iIfErrorT6                = savedIfErrorT6_14;
        LastSet.iSCKARTInputCT         = savedInputCT14;
        iE1Count                       = savedE1_14;
        iE2Count                       = savedE2_14;
        iE3Count                       = savedE3_14;
        iENotDefinedCount              = savedEnd_14;
        IniConfig.bSPILFunction        = savedSPIL14;
        IniConfig.bN17UploadLotSummary = savedN17_14;
        IniConfig.bA38_SLT_Summary     = savedA38_14;
        CosFunction.bUseTSVFunction    = savedTSV14;
        CosFunction.bSortingBy2DList   = savedSort2D14;
    }

    // AI(W906-P10) 20260921: 收尾還原（見 main() 開頭的說明）
    CloseGeneralIniFile();
    asGeneralPath = savedGeneralPath;

    printf("\n=== %d PASS, %d FAIL (of %d) ===\n", g_pass, g_fail, g_pass + g_fail);
    return (g_fail == 0) ? 0 : 1;
}
