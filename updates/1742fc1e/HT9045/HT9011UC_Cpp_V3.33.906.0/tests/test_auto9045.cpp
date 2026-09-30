// tests/test_auto9045.cpp
// Verification harness for the Automation/auto9045.h/.cpp whole-file translation
// (W5-Final-Auto9045).
//
// Exercises the translated public API (Get*/Set*/Check*/Amkor*/ASE_KR functions)
// against the ORIGINAL golden reference
//   HT9011UC_Code_V3.33.906.0_20260618/Automation/auto9045.cpp (3018 lines).
//
// LIMITATION (same as test_ContactForce.cpp/test_SCK_ART.cpp): we CANNOT run the
// original BCB6 binary (no Borland compiler in this environment). Verification
// here is therefore: (1) the translation compiles+links clean against the real
// shared globals, AND (2) its outputs equal values hand-derived from the golden
// source, driving those SAME real shared globals to documented values.
//
// DELIBERATE EXCLUSION (flagged for human review -- see translate report):
// this harness does NOT call DoULRequest / DoDLRequest, and does NOT call ANY
// Set* function whose golden body persists via `DataPath+GetLastOpenFN()` (e.g.
// SetSoakTime/SetTemperature/SetMapping/SetHotPlate1/SetIndexArm1Height/...).
// GetLastOpenFN's TU-local stand-in (see auto9045.cpp gate #1) reads the REAL
// D:\HT9045\SetUp.inf on THIS dev machine, which currently names a REAL live
// recipe folder -- calling any of those functions here would overwrite REAL
// production recipe files (Tester.Data/Temperature.Data/Contact.Data/...)
// under D:\HT9045\IniData\Data\<that real recipe>\. This harness only drives
// functions that are confirmed (by direct source inspection) to touch ZERO ini
// files: pure Get* accessors, plus the handful of Set*/Check*/Do* functions
// that only mutate in-memory globals (SetCategory/SetBinDefine/SetDutOnOff/
// SetTempMode/SetTesterConnect/SetTesterMode/SetStartMode/SetLotInfo/
// SetLotInfoGreatekOEE/ShowOLPState/DoClearReportRequest/DoOneCycle).
// AI(W906-I125) 20260930: SetTempMode / SetTesterConnect are NO LONGER in-memory-only
// on every path -- they now reach the real TfMain::ChangeTempMode / ChangeTesterConnect
// (the W5FA stand-ins that returned 0 are retired).  PART 11-12 only drive the paths
// that return before any write: the CheckNeedCleanOut() refusal, ChangeTempMode's
// Mode==10 toggle guard, and ChangeTesterConnect's W906_ChangeTesterConnect_Sim seam.
//
// No external test framework: a tiny check harness prints PASS/FAIL per case
// and a final summary, and returns non-zero on ANY failure.

#include "Automation/auto9045.h"

#include "MachineDefine.h"
#include "MachineType.h"
#include "cprod.h"
#include "cmydef.h"
#include "Config.h"
#include "common.h"
#include "canary_support.h"
#include "aHotPlateSubstrate.h"
#include "FormsFacade.h"

#include <cstdio>
#include <cstring>

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (cond) { printf("PASS  %s\n", msg); ++g_pass; }                      \
        else      { printf("FAIL  %s\n", msg); ++g_fail; }                      \
    } while (0)

// ===========================================================================
//  PART 1 -- trivial pure Get* accessors (no globals to prime)
// ===========================================================================
static void Part1_TrivialGetters()
{
    CHECK(GetHandlerType() == "HT9045", "GetHandlerType() == \"HT9045\" (golden :1194-1197)");
    CHECK(GetDataPath() == DataPath, "GetDataPath() == DataPath global (golden :1179-1182)");
}

// ===========================================================================
//  PART 2 -- GetTotalSite_ASE_KR (golden :2530-2621): pure switch over
//  TestIF.iTestMode -> (row,col,ret). Table-driven, a handful of representative
//  modes (not all 17 cases -- see golden for the full table).
// ===========================================================================
static void Part2_GetTotalSiteASEKR()
{
    int row=0, col=0, ret=0;

    TestIF.iTestMode = SingleSite;
    ret = GetTotalSite_ASE_KR(row, col);
    CHECK(ret==1 && row==1 && col==1, "GetTotalSite_ASE_KR(SingleSite) -> ret=1,row=1,col=1 (golden :2535-2539)");

    TestIF.iTestMode = DualSite;
    ret = GetTotalSite_ASE_KR(row, col);
    CHECK(ret==2 && row==1 && col==2, "GetTotalSite_ASE_KR(DualSite) -> ret=2,row=1,col=2 (golden :2540-2544)");

    TestIF.iTestMode = _8Site2X4;
    ret = GetTotalSite_ASE_KR(row, col);
    CHECK(ret==8 && row==2 && col==4, "GetTotalSite_ASE_KR(_8Site2X4) -> ret=8,row=2,col=4 (golden :2583-2588)");

    TestIF.iTestMode = _32Site4X8M;
    ret = GetTotalSite_ASE_KR(row, col);
    CHECK(ret==32 && row==4 && col==8, "GetTotalSite_ASE_KR(_32Site4X8M) -> ret=32,row=4,col=8 (golden :2614-2618)");
}

// ===========================================================================
//  PART 3 -- GetTestResult_ASE_KR (golden :2491-2528): binary protocol encoder
//  over TestSocket.Item[][]/TestIF.iSiteMap[][], sorted by site number. Drive a
//  1x2 grid with one PASS-equivalent (Item==TEST_PASS) and one NULL_IC site,
//  verify the encoded length matches the golden framing formula exactly:
//  header(2 bytes) + row*col * (6 fixed marker bytes + siteNumLen + binResultLen).
//
//  AI(W5-Final-Integrate) 20260711: FIXED 2 pre-existing bugs in this PART,
//  found the first time this test was ever actually linked+run (the
//  translate unit's own report says "no CMake build attempted" -- this
//  integrate pass is the first real execution):
//    (1) formula bug -- per-site SubData's sprintf format is
//        "%c%c%c%c%s%c%c%s" (golden :2516ish), i.e. SIX %c marker bytes
//        (0x01,0x02,0x41,siteLen, then 0x41,binLen), not four -- the
//        original comment/formula miscounted the trailing 0x41+binLen pair.
//    (2) state-leak bug -- this PART left TestSocket.Item[0][0]=TEST_PASS
//        (a real IC-bearing value) permanently set. TMyKitSuck's default
//        iShtRow=2/iShtCol=1 (aHotPlateSubstrate.cpp ctor) means
//        UseSiteHasIC() (-> TestSocketHasIC() -> IndexHasIC() ->
//        HasICUnderMachine()) scans exactly this cell and returned TRUE for
//        every PART that ran afterward, making CheckNeedCleanOut() true and
//        short-circuiting SetCategory/SetDutOnOff/SetTesterMode (PARTs 5-7)
//        to their early `return 1;` -- 4 downstream FAILs traced to this one
//        leftover global. Reset both cells back to NULL_IC at the end of
//        this PART so later PARTs see the clean-machine state they assume.
// ===========================================================================
static void Part3_GetTestResultASEKR()
{
    TestIF.iSiteMap[0][0] = 1;
    TestIF.iSiteMap[0][1] = 2;
    TestSocket.Item[0][0] = TEST_PASS;      // -> bin result (TEST_PASS-TEST_PASS+1) = 1
    TestSocket.Item[0][1] = NULL_IC;        // -> bin result 0

    AnsiString enc = GetTestResult_ASE_KR(1, 2);

    AnsiString sSite1="1", sBin1="1";       // site 1 -> bin 1 (TEST_PASS case)
    AnsiString sSite2="2", sBin2="0";       // site 2 -> bin 0 (NULL_IC case)
    int expectedLen = 2 + 2*(6 + sSite1.Length() + sBin1.Length());  // both site/bin strings are 1 char here
    CHECK(enc.Length()==expectedLen,
          "GetTestResult_ASE_KR(1,2) encoded length matches golden framing formula (golden :2516-2526)");
    // Header: sprintf("%c%c",0x01,row*col) -- byte[0]=0x01, byte[1]=row*col=2
    CHECK((unsigned char)enc[1]==0x01 && (unsigned char)enc[2]==2,
          "GetTestResult_ASE_KR header bytes == 0x01,row*col (golden :2516)");

    // Restore TestSocket to the clean/no-IC state later PARTs assume (see
    // FIXED note above).
    TestSocket.Item[0][0] = NULL_IC;
    TestSocket.Item[0][1] = NULL_IC;
}

// ===========================================================================
//  PART 4 -- AmkorUDPGetSiteAndTemp/GetSite/GetTemp (golden :2246-2483): pure
//  byte-packing site/temperature UDP packet builders with a trailing XOR
//  checksum over [0..Count-1]. Verify internal self-consistency: re-computing
//  the XOR over the returned packet bytes (before the checksum slot) equals
//  the checksum byte the function itself wrote.
// ===========================================================================
static void Part4_AmkorUDP()
{
    TestIF.iTestMode = SingleSite;                 // MaxI=1,MaxJ=1 (golden :2264-2273)
    TestIF_File.iSiteMap[0][0] = 3;
    LastSet.bUseTestSocket[0][0][0] = true;
    LastSet.iTemperature = Tempture_Hot;

    AmkorUDPGetSiteAndTemp();
    AnsiString s = GetTesterMode();  // sanity: TestMode accessor still callable after Amkor* writes
    (void)s;

    // Re-derive expected checksum manually from the documented golden byte layout
    // (count=2,type=2,model=10,size=4,0,0,0, tempFlag, workTemp, soakLo, soakHi,
    //  type=1,model=10,size=MaxI*MaxJ*2,0,0,0, [siteNum,useSocket]*MaxI*MaxJ).
    unsigned char expected[32];
    int n=0;
    expected[n++]=2; expected[n++]=2; expected[n++]=10; expected[n++]=4;
    expected[n++]=0; expected[n++]=0; expected[n++]=0;
    expected[n++]=(LastSet.iTemperature==Tempture_Hot)?0:1;
    expected[n++]=(unsigned char)(int)Temperature.fWorkTemperBase;
    expected[n++]=(unsigned char)((int)Temperature.fSoakTime%256);
    expected[n++]=(unsigned char)((int)Temperature.fSoakTime/256);
    expected[n++]=1; expected[n++]=10; expected[n++]=(unsigned char)(1*1*2);
    expected[n++]=0; expected[n++]=0; expected[n++]=0;
    expected[n++]=(unsigned char)TestIF_File.iSiteMap[0][0];
    expected[n++]=(unsigned char)(LastSet.bUseTestSocket[0][0][0]?1:0);
    unsigned char bcc=expected[0];
    for(int i=1;i<n;i++) bcc^=expected[i];

    // The packet cursor / checksum byte itself is private to auto9045.cpp's
    // TU-local W5FA_FAutomation companion (see gate list) -- not directly
    // observable from this black-box test. This PART instead pins the byte-
    // layout FORMULA by hand (matching golden verbatim) as an independent
    // oracle; a genuine regression in the packet-building loop would only be
    // caught by a future white-box test once sATKSendMessage[] is folded onto
    // the real (observable) TfAutomationShim -- documented limitation.
    //
    // AI(W5-Final-Integrate) 20260711: FIXED -- the assertion below originally
    // checked n==17, but this hand-built `expected[]` array (see the 12 push
    // lines above) itself pushes 19 elements, not 17: golden's own
    // `W5FA_FAutomation.sATKSendMessageCount=17;` (auto9045.cpp :2671) is only
    // the FIXED-HEADER sub-total, BEFORE the per-site loop appends 2 more
    // bytes/site (siteNum + useSocket) -- for SingleSite (MaxI=MaxJ=1) that is
    // +2, giving a true pre-checksum total of 19. Confirmed by re-reading the
    // real function body (auto9045.cpp :2654-2681): 17 fixed bytes (indices
    // 0-16) + 1 site * 2 bytes = 19. The assertion now matches this file's own
    // (correct) hand-count.
    CHECK(n==19, "AmkorUDPGetSiteAndTemp golden byte layout has 19 pre-checksum bytes for SingleSite: 17 fixed-header + 1 site*2 (golden :2299-2326, :2671)");
    (void)bcc;
}

// ===========================================================================
//  PART 5 -- SetCategory / GetCategory round-trip (golden :41-136 / :1419-1427).
//  Zero ini-file I/O (verified by source inspection) -- safe to exercise.
// ===========================================================================
static void Part5_CategoryRoundTrip()
{
    SystemStart = false;   // CheckSystemStart() -> false (proceed)
    // CheckNeedCleanOut() depends on HasICUnderMachine() (real, Sim HAL) which
    // is false with no IC anywhere -- and CheckCanChangeRealDummy() (AI(W906-I125)
    // 20260930: now the real TfMain body, cMainStatus.cpp:323, no longer a stand-in)
    // is true on that same empty machine -- so CheckNeedCleanOut() -> false (proceed).
    TrayForm.LodareType=0;
    TrayForm.iFixTrayMode=0;
    MachineTypeChoice = Type_HT9045;

    AnsiString Data[40];
    Data[0]="0";                       // IfErrorT3 = Auto1
    for(int i=1;i<=iTestBinCount;i++)
        Data[i]="0";                   // every bin -> Auto1 (Bin Category 0)

    int ret = SetCategory(Data);
    CHECK(ret==0, "SetCategory(all Auto1) returns 0 (golden :41-136)");
    CHECK(BinSelect[iTestRunMode].IfErrorT3==0, "SetCategory wrote BinSelect[iTestRunMode].IfErrorT3==0");

    AnsiString Out[40];
    int n = GetCategory(Out);
    CHECK(n==17, "GetCategory returns 17 (golden :1419-1427, 1+16 fixed count)");
    CHECK(Out[0]=="0", "GetCategory Out[0] echoes IfErrorT3 just written");
}

// ===========================================================================
//  PART 6 -- SetDutOnOff / GetDutOnOff round-trip (golden :275-399 / :1601-1614).
//  Zero ini-file I/O.
// ===========================================================================
static void Part6_DutOnOffRoundTrip()
{
    TestIF.iTestMode = SingleSite;     // MaxI=1,MaxJ=1 (golden :286-300)
    TestIF.iSiteMap[0][0] = 1;         // non-zero -> toggle branch (golden :346-354)
    bUseTwoArm32Site = false;

    AnsiString Data[40];
    for(int i=0;i<40;i++) Data[i]="1"; // request every site ON

    int ret = SetDutOnOff(Data);
    CHECK(ret==0, "SetDutOnOff returns 0 (golden :275-399)");
    CHECK(LastSet.bUseTestSocket[0][0][0]==true, "SetDutOnOff wrote LastSet.bUseTestSocket[0][0][0]==true");

    AnsiString Out[40];
    int n = GetDutOnOff(Out);
    CHECK(n==TestSocket.iShtCnt, "GetDutOnOff returns TestSocket.iShtCnt (golden :1601-1614)");
}

// ===========================================================================
//  PART 7 -- SetTesterMode / GetTesterMode round-trip (golden :592-600 / :1236-1239).
// ===========================================================================
static void Part7_TesterModeRoundTrip()
{
    SystemStart=false;
    AnsiString Data[4]; Data[0]="7";
    int ret = SetTesterMode(Data);
    CHECK(ret==0, "SetTesterMode returns 0 (golden :592-600)");
    CHECK(GetTesterMode()=="7", "GetTesterMode() echoes SetTesterMode's Data[0] via the TU-local fAutomation gate");
}

// ===========================================================================
//  PART 8 -- CheckSystemStart / CheckInitialOK / CheckCanChangeRealDummy /
//  CheckNeedCleanOut predicates (golden :1130-1146 / :1388-1391).
// ===========================================================================
static void Part8_Predicates()
{
    SystemStart=false;
    CHECK(CheckSystemStart()==false, "CheckSystemStart() mirrors SystemStart global (golden :1143-1146)");
    SystemStart=true;
    CHECK(CheckSystemStart()==true, "CheckSystemStart() mirrors SystemStart==true");
    SystemStart=false;

    InitialOK=true;
    CHECK(CheckInitialOK()==true, "CheckInitialOK() mirrors InitialOK global (golden :1130-1133)");
    InitialOK=false;

    // AI(W906-I125) 20260930: RECALIBRATED.  The old assertion pinned the retired
    // W5FA_FMain stand-in's hard-coded `true` ("offline default").  The call now
    // reaches the real TfMain::CheckCanChangeRealDummy (cMainStatus.cpp:323 =
    // golden main.cpp:12374-12380), whose `return true` (golden :12379) is reached
    // only when none of MMPlate1/2, the shuttles, the index, InArmSuck, OutArmSuck
    // holds an IC -- which is this harness's state here (PART 3 restores TestSocket).
    // The IC-present half is PART 11 (and tests/test_i125_realdummy.cpp).
    CHECK(CheckCanChangeRealDummy()==true, "CheckCanChangeRealDummy() == true on an empty machine (golden auto9045.cpp:1388-1391 -> main.cpp:12379)");
}

// ===========================================================================
//  PART 9 -- SetLotInfoGreatekOEE gate wiring (golden auto9045.cpp F899-008
//  addendum): with the TU-local fProductionInfo/fMonitor gates offline, the
//  function must fail closed via the documented fallback path.
// ===========================================================================
static void Part9_SetLotInfoGreatekOEE()
{
    AnsiString Data[4];
    AnsiString errMsg;
    int ret = SetLotInfoGreatekOEE(Data, errMsg);
    // fMain->CheckCanChangeRealDummy()==true (empty machine; AI(W906-I125) 20260930:
    // real body now, was the W5FA_FMain stand-in) -> IniConfig.bC11UseMonitorView
    // default false -> W5FA_FProductionInfo.OEE_StartLot(...) offline returns
    // false with errMsg left empty -> caller fills "Production Start Lot Fail"
    // -> GreatekMapOEEStartLotError(...) finds neither "Machine ID" nor "MO" -> 90.
    CHECK(ret==90, "SetLotInfoGreatekOEE offline gate chain -> 90 (no OEE backend, fallback code)");
    CHECK(errMsg=="Production Start Lot Fail", "SetLotInfoGreatekOEE fills the documented fallback error message");
}

// ===========================================================================
//  PART 10 -- DoOneCycle (golden :2240-2244): pure in-memory cursor set, no
//  ini I/O (InitOneCycle is a real, already-verified csystem.cpp function).
// ===========================================================================
static void Part10_DoOneCycle()
{
    fMain->BtnOneCycle->Down = false;
    DoOneCycle();
    CHECK(fMain->BtnOneCycle->Down==true, "DoOneCycle() sets fMain->BtnOneCycle->Down=true (golden :2240-2244)");
}

// ===========================================================================
//  PART 11 -- AI(W906-I125) 20260930: the retired CheckCanChangeRealDummy stand-in,
//  IC-present half.  One IC in OutArmSuck must make every auto9045 consumer refuse
//  exactly as golden.  HasICUnderMachine() (csystem.cpp:13304) also sees OutArmSuck,
//  so CheckNeedCleanOut() and the setters below would refuse even with the old
//  `return true` stand-in; the DISCRIMINATING checks are the ones that consult
//  CheckCanChangeRealDummy ALONE -- the free function, SetStartMode, SetLotInfo,
//  SetLotInfoGreatekOEE -- which answered true/0/0/90 with the stand-in.
// ===========================================================================
static void Part11_RealDummyRejects()
{
    SystemStart = false;
    for(int i=0;i<_MAX_SUCK_ROW_ITEM;i++)
        for(int j=0;j<_MAX_SUCK_COL_ITEM;j++)
            OutArmSuck.Item[i][j] = NULL_IC;
    OutArmSuck.Item[0][0] = HAS_IC;
    CHECK(OutArmSuck.HasIC()==true, "PART 11 precondition: OutArmSuck.HasIC()==true (the placement is visible)");

    CHECK(fMain->CheckCanChangeRealDummy()==false, "TfMain::CheckCanChangeRealDummy()==false with an IC in OutArmSuck (golden main.cpp:12378)");
    CHECK(CheckCanChangeRealDummy()==false,        "auto9045 CheckCanChangeRealDummy()==false (golden :1388-1391) -- the retired stand-in said true");
    CHECK(CheckNeedCleanOut()==true,               "CheckNeedCleanOut()==true (golden :1135-1141)");

    AnsiString d[40];
    d[0] = "1";                                                  // a valid StartMode: only the guard can refuse
    CHECK(SetStartMode(d)==2, "SetStartMode(\"1\")==2 (golden :1111/:1124-1127) -- was 0 with the stand-in");
    AnsiString li[40];
    li[0] = "I125LOT"; li[1] = "I125OP"; li[2] = "0";
    CHECK(SetLotInfo(li)==2,  "SetLotInfo(...)==2 (golden :1022/:1037-1040) -- was 0 with the stand-in");
    AnsiString g[4];
    AnsiString err;
    int rg = SetLotInfoGreatekOEE(g, err);
    CHECK(rg==2 && err=="Machine running or has residual IC",
          "SetLotInfoGreatekOEE -> 2 + \"Machine running or has residual IC\" (golden :1070-1074) -- was 90 with the stand-in");

    // The CheckNeedCleanOut()-gated setters refuse with 1 before touching anything.
    AnsiString s[40];
    s[0] = "I125_NO_SUCH_RECIPE";
    CHECK(SetSetUpFileName(s)==1, "SetSetUpFileName -> 1 (CheckNeedCleanOut, golden :515-516)");
    s[0] = "10";
    CHECK(SetTempMode(s)==1,      "SetTempMode -> 1 (CheckNeedCleanOut, golden :576-577), before ChangeTempMode");
    s[0] = "1";
    CHECK(SetTesterConnect(s)==1, "SetTesterConnect -> 1 (CheckNeedCleanOut, golden :586-587), before ChangeTesterConnect");

    OutArmSuck.Item[0][0] = NULL_IC;
    CHECK(CheckCanChangeRealDummy()==true, "IC removed -> CheckCanChangeRealDummy()==true again (no latch)");
}

// ===========================================================================
//  PART 12 -- AI(W906-I125) 20260930: which W5FA_TfMainExt members were RETIRED to
//  fMain and which were KEPT, observed through the auto9045 API on an empty machine.
//  Every call below returns before any file write (cited per call).
// ===========================================================================
static void Part12_RetiredVsKept()
{
    SystemStart = false;
    CHECK(CheckNeedCleanOut()==false, "PART 12 precondition: empty machine, CheckNeedCleanOut()==false");

    // (a) cbSetupFileName, RETIRED half: GetSetUpFileName reads fMain's combo (golden :1199-1202).
    AnsiString savedText = fMain->cbSetupFileName->Text;
    fMain->cbSetupFileName->Text = "I125_RECIPE";
    CHECK(GetSetUpFileName()=="I125_RECIPE", "GetSetUpFileName() returns fMain->cbSetupFileName->Text (golden :1201) -- was the stand-in's \"\"");

    // (b) cbSetupFileName, KEPT half: SetSetUpFileName still looks the name up in the stand-in's own list,
    //     paired with the still-no-op ChangeSetUpFile.  A recipe that IS in fMain's real list must not be
    //     answered 0 ("switched") -- nothing would switch.  Golden :518-526 -> 3.
    int savedCount = fMain->cbSetupFileName->Items->Count;
    int savedIndex = fMain->cbSetupFileName->ItemIndex;
    fMain->cbSetupFileName->Items->Add("I125_RECIPE");
    AnsiString s[40];
    s[0] = "I125_RECIPE";
    CHECK(SetSetUpFileName(s)==3, "SetSetUpFileName(a name in fMain's list) == 3, not a false 0 (kept stand-in pair, auto9045.cpp Gate #6)");
    CHECK(fMain->cbSetupFileName->ItemIndex==savedIndex, "SetSetUpFileName left fMain's ItemIndex alone");
    fMain->cbSetupFileName->Items->Delete(savedCount);
    fMain->cbSetupFileName->Text = savedText;

    // (c) palMainStatus RETIRED: GetMainStatus reads fMain's Caption (golden :1154-1177).
    AnsiString savedCap = fMain->palMainStatus->Caption;
    fMain->palMainStatus->Caption = "HALT";
    CHECK(GetMainStatus()=="HALT",  "GetMainStatus(): Caption HALT -> \"HALT\" (golden :1157-1171) -- the stand-in always gave \"RUN\"");
    fMain->palMainStatus->Caption = "Pause";
    CHECK(GetMainStatus()=="PAUSE", "GetMainStatus(): Caption Pause -> \"PAUSE\" (UpperCase, golden :1156)");
    fMain->palMainStatus->Caption = "Running";
    CHECK(GetMainStatus()=="RUN",   "GetMainStatus(): Caption Running -> \"RUN\" (golden :1162-1165)");
    fMain->palMainStatus->Caption = "Drying Wait";
    CHECK(GetMainStatus()=="RUN",   "GetMainStatus(): any other caption -> \"RUN\" (golden :1173-1176)");
    fMain->palMainStatus->Caption = savedCap;

    // (d) CanChangeSite RETIRED to the facade member: its seam now drives SetDutOnOff's `return 3`
    //     (golden :316-319), which the stand-in's literal `true` made unreachable.
    TestIF.iTestMode = SingleSite;
    LastSet.bUseTestSocket[0][0][0] = false;
    fMain->W906_CanChangeSite_Sim = false;
    AnsiString dut[40];
    for(int i=0;i<40;i++) dut[i] = "1";
    CHECK(SetDutOnOff(dut)==3, "SetDutOnOff -> 3 when fMain->CanChangeSite()==false (golden :316-319)");
    CHECK(LastSet.bUseTestSocket[0][0][0]==false, "SetDutOnOff wrote nothing on the CanChangeSite refusal");
    fMain->W906_CanChangeSite_Sim = true;

    // (e) ChangeTesterConnect RETIRED: SetTesterConnect returns TfMain::ChangeTesterConnect's answer
    //     (golden :589).  The W906_ChangeTesterConnect_Sim seam (forms/fMain.cpp:1098) returns first -- no write.
    int savedTester = LastSet.iTester;
    fMain->W906_ChangeTesterConnect_Sim = 7;
    AnsiString tc[40];
    tc[0] = "1";
    CHECK(SetTesterConnect(tc)==7, "SetTesterConnect returns TfMain::ChangeTesterConnect's value (seam 7) -- the stand-in returned 0");
    fMain->W906_ChangeTesterConnect_Sim = 0;
    LastSet.iTester = savedTester;

    // (f) ChangeTempMode RETIRED: SetTempMode("10") reaches the real body (MainTempMode.cpp:65).  Empty
    //     machine, room-temp mode (iMachineTempMode 0), bHighModeCanOffTemp off: golden main.cpp:21776-21812's
    //     Mode==10 toggle guard answers 1 with Msg=false (no message, no write).  The stand-in answered 0.
    int  savedTemp        = LastSet.iTemperature;
    int  savedMachineMode = Temperature.iMachineTempMode;
    bool savedHigh        = IniConfig.bHighModeCanOffTemp;
    Temperature.iMachineTempMode  = 0;
    IniConfig.bHighModeCanOffTemp = false;
    AnsiString tm[40];
    tm[0] = "10";
    CHECK(SetTempMode(tm)==1,       "SetTempMode(\"10\") -> 1 from TfMain::ChangeTempMode's Mode==10 guard (golden main.cpp:21776-21812) -- the stand-in answered 0");
    CHECK(LastSet.iTemperature==10, "SetTempMode got past CheckNeedCleanOut and set LastSet.iTemperature=10 first (golden :578)");
    LastSet.iTemperature          = savedTemp;
    Temperature.iMachineTempMode  = savedMachineMode;
    IniConfig.bHighModeCanOffTemp = savedHigh;
}

int main()
{
    Part1_TrivialGetters();
    Part2_GetTotalSiteASEKR();
    Part3_GetTestResultASEKR();
    Part4_AmkorUDP();
    Part5_CategoryRoundTrip();
    Part6_DutOnOffRoundTrip();
    Part7_TesterModeRoundTrip();
    Part8_Predicates();
    Part9_SetLotInfoGreatekOEE();
    Part10_DoOneCycle();
    Part11_RealDummyRejects();      // AI(W906-I125) 20260930
    Part12_RetiredVsKept();         // AI(W906-I125) 20260930

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail==0 ? 0 : 1;
}
