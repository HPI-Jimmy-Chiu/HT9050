// ===========================================================================
//  test_change_log.cpp -- AI(W906-CHGLOG) 20260927
//
//  golden WriteIniData change-log blocks + TempChangeLog (common_ChangeLog.cpp,
//  generated from golden common.cpp:622-1099 / :1802-2037).
//   [1] TempChangeLog name mapping, incl. the index guards (banner (c)).
//   [2]-[5] the four blocks called directly: Str1 / Str2 / bHasChange exactly as
//       golden formats them, the InitialOK gate, and the item captions
//       (banner (b): .dfm tables, fContactForm, the ASE GPIB list, the DIO listing).
//   [6] end to end through common.cpp's WriteIniData: no hook -> no change log
//       (the behaviour before this change); hook installed -> logged once
//       (ExString, which RecordChangeLogProcess sets); same value again -> not.
//  Writes only under %TEMP%\ht9045_chglog_<pid>\ and removes it.  AI(W906-TESTGUARD) 20260928: those are the test's OWN files; the change-log record itself ([2]-[6] run with InitialOK=true, [6] through the installed hooks) goes RecordChangeLogProcess -> MyDBIProcess("ChangeLog") into the cMyDB log roots, so main() refuses to run outside ctest's redirect environment (tests/w906_ctest_guard.h, exit 2; W906_TEST_ALLOW_REAL_FILES=1 runs it anyway).
//  [4b] the double block: numeric compare (NB2 R87), Kit Diameter / mm texts, the torque re-learn flags.
//  NOT COVERED: where the record
//  lands (MyDBIProcess("ChangeLog") is still the counting stand-in before cMyDB P4).  AI(W906-TESTGUARD) 20260928: once cMyDB P4 (St02, MR !3) is linked it is the golden body -- EventLogTxt / SaveEventLog / Production_Log files under the ctest roots; still not asserted here.
// ===========================================================================
#include "vclcompat/vcl_compat.h"
#include "common.h"
#include "common_ChangeLog.h"
#include "cmydef.h"          // InitialOK, asTempCtrl, CUSTOMER_CODE
#include "cprod.h"           // TestIF_File
#include "cpublic.h"         // ConvertToMMType
#include "cMyDB.h"           // ExString
#include "MachineType.h"     // _8Site2X4, CC_ASE_KaohSiung
#include "forms/fContact.h"  // fContactForm
#include "w906_ctest_guard.h"   // AI(W906-TESTGUARD) 20260928: W906TestRequireCtestRedirects (occupies the old blank line; no line moves)
#include <windows.h>
#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;

#define CHECK(c) do { if (c) { ++g_pass; std::printf("  PASS: %s\n", #c); } \
                      else { ++g_fail; std::printf("  FAIL: %s  (line %d)\n", #c, __LINE__); } } while (0)
#define CHECK_S(a, b) do { AnsiString _a = (a); const char* _b = (b); \
                           if (std::strcmp(_a.c_str(), _b) == 0) { ++g_pass; std::printf("  PASS: %s == \"%s\"\n", #a, _b); } \
                           else { ++g_fail; std::printf("  FAIL: %s == \"%s\"  got \"%s\"  (line %d)\n", #a, _b, _a.c_str(), __LINE__); } } while (0)

static AnsiString S1, S2;
static bool HC;
static void reset() { S1 = ""; S2 = ""; HC = false; ExString = ""; }

static std::string g_root;
static void mkfile(const std::string& p, DWORD attr)
{
    FILE* f = std::fopen(p.c_str(), "wb");
    if (f) std::fclose(f);
    if (attr) SetFileAttributesA(p.c_str(), attr);
}

int main()
{   const char* const rt[] = { "as9045LogPath", as9045LogPath.c_str(), "asSaveEventLogPath", asSaveEventLogPath.c_str(), "asGeneralPath", asGeneralPath.c_str(), "AuthPath", AuthPath.c_str(), 0 }; if (!W906TestRequireCtestRedirects("ChangeLog", rt)) return 2;   //AI(W906-TESTGUARD) 20260928: first statement -- refuse outside ctest's redirect environment (the variables AND the common.h globals the log writers use); see the banner
    char tmp[MAX_PATH];
    DWORD n = GetTempPathA(MAX_PATH, tmp);
    if (n == 0 || n >= MAX_PATH) { std::printf("no TEMP\n"); return 1; }
    char pid[32];
    std::snprintf(pid, sizeof(pid), "%lu", (unsigned long)GetCurrentProcessId());
    g_root = std::string(tmp) + "ht9045_chglog_" + pid;
    CreateDirectoryA(g_root.c_str(), NULL);

    const bool savedInitialOK = InitialOK;
    const int savedCustomer = CUSTOMER_CODE;
    const int savedTestMode = TestIF_File.iTestMode;
    const bool savedOctal = TestIF_File.bOctal_16Kit;
    const AnsiString savedDio = DIOCFGPath;
    CUSTOMER_CODE = 0;
    TestIF_File.iTestMode = 0;
    TestIF_File.bOctal_16Kit = false;

    std::printf("[1] TempChangeLog (golden common.cpp:1802-2037)\n");
    CHECK(std::strcmp(TempChangeLog("Low OffSet", "CH1").c_str(), asTempCtrl[0].c_str()) == 0);        // asTempCtrl[iSiteAdd-1]
    CHECK(std::strcmp(TempChangeLog("SingleTempLimit", "CH2").c_str(), asTempCtrl[2].c_str()) == 0);   // SingleTempLimit: no -1
    CHECK_S(TempChangeLog("Low OffSet", "CH0"), "CH0");                     // guard (c): golden would read asTempCtrl[-1]
    CHECK_S(TempChangeLog("Low OffSet", "Offset"), "Offset");               // not "CH.." -> unchanged
    // key format as in the recipes (IniData Temperature.Data: ATCTempOffset[0]=0 ...): SubString(15, Length-15) = the digits
    CHECK_S(TempChangeLog("ATC", "ATCTempOffset[3]"), "OffSet_Ad1");
    CHECK_S(TempChangeLog("ATC", "ATCTempOffset[12]"), "OffSet_Be1");
    TestIF_File.iTestMode = _8Site2X4; TestIF_File.bOctal_16Kit = true;    // JerryYang 20230828: -2
    CHECK_S(TempChangeLog("ATC", "ATCTempOffset[3]"), "OffSet_Ab1");
    CHECK_S(TempChangeLog("ATC", "ATCTempOffset[1]"), "OffSet_Ab1");         // golden: only >=2 is shifted
    TestIF_File.iTestMode = 0; TestIF_File.bOctal_16Kit = false;
    CHECK_S(TempChangeLog("InitialMode", "iInitialDelay_3"), "After Auto Clean Function");
    CHECK_S(TempChangeLog("InitialMode", "iInitialDelay"), "Every first devices");
    CHECK_S(TempChangeLog("InitialMode", "bOTDUnlockDelay"), "bOTD Unlock Use Initial Delay");
    CHECK_S(TempChangeLog("Time", "Stary Delay"), "Start Delay");
    CHECK_S(TempChangeLog("Mode", "iSocketInitialICCheckPosition"), "iSocket Initial IC Check Position");
    CHECK_S(TempChangeLog("Configuration", "iAutoClean_MotorSpeed[2]"), "Auto Clean Index Arm Speed");
    CHECK_S(TempChangeLog("Configuration", "iAutoClean_MotorSpeed[7]"), "iAutoClean_MotorSpeed[7]");   // guard (c)
    CHECK_S(TempChangeLog("Configuration", "ShuttlePitchOffset"), "Auto Clean In Arm Shuttle Pitch Offset");
    CHECK_S(TempChangeLog("Speed", "X"), "X");

    std::printf("[2] WriteIniData(bool) block (golden :637-671)\n");
    InitialOK = false; reset();
    W906_ChangeLog_Bool("x\\HandlerCondition.Data", "Configuration", "bAutoClean_UseTray", false, true, S1, S2, HC);
    CHECK(HC == false); CHECK_S(S1, ""); CHECK_S(ExString, "");            // InitialOK==false -> nothing
    InitialOK = true; reset();
    W906_ChangeLog_Bool("x\\HandlerCondition.Data", "Configuration", "bAutoClean_UseTray", false, true, S1, S2, HC);
    CHECK(HC == true);
    CHECK_S(S1, "Configuration_bAutoClean_UseTray change Value");
    CHECK_S(S2, "Kit ==> Tray");                                            // uCleaning.dfm rgCleanKitType
    CHECK_S(ExString, "Configuration_bAutoClean_UseTray change Value");     // RecordChangeLogProcess ran
    reset();
    W906_ChangeLog_Bool("x\\HandlerCondition.Data", "Configuration", "bAutoClean_UseTray", true, true, S1, S2, HC);
    CHECK(HC == false);                                                     // unchanged value
    reset();
    W906_ChangeLog_Bool("x\\ArmOffset.Data", "Test Arm1", "bUse", true, false, S1, S2, HC);
    CHECK_S(S1, "Test Arm1_bUse Offset change Value");
    CHECK_S(S2, "1==>0");

    std::printf("[3] WriteIniData(int) block (golden :706-855)\n");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "Mode", "Tester Type", 1, 3, S1, S2, HC);
    CHECK_S(S1, "Mode_Tester Type change Value"); CHECK_S(S2, "GP-IB ==> TCP/IP");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "RS-232C", "Baud Rate", 0, 2, S1, S2, HC);
    CHECK_S(S2, "19200 ==> 4800");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "RS-232C", "Parity", 2, 0, S1, S2, HC);
    CHECK_S(S2, "None ==> Even");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "RS-232C", "Stop Bit", 0, 1, S1, S2, HC);
    CHECK_S(S2, "1 Bit ==> 2Bits");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "RS-232C", "Bit Length", 1, 0, S1, S2, HC);
    CHECK_S(S2, "8 Bits ==> 7 Bits");
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "GP-IB", "Type", 1, 9, S1, S2, HC);
    CHECK_S(S2, "255 Bin ==> Delta_Castle");                                // cTesterIF.dfm list
    CUSTOMER_CODE = CC_ASE_KaohSiung;
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "GP-IB", "Type", 1, 9, S1, S2, HC);
    CHECK_S(S2, "256 Bin ==> ");                                            // golden ctor's ASE list has 9; index 9 -> "" (banner (b))
    CUSTOMER_CODE = 0;
    reset(); W906_ChangeLog_Int("x\\Tester.Data", "GP-IB", "Address", 4, 5, S1, S2, HC);
    CHECK_S(S2, "4==>5");                                                   // bStr2HasFind reset -> numeric
    reset(); W906_ChangeLog_Int("x\\Contact.Data", "Mode", "iSocketInitialICCheckPosition", 0, 1, S1, S2, HC);
    CHECK_S(S1, "Mode_iSocket Initial IC Check Position change Value");
    CHECK_S(S2, "Inside socket ==> Above Socket");
    CHECK(fContactForm != NULL);
    if (fContactForm != NULL)
    {
        const int before = fContactForm->cbContactMode->Items->Count;
        reset(); W906_ChangeLog_Int("x\\Contact.Data", "Mode", "Contact", before, before + 1, S1, S2, HC);
        CHECK_S(S2, " ==> ");                                               // past the facade's list -> "" (banner (b))
        fContactForm->cbContactMode->Items->Add("Direct Contact");
        fContactForm->cbContactMode->Items->Add("Drop Contact");
        reset(); W906_ChangeLog_Int("x\\Contact.Data", "Mode", "Contact", before, before + 1, S1, S2, HC);
        CHECK_S(S2, "Direct Contact ==> Drop Contact");                     // read from the facade
        fContactForm->cbContactMode->Items->Delete(before + 1);
        fContactForm->cbContactMode->Items->Delete(before);
        CHECK(fContactForm->cbContactMode->Items->Count == before);
    }
    reset(); W906_ChangeLog_Int("x\\HandlerCondition.Data", "Configuration", "iAutoClean_SelectArm", 0, 2, S1, S2, HC);
    CHECK_S(S2, "Arm1 ==> Arm1 & Arm2");
    reset(); W906_ChangeLog_Int("x\\HandlerCondition.Data", "Configuration", "iAutoClean_ContactMode", 1, 0, S1, S2, HC);
    CHECK_S(S2, "Drop Contact Mode ==> Direct Contact Mode");
    reset(); W906_ChangeLog_Int("x\\HandlerCondition.Data", "Configuration", "iAutoClean_ContactTime", 15, 20, S1, S2, HC);
    CHECK_S(S2, "1.5s ==> 2.0s");
    {
        AnsiString a = AnsiString(ConvertToMMType(125));
        AnsiString b = AnsiString(ConvertToMMType(250));
        AnsiString want;
        want.sprintf("%smm ==> %smm", a.c_str(), b.c_str());
        reset(); W906_ChangeLog_Int("x\\HandlerCondition.Data", "Configuration", "iAutoClean_iPadThickness", 125, 250, S1, S2, HC);
        CHECK_S(S2, want.c_str());
        CHECK_S(S1, "Configuration_Auto Clean Clean Pad Deviation change Value");
    }
    reset(); W906_ChangeLog_Int("D:\\HT9045\\config\\config.ini", "O_Count", "O_1", 10, 11, S1, S2, HC);
    CHECK(HC == false); CHECK_S(ExString, "");                              // Ifor 20191017: head contact count not logged
    {
        std::string dio = g_root + "\\DioCfg";
        CreateDirectoryA(dio.c_str(), NULL);
        mkfile(dio + "\\A1.ini", 0);
        mkfile(dio + "\\B2.ini", 0);
        mkfile(dio + "\\C0.ini", FILE_ATTRIBUTE_HIDDEN);                    // golden skips hidden files
        mkfile(dio + "\\note.txt", 0);
        DIOCFGPath = AnsiString((dio + "\\").c_str());
        reset(); W906_ChangeLog_Int("x\\Tester.Data", "DIO", "Type", 0, 1, S1, S2, HC);
        CHECK_S(S2, "A1 ==> B2");
        reset(); W906_ChangeLog_Int("x\\Tester.Data", "DIO", "Type", 1, 2, S1, S2, HC);
        CHECK_S(S2, "B2 ==> ");
        DIOCFGPath = savedDio;
        SetFileAttributesA((dio + "\\C0.ini").c_str(), FILE_ATTRIBUTE_NORMAL);
        DeleteFileA((dio + "\\A1.ini").c_str()); DeleteFileA((dio + "\\B2.ini").c_str());
        DeleteFileA((dio + "\\C0.ini").c_str()); DeleteFileA((dio + "\\note.txt").c_str());
        RemoveDirectoryA(dio.c_str());
    }

    std::printf("[4] WriteIniData(unsigned long) block (golden :988-1002)\n");
    reset(); W906_ChangeLog_ULong("x\\Lot.Data", "Count", "Total", 5, 7, S1, S2, HC);
    CHECK(HC == true); CHECK_S(S1, "Count_Total change Value"); CHECK_S(S2, "5==>7");

    std::printf("[4b] WriteIniData(double) block (golden :891-954; NB2 R87: BCB6 compares numerically)\n");
    {
        AnsiString str;
        str.sprintf("%0.4f", 1.5);
        reset(); W906_ChangeLog_Double("x\\Lot.Data", "Info", "Rate", 1.5, 1.5, str, S1, S2, HC);
        CHECK(HC == false); CHECK_S(ExString, "");                          // 1.5 vs "1.5000": equal in BCB6 (Variant) -> not logged
        str.sprintf("%0.4f", 2.0);
        reset(); W906_ChangeLog_Double("x\\Lot.Data", "Info", "Rate", 1.5, 2.0, str, S1, S2, HC);
        CHECK(HC == true); CHECK_S(S1, "Info_Rate change Value"); CHECK_S(S2, "1.5000==>2.0000");
        str.sprintf("%0.4f", 1.2346);
        reset(); W906_ChangeLog_Double("x\\Lot.Data", "Info", "Rate", 1.23456789, 1.2346, str, S1, S2, HC);
        CHECK(HC == true); CHECK_S(S2, "1.2346==>1.2346");                  // NB2 R87 table row 5: golden logs this once too
        str.sprintf("%0.4f", 0.6);
        reset(); W906_ChangeLog_Double("x\\Contact.Data", "Mode", "Kit Diameter", 0.5, 0.6, str, S1, S2, HC);
        CHECK_S(S2, "5.0mm ==>6.0mm");
        {
            AnsiString a = AnsiString(ConvertToMMType((int)1.25));
            AnsiString c = AnsiString(ConvertToMMType((int)3.75));
            AnsiString want;
            want.sprintf("%smm ==> %smm", a.c_str(), c.c_str());
            str.sprintf("%0.4f", 3.75);
            reset(); W906_ChangeLog_Double("x\\HandlerCondition.Data", "Configuration", "dAutoClean_XPitch_Kit", 1.25, 3.75, str, S1, S2, HC);
            CHECK_S(S2, want.c_str());                                      // golden passes the double to ConvertToMMType(int)
        }
        const bool savedTorque = TestIF_File.bEnableReadAndCheckTorque;
        const bool r1 = bResetArm1Value, r2 = bResetArm2Value;
        TestIF_File.bEnableReadAndCheckTorque = false; bResetArm1Value = false; bResetArm2Value = false;
        str.sprintf("%0.4f", 0.2);
        reset(); W906_ChangeLog_Double("x\\IndexOffset.Data", "Test Arm1", "Contact", 0.1, 0.2, str, S1, S2, HC);
        CHECK(bResetArm1Value == false && bResetArm2Value == false);       // torque mode off
        CHECK_S(S1, "Test Arm1_Contact Offset change Value");
        TestIF_File.bEnableReadAndCheckTorque = true;
        reset(); W906_ChangeLog_Double("x\\IndexOffset.Data", "Test Arm1", "Contact", 0.1, 0.2, str, S1, S2, HC);
        CHECK(bResetArm1Value == true && bResetArm2Value == true);         // kevin 20210804: contact offset changed -> re-learn both torque standards
        TestIF_File.bEnableReadAndCheckTorque = savedTorque; bResetArm1Value = r1; bResetArm2Value = r2;
    }

    std::printf("[5] WriteIniData(AnsiString) block (golden :1053-1081)\n");
    reset(); W906_ChangeLog_Str("x\\Lot.Data", "Info", "Rate", "1.50", "1.5", S1, S2, HC);
    CHECK(HC == false);                                                     // both float and equal
    reset(); W906_ChangeLog_Str("x\\Lot.Data", "Info", "Name", "abc", "abd", S1, S2, HC);
    CHECK(HC == true); CHECK_S(S2, "abc==>abd");
    reset(); W906_ChangeLog_Str("x\\Lot.Data", "Info", "Name", "1.5", "abc", S1, S2, HC);
    CHECK(HC == false);                                                     // golden: one float, one not -> not logged
    reset(); W906_ChangeLog_Str("x\\Contact.Data", "Mode", "fSocketInitialICCheckPositionOffset", "0.1", "0.25", S1, S2, HC);
    CHECK_S(S1, "Mode_fSocket Initial IC Check Position Offset change Value");
    CHECK_S(S2, "0.10mm ==> 0.25mm");
    InitialOK = false;
    reset(); W906_ChangeLog_Str("x\\Lot.Data", "Info", "Name", "abc", "abd", S1, S2, HC);
    CHECK(HC == false);
    InitialOK = true;

    std::printf("[6] through common.cpp WriteIniData\n");
    {
        AnsiString ini = AnsiString((g_root + "\\e2e_Tester.Data").c_str());
        W906_ChangeLogHook_Bool = 0; W906_ChangeLogHook_Int = 0; W906_ChangeLogHook_ULong = 0; W906_ChangeLogHook_Str = 0; W906_ChangeLogHook_Double = 0;
        reset(); WriteIniData(ini, "Mode", "Tester Type", 1);
        reset(); WriteIniData(ini, "Mode", "Tester Type", 3);
        CHECK_S(ExString, "");                                              // no hook: exactly the old behaviour
        W906_InstallChangeLogHooks();
        CHECK(W906_ChangeLogHook_Int == W906_ChangeLog_Int);
        reset(); WriteIniData(ini, "Mode", "Tester Type", 1);
        CHECK_S(ExString, "Mode_Tester Type change Value");                 // 3 -> 1 logged
        reset(); WriteIniData(ini, "Mode", "Tester Type", 1);
        CHECK_S(ExString, "");                                              // same value: the write above landed
        reset(); WriteIniData(ini, "Mode", "Tester Name", AnsiString("A"));
        reset(); WriteIniData(ini, "Mode", "Tester Name", AnsiString("B"));
        CHECK_S(ExString, "Mode_Tester Name change Value");
        reset(); WriteIniData(ini, "Mode", "Delay", 1.5);
        reset(); WriteIniData(ini, "Mode", "Delay", 1.5);
        CHECK_S(ExString, "");                                              // double, same value: not logged (numeric compare)
        reset(); WriteIniData(ini, "Mode", "Delay", 2.25);
        CHECK_S(ExString, "Mode_Delay change Value");
        CloseIniFile();
        DeleteFileA(ini.c_str());
    }

    InitialOK = savedInitialOK;
    CUSTOMER_CODE = savedCustomer;
    TestIF_File.iTestMode = savedTestMode;
    TestIF_File.bOctal_16Kit = savedOctal;
    RemoveDirectoryA(g_root.c_str());
    const bool clean = GetFileAttributesA(g_root.c_str()) == INVALID_FILE_ATTRIBUTES;
    CHECK(clean);

    std::printf("%s: %d / %d checks passed\n", g_fail ? "FAIL" : "PASS", g_pass, g_pass + g_fail);
    return g_fail ? 1 : 0;
}
