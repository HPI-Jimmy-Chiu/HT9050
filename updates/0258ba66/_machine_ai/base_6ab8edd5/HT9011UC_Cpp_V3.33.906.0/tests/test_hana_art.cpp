// =============================================================================
//  test_hana_art.cpp  --  W5 VERIFY: HANA-ART SRQ protocol helper
//                          (Automation/HANA_ART.cpp, class uHANA_ART)
//
//  Translation wave: W5 Wave 1 (parallel translate batch)
//  Author: AI(W5-HanaART-Translate) 20260710
//
//  PURPOSE
//  -------
//  Drives the ACTUAL translated uHANA_ART class through its public API over a
//  known global-state baseline (no hardware), asserting outputs against
//  values hand-derived from the golden BCB6 source (golden file:line cited
//  per case), same style as test_amr.cpp / test_w6_3_catchtray_core.cpp /
//  test_cContact.cpp.  uHANA_ART's HdWaitData/HdStartData/TestData members
//  are `protected` in the golden class (preserved as such here) so this is
//  necessarily BLACK-BOX testing through the public method surface -- exactly
//  the encapsulation golden itself uses.
//
//  EQUIVALENCE NOTE: no Borland binary exists, so "equivalence" here == clean
//  g++ compile/link + the translated code producing outputs that match values
//  hand-derived from the golden source, over an explicitly-set global-state
//  baseline.  This TU touches shared mutable global state (CosFunction/
//  IniConfig/TestIF_File/BinSelect/Temperature/RunInfo/Prod/LastSet/
//  TestSocket/fSCKART/fLotInfo), so each PART resets exactly the fields it
//  reads/writes before asserting, to stay independent of PART execution order
//  (matches the test_amr.cpp reset-helper convention).
//
//  BUILD NOTE (expected, not a bug in this test): this TU will not compile
//  until the integrate agent adds the fields/methods this unit's translate
//  report lists:
//    canary_support.h LAST_GENERAL_SET : iASEContact (int) / iBinData32[4][260] (int)
//      (iBinData32 is ALSO independently needed by Automation/AMR.cpp -- same
//      field, same [4][260] golden LastSet.h:366 citation -- adding it once
//      satisfies both units; see HANA_ART.cpp's own file-header note.)
//    FormsFacade.h TfMain    : SendMSG_CMD(int) / SendMSG_CMD(int,AnsiString) /
//                              GetSamSungMap(bool=true) / GetSamSungSoakTime(bool=true) /
//                              ArmStatusStrings()
//    FormsFacade.h TfSCKART  : iCurrent93KARTStep (int -- ALSO independently
//                              needed by Automation/AMR.cpp, same golden
//                              Automation/SCK_ART.h:263 citation) / sLotID /
//                              sProcessCode / DoARTLotStart(AnsiString,AnsiString,int) /
//                              AccessFile(bool,int=-1) / palLotNumber->Caption /
//                              palTestCnt->Caption / palRTTryCnt->Caption /
//                              edlRTTryCnt->Text / pnlProcessCode->Caption
//    FormsFacade.h TfLotInfo : cbProcess->Text / SetLotID(AnsiString,bool=false) /
//                              SetLotStart(AnsiString,bool=false)
//  HANA_ART.cpp's own syntax was independently verified clean via
//  `g++ -fsyntax-only`: every reported error maps 1:1 to one of the fields
//  above, nothing else (see translate report).  This test file is written
//  against the POST-INTEGRATE API, matching the test_amr.cpp precedent for
//  this exact situation in this same W5 batch.
// =============================================================================
#include "Automation/HANA_ART.h"
#include "FormsFacade.h"            // fMain / fSCKART / fLotInfo
#include "cprod.h"                  // Prod / RunInfo / TestIF_File / BinSelect / Temperature / IniConfig / CosFunction
#include "cmydef.h"                 // iTestRunMode / iTestBinCount / Tempture_*
#include "canary_support.h"         // LastSet
#include "aHotPlateSubstrate.h"     // TestSocket
#include <cstdio>
#include <cstring>
#include "w906_ctest_guard.h"   // AI(W906-S09-B6T) 20260929 (St02-E, claim): W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("  PASS: %s\n", msg); ++g_pass; }                    \
        else      { printf("  FAIL: %s  (line %d)\n", msg, __LINE__); ++g_fail; }\
    } while (0)

static bool streq(const AnsiString& a, const char* b)
{
    return std::strcmp(a.c_str(), b) == 0;
}

// ---------------------------------------------------------------------------
//  Shared-state reset helpers -- each PART calls the relevant one(s) first so
//  PARTS are independent of execution order (global mutable state).
// ---------------------------------------------------------------------------
static void ResetHanaArtGlobals()
{
    CosFunction.bUseSCKART                    = false;
    IniConfig.bA10_AutoReTest                 = false;
    IniConfig.bA10_6_HANA_ART_TestMode_Enable = false;
    TestIF_File.bSCKART_EnableART             = false;
    LastSet.iRunStartMode = 0;
    LastSet.iTemperature  = Tempture_Ambient;
    LastSet.iASEContact   = 0;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 260; j++)
            LastSet.iBinData32[i][j] = 0;
    iTestRunMode  = 0;
    iTestBinCount = 4;
    for (int i = 0; i < 8; i++) BinSelect[0].iCatDataT3Pos[i] = 0;
    for (int i = 0; i < 8; i++) Prod.bIsPassBin[i] = 0;
    Temperature.fWorkTemperBase = 0.0;
    Temperature.bUseAbitCHK     = false;
    Temperature.fAbitTemp       = 0.0;
    RunInfo.bLotStart = false;
}

int main()
{   extern AnsiString DataPath; const char* const w906rt[] = { "DataPath", DataPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("HanaART", w906rt)) return 2;   // AI(W906-S09-B6T) 20260929 (St02-E, claim): cBinSel ReadFile/Save write under DataPath once S-09 batch 6 is lifted -- refuse a run outside ctest's redirect
    printf("=== uHANA_ART (Automation/HANA_ART.cpp) translation verification ===\n");
    printf("(values hand-derived/transcribed from BCB6 HANA_ART.cpp; original binary NOT run)\n\n");

    // =========================================================================================
    // PART 1: GetHanaSRQStr -- golden HANA_ART.cpp:48-90 (pure switch, no globals touched)
    // =========================================================================================
    printf("-- GetHanaSRQStr: SRQ code -> tag string (golden :48-90) --\n");
    {
        uHANA_ART h;
        CHECK(streq(h.GetHanaSRQStr(uHANA_ART::SETUP_INFORM_REQUEST_SRQ0x55), "SETUP_INFORM_REQUEST_SRQ"),
              "0x55 -> SETUP_INFORM_REQUEST_SRQ");
        // golden :61 special-cases PRIME_START_SRQ0x50 to the literal "PRIMETESTSTARTOK"
        // (NOT "PRIME_START_SRQ" -- faithful golden quirk, not a naming bug).
        CHECK(streq(h.GetHanaSRQStr(uHANA_ART::PRIME_START_SRQ0x50), "PRIMETESTSTARTOK"),
              "0x50 -> PRIMETESTSTARTOK (faithful golden quirk, not PRIME_START_SRQ)");
        CHECK(streq(h.GetHanaSRQStr(uHANA_ART::LOT_END_SRQ0x64), "LOT_END_SRQ"), "0x64 -> LOT_END_SRQ");
        CHECK(streq(h.GetHanaSRQStr(0x1234), "UNKNOWN_SRQ"), "unmapped code -> UNKNOWN_SRQ (golden :86 default)");
    }

    // =========================================================================================
    // PART 2: ParseLOTONStr -- golden HANA_ART.cpp:777-803 (pure string parsing)
    // =========================================================================================
    printf("-- ParseLOTONStr: \"LOTON:<lot>,<size>,<mode>\" parse (golden :777-803) --\n");
    {
        uHANA_ART h;
        AnsiString sLotNo, sMode;
        int LotSize = -999;

        bool ok1 = h.ParseLOTONStr("LOTON:ENGLOT,3000,XE", sLotNo, LotSize, sMode);
        CHECK(ok1, "well-formed LOTON: string parses OK");
        CHECK(streq(sLotNo, "ENGLOT"), "sLotNo == ENGLOT");
        CHECK(LotSize == 3000, "LotSize == 3000");
        CHECK(streq(sMode, "XE"), "sMode == XE");

        AnsiString sLotNo2, sMode2;
        int LotSize2 = -999;
        bool ok2 = h.ParseLOTONStr("NOCOLONHERE", sLotNo2, LotSize2, sMode2);
        CHECK(ok2 == false, "no ':' -> false (golden :780-783)");

        AnsiString sLotNo3, sMode3;
        int LotSize3 = -999;
        bool ok3 = h.ParseLOTONStr("LOTON:ONLYONEFIELD", sLotNo3, LotSize3, sMode3);
        CHECK(ok3 == false, "no ',' after lot -> false (golden :786-789)");

        AnsiString sLotNo4, sMode4;
        int LotSize4 = -999;
        bool ok4 = h.ParseLOTONStr("LOTON:LOT2,500", sLotNo4, LotSize4, sMode4);
        CHECK(ok4 == false, "no 2nd ',' (missing mode) -> false (golden :792-795)");
    }

    // =========================================================================================
    // PART 3: IsHanaArtAvailable -- golden HANA_ART.cpp:740-753 (4-way AND over globals)
    // =========================================================================================
    printf("-- IsHanaArtAvailable: 4-way AND gate (golden :742-755) --\n");
    {
        uHANA_ART h;
        ResetHanaArtGlobals();
        CHECK(h.IsHanaArtAvailable() == false, "all 4 gates false -> false");

        CosFunction.bUseSCKART = true;
        IniConfig.bA10_AutoReTest = true;
        TestIF_File.bSCKART_EnableART = true;
        IniConfig.bA10_6_HANA_ART_TestMode_Enable = true;
        CHECK(h.IsHanaArtAvailable() == true, "all 4 gates true -> true");

        IniConfig.bA10_6_HANA_ART_TestMode_Enable = false;
        CHECK(h.IsHanaArtAvailable() == false, "one gate false (bA10_6_HANA_ART_TestMode_Enable) -> false");
        ResetHanaArtGlobals();
    }

    // =========================================================================================
    // PART 4: GetHD_ID / SetHandlerWaitingData -- golden HANA_ART.cpp:150-153/95-108
    // =========================================================================================
    printf("-- SetHandlerWaitingData + GetHD_ID (golden :95-108/150-153) --\n");
    {
        uHANA_ART h;
        h.SetHandlerWaitingData("HT9046", "HT9046-01", "MYSITE", 0);
        // SOFT_SIMULTE is NOT defined in this target build (matches every other
        // translated engine file) -- golden's `#ifdef SOFT_SIMULTE Site="HANA"`
        // override does NOT fire, so the passed-in Site is kept verbatim.
        CHECK(streq(h.GetHD_ID(), "HT9046,HT9046-01,MYSITE"), "GetHD_ID formats Model,Model_NO,Site");
    }

    // =========================================================================================
    // PART 5: GetHD_CT -- golden HANA_ART.cpp:161-181 (pass-bin -> \"CTxx\" bin-config string)
    // =========================================================================================
    printf("-- GetHD_CT: pass-bin digit-string with CT/CT0 zero-pad (golden :162-183) --\n");
    {
        uHANA_ART h;
        ResetHanaArtGlobals();
        // No bin passes -> sRet="" (length 0 < 2) -> "CT0"+"" == "CT0".
        CHECK(streq(h.GetHD_CT(), "CT0"), "no pass bin -> CT0 (golden :174-177 zero-pad branch)");

        // One passing bin (index 5): iCatDataT3Pos[5]=1 (iT3=0>=0) + bIsPassBin[5]=1 -> sRet="5"
        // (length 1 < 2) -> "CT0"+"5" == "CT05".
        BinSelect[0].iCatDataT3Pos[5] = 1;
        Prod.bIsPassBin[5] = 1;
        CHECK(streq(h.GetHD_CT(), "CT05"), "single pass bin 5 -> CT05 (zero-padded)");

        // Two passing bins (2 and 7) -> sRet="27" (length 2, not <2) -> "CT"+"27" == "CT27".
        ResetHanaArtGlobals();
        BinSelect[0].iCatDataT3Pos[2] = 1;
        Prod.bIsPassBin[2] = 1;
        BinSelect[0].iCatDataT3Pos[7] = 1;
        Prod.bIsPassBin[7] = 1;
        CHECK(streq(h.GetHD_CT(), "CT27"), "two pass bins 2,7 -> CT27 (no zero-pad)");

        // iCatDataT3Pos[i]-1 < 0 (i.e. iCatDataT3Pos[i]==0) must NOT count even if bIsPassBin set
        // (golden :168 requires iT3>=0 AND bIsPassBin -- both conditions, not just the bin flag).
        ResetHanaArtGlobals();
        Prod.bIsPassBin[3] = 1;                      // bIsPassBin true, but iCatDataT3Pos stays 0
        CHECK(streq(h.GetHD_CT(), "CT0"), "bIsPassBin true but iCatDataT3Pos==0 -> still excluded");
        ResetHanaArtGlobals();
    }

    // =========================================================================================
    // PART 6: GetHD_Tempset -- golden HANA_ART.cpp:183-204 (4-branch temperature-mode ladder)
    // =========================================================================================
    printf("-- GetHD_Tempset: 4-branch temperature-mode ladder (golden :185-206) --\n");
    {
        uHANA_ART h;
        ResetHanaArtGlobals();

        LastSet.iTemperature = Tempture_Hot;
        Temperature.fWorkTemperBase = 85.0;
        CHECK(streq(h.GetHD_Tempset(), "85"), "Tempture_Hot -> fWorkTemperBase (golden :188-191)");

        LastSet.iTemperature = Tempture_AmbientHot;
        Temperature.fWorkTemperBase = 45.0;
        CHECK(streq(h.GetHD_Tempset(), "45"), "Tempture_AmbientHot -> fWorkTemperBase (golden :192-195)");

        LastSet.iTemperature = Tempture_Ambient;   // neither Hot nor AmbientHot
        Temperature.bUseAbitCHK = true;
        Temperature.fAbitTemp = 60.0;
        CHECK(streq(h.GetHD_Tempset(), "60"), "Ambient + bUseAbitCHK -> fAbitTemp (golden :196-199)");

        Temperature.bUseAbitCHK = false;
        CHECK(streq(h.GetHD_Tempset(), "25"), "Ambient, no AbitCHK -> literal 25 default (golden :200-203)");
        ResetHanaArtGlobals();
    }

    // =========================================================================================
    // PART 7: GetHD_TDATA / GetHD_JDATA / GetHD_ADATA -- golden :213-225/254-268/270-274
    //   (pure formatting over the class's own protected timeData member; only reachable via
    //   Clear()'s zero-init, verifying the ctor -> Clear() chain golden :31-33/36-45).
    // =========================================================================================
    printf("-- GetHD_TDATA/JDATA/ADATA: zero-initialized after ctor (golden Clear() :36-45) --\n");
    {
        uHANA_ART h;   // ctor -> Clear() zero-inits timeData (golden :26-29)
        CHECK(streq(h.GetHD_TDATA(), "0,0,0,0,0,0,0,0,0"), "9 zeroed TDATA slots, comma-joined (golden :213-225)");
        CHECK(streq(h.GetHD_JDATA(), "IJ:0,OJ:0,FCJ:0,BCJ:0,SJ:0,FJ:0,TTJ:0,APJ:0"),
              "8 zeroed JDATA slots with position tags (golden :254-268)");
        CHECK(streq(h.GetHD_ADATA(), "ZZ"), "GetHD_ADATA is a fixed literal \"ZZ\" (golden :270-274)");
    }

    // =========================================================================================
    // PART 8: DoSETUP_INFORM / GetNow_FTRT_Mode -- golden :276-284/132-149
    //   (protected HdStartData is only observable through GetNow_FTRT_Mode()/IsPrimeTest())
    // =========================================================================================
    printf("-- DoSETUP_INFORM drives frtHALT -> frtFT (golden :276-284), observed via GetNow_FTRT_Mode --\n");
    {
        uHANA_ART h;
        CHECK(streq(h.GetNow_FTRT_Mode(), "Halt"), "fresh ctor -> frtHALT -> \"Halt\" (golden :144-147 else branch)");
        h.DoSETUP_INFORM();
        CHECK(streq(h.GetNow_FTRT_Mode(), "FT"), "DoSETUP_INFORM() from frtHALT -> frtFT -> \"FT\"");
        // golden :278 guards on frtHALT -- calling again while already frtFT is a no-op.
        h.DoSETUP_INFORM();
        CHECK(streq(h.GetNow_FTRT_Mode(), "FT"), "DoSETUP_INFORM() re-entry while already frtFT is a no-op (golden :278 guard)");
    }

    // =========================================================================================
    // PART 9: StartPrimeTest / IsPrimeTest / EndPrimeTest -- golden :654-667/(GetHanaSRQStr helper)/674-689
    // =========================================================================================
    printf("-- StartPrimeTest/IsPrimeTest/EndPrimeTest (golden :654-689) --\n");
    {
        uHANA_ART h;
        ResetHanaArtGlobals();
        TestSocket.iShtRow = 2;
        TestSocket.iShtCol = 4;
        CHECK(h.IsPrimeTest() == false, "fresh ctor -> bEnableContact_FT==false");
        h.StartPrimeTest();
        CHECK(h.IsPrimeTest() == true, "StartPrimeTest() sets bEnableContact_FT=true (golden :656)");
        h.EndPrimeTest();
        CHECK(h.IsPrimeTest() == false, "EndPrimeTest() clears bEnableContact_FT (golden :676)");
    }

    // =========================================================================================
    // PART 10: RecoveryARTStatus -- golden :915-926 (LastSet.iRunStartMode -> contact-enable flags)
    // =========================================================================================
    printf("-- RecoveryARTStatus: iRunStartMode -> contact-enable flags (golden :915-926) --\n");
    {
        ResetHanaArtGlobals();
        uHANA_ART h;
        LastSet.iRunStartMode = rsmContinuStart_ART;
        h.RecoveryARTStatus();
        CHECK(h.IsPrimeTest() == true, "rsmContinuStart_ART -> bEnableContact_FT=true (golden :918-921)");
        ResetHanaArtGlobals();
    }

    // =========================================================================================
    // PART 11: IsContactAvailable -- golden :757-775 (modeGeneral vs modeSMILL branch)
    //   HD_Mode is passed as a plain int: 0==golden HandlerWaitingData::modeGeneral,
    //   1==modeSMILL (HANA_ART.h's nested ModeType enum is private, so the test drives
    //   it through SetHandlerWaitingData's public int parameter with the golden literal
    //   values, same black-box-through-public-API approach as every other PART here).
    // =========================================================================================
    printf("-- IsContactAvailable: modeGeneral vs modeSMILL branch (golden :757-775) --\n");
    {
        uHANA_ART h;
        ResetHanaArtGlobals();
        TestSocket.iShtRow = 1;
        TestSocket.iShtCol = 1;
        h.SetHandlerWaitingData("M", "M-01", "S", 0);   // 0 == modeGeneral
        CHECK(h.IsContactAvailable() == false, "modeGeneral, no prime test yet -> false");
        h.StartPrimeTest();
        CHECK(h.IsContactAvailable() == true,
              "modeGeneral -> returns bEnableContact_FT directly (golden :759-762), ignores bDUMMY_TEST_0x42_OK");
    }
    {
        uHANA_ART h;
        ResetHanaArtGlobals();
        TestSocket.iShtRow = 1;
        TestSocket.iShtCol = 1;
        h.SetHandlerWaitingData("M", "M-01", "S", 1);   // 1 == modeSMILL
        h.StartPrimeTest();                              // bEnableContact_FT=true, bDUMMY_TEST_0x42_OK=false (golden :656-657)
        CHECK(h.IsContactAvailable() == false,
              "modeSMILL, FT enabled but bDUMMY_TEST_0x42_OK still false -> false (golden :765-772)");
        h.DoCmdWhenHDStart("DUMMY_TEST_0x42_OK");        // sets bDUMMY_TEST_0x42_OK=true
        CHECK(h.IsContactAvailable() == true,
              "modeSMILL, FT enabled AND bDUMMY_TEST_0x42_OK now true -> true");
        ResetHanaArtGlobals();
    }

    // -----------------------------------------------------------------------
    // Summary
    // -----------------------------------------------------------------------
    printf("\n=== SUMMARY: %d passed, %d failed (total %d) ===\n",
           g_pass, g_fail, g_pass + g_fail);
    if (g_fail != 0)
    {
        printf("RESULT: FAIL\n");
        return 1;
    }
    printf("RESULT: ALL PASS\n");
    return 0;
}
